/***********************************************************************************
 * 文 件 名   : electricityMeter.c
 * 负 责 人   : Edward
 * 创建日期   : 2017年2月21日
 * 文件描述   : 电表操作接口
 * 版权说明   : Copyright (c) 2008-2017   无锡飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

/*********************************************************************
* INCLUDES
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include "queue.h"
#include "math.h"

#include "electricityMeter.h"

#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"

#include "zbSocPrivate.h"
#include "mt_zbSocCmd.h"
#include "handleManage.h"
#include "comParse.h"
#include "interface_srpcserver.h"
#include "interface_srpcserver_defs.h"
/*********************************************************************
* MACROS
*/

/*start of frame*/
#define ELECMETER_SOF		0x68
/*end of frame*/
#define ELECMETER_EOF		0x16
#define ELECMETER_FIRST_WKUP 0xFE

/*控制码*/
#define CONTROL_FIELD_DIR			0x80
#define CONTROL_FIELD_SLAVE_CFM		0x40
#define CONTROL_FIELD_FOLLOWUP_DATA	0x20
#define CONTROL_FIELD_FUNCTION		0x1F

#define PACKET_DEFAULT_SIZE	16
#define ADDR_DEFAULT_SIZE	6

#define MAX_FUN_SIZE		28

#define SOF_STATE      		0x00
#define ADDR_STATE	   0x01
#define SOF_STATE2	   0x02
#define CTRL_STATE	   0x03
#define LEN_STATE	   0x04
#define DATA_STATE	   0x05
#define FCS_STATE	   0x06
#define END_STATE	   0x07


/*********************************************************************
* CONSTANTS
*/

/*********************************************************************
* TYPEDEFS
*/
//电表信息集合
typedef struct
{
    uint32_t rechargeValue;
    uint32_t totalElectricity;      //总用电量
    uint32_t surplusElectricity;    //剩余电量
    uint32_t overdraftElectricity;  //透支电量
    uint32_t lastrRecharge;         //上次充值电量
    uint32_t totalRecharge;         //总充值电量
    uint16_t RechargeNumber;        //充值次数
    uint32_t lightWarnValue;        //光电报警阈值
    uint32_t offWarnValue;         	//拉闸报警阈值
    uint32_t cornerValue;           //囤积限量
    uint32_t creditValue;           //赊欠限量
    uint32_t limitingValue;         //限容
    uint16_t state;                 //状态字
}METER_INFO_GATHER_T;

//服务端命令定义
typedef enum
{
    SERVER_CMD_ELECTRICITY_QUERY = 0X01,
    SERVER_CMD_ELECTRICITY_REPORT,
    SERVER_CMD_STATE_CTRL,
    SERVER_CMD_STATE_QUERY,
    SERVER_CMD_STATE_REPORT,
    SERVER_CMD_MODE_CTRL,
    SERVER_CMD_MODE_QUERY,
    SERVER_CMD_MODE_REPORT,
    SERVER_CMD_RECHARGE,
    SERVER_CMD_RECHARGE_REPORT,
    SERVER_CMD_WARN,
    SERVER_CMD_RESET,
    SERVER_CMD_RESET_REPORT,
    
}SERVER_CMD_T;

/*控制码的功能表*/
typedef enum{
	CONTROL_FIELD_FUN_RESERVE		=0x00,//保留
	CONTROL_FIELD_FUN_BROADCAST		=0x08,//广播校时
	CONTROL_FIELD_FUN_READ			=0x11,//读数据
	CONTROL_FIELD_FUN_READ_FOLLOW	=0x12,//读后续数据
	CONTROL_FIELD_FUN_READ_ADDR		=0x13,//读通讯地址
	CONTROL_FIELD_FUN_WRITE			=0x14,//写数据
	CONTROL_FIELD_FUN_WRITE_ADDR	=0x15,//写通讯地址
	CONTROL_FIELD_FUN_FEEZE_CMD		=0x16,//冻结命令
	CONTROL_FIELD_FUN_SET_BAUDRATE	=0x17,//更改通讯速率
	CONTROL_FIELD_FUN_SET_PASSWD	=0x18,//修改密码
	CONTROL_FIELD_FUN_NEED_CLEAN	=0x19,//最大需量清零
	CONTROL_FIELD_FUN_METER_CLEAN	=0x1A,//电表清零
	CONTROL_FIELD_FUN_EVENT_CLEAN	=0x1B,//事件清零
}ELECMETER_FUN_TYPE_T;

typedef int (*meter_sendCmdCallBack_t)(uint8_t *cmds,uint8_t len);

typedef struct 
{
    uint8_t aAddress[6];     //地址域
    uint8_t nCtrlNumber;    //控制码
    uint8_t nDataLength;    //数据长度
    uint8_t aData[MaxPacketLength];
    
}serial_packet_t;

typedef struct serial_fifo_t
{
	uint8_t state;
    uint16_t len;
    uint8_t idx;
    uint8_t tempDataLen;
    uint8_t endpoint;
    uint8_t IEEEAddr[8];
	uint8_t meter_addrs[6] ;
    uint8_t data[MaxPacketLength];
    uint8_t reportCMD[100];
	uint32_t reportCount;
    
    uint8_t writeRespCMD[100];
    uint32_t respCount;
    
    serial_packet_t packet;
    METER_INFO_GATHER_T meterInfo;
}serial_fifo_t;

typedef struct SerialListHead_t
{
	uint8_t hasInit;
	serial_fifo_t *serialFifo;
    LIST_ENTRY(SerialListHead_t) entry_;
} SerialListHead_t;

/*********************************************************************
* GLOBAL VARIABLES
*/

/*********************************************************************
* LOCAL VARIABLES
*/

LIST_HEAD(SerialList, SerialListHead_t) ;

static struct SerialList SerialListHead;

static uint8_t broadcast_addr[6] = {0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};
static uint8_t meter_addrs[6] = {0};
static meter_sendCmdCallBack_t  *meter_sendCmd = NULL;

//static serial_fifo_t serial_fifo={SOF_STATE,0,0,0,0,{0},{0}};

static serial_fifo_t *fifoTable[100] = {NULL};

//static METER_INFO_GATHER_T meterInfo;
static HANDLE_MANAGE_HANDLE fifoManage = NULL;

/*电表操作标识码*/
static char DataIdentList[MAX_FUN_SIZE][4] =
{
	{0x04, 0x00, 0x04, 0x01},
	{0x04, 0x00, 0x04, 0x09},
	{0x00, 0x00, 0x00, 0x00},
	{0x00, 0x90, 0x01, 0x00},
	{0x00, 0x90, 0x01, 0x01},
	{0x03, 0x32, 0x01, 0x02},
	{0x03, 0x32, 0x01, 0x03},
	{0x03, 0x32, 0x01, 0x06},
	{0x04, 0x00, 0x0F, 0x01},
	{0x04, 0x00, 0x0F, 0x02},
	{0x04, 0x00, 0x0F, 0x03},
	{0x04, 0x00, 0x0F, 0x04},
	{0x04, 0x00, 0x0E, 0x01},
	{0x02, 0x03, 0x00, 0x00},
	{0x04, 0x00, 0x0E, 0x05},
	{0x04, 0x00, 0x05, 0x03},
	{0x04, 0x00, 0x04, 0x10},
	{0x04, 0x00, 0x04, 0x11},
	{0x04, 0x00, 0x04, 0x12},
	{0x04, 0x00, 0x04, 0x13},
	{0x02, 0x01, 0x01, 0x00},
	{0x02, 0x02, 0x01, 0x00},
	{0x02, 0x03, 0x01, 0x00},
	{0x02, 0x04, 0x01, 0x00},
	{0x02, 0x05, 0x01, 0x00},
	{0x02, 0x06, 0x01, 0x00},
	{0x02, 0x80, 0x00, 0x02},
  	{0x04, 0x00, 0x05, 0x08},
};

/*********************************************************************
* LOCAL FUNCTIONS
*/

serial_fifo_t *meter_fifoManage(epInfo_t *epInfo)
{
    uint8_t i;
    
    for (i = 0; i < 100; i++)
    {
        if (memcmp(fifoTable[i]->IEEEAddr, epInfo->IEEEAddr, 8) == 0)
        {
            return fifoTable[i];
        }
    }
}


/******************************************************************
  * @函数说明：   整型转BCD码
  * @输入参数：   uint8_t *buf BCD输出 
  *               uint32_t data     要转换的数据
  * @输出参数：   无 
  * @返回参数：   buf长度
  * @修改记录：   
******************************************************************/
static uint8_t u32ToBcd(uint8_t *buf, uint32_t data)
{
    uint8_t i;
    uint8_t length;
    for (i = 0; i < 4; i++)
    {
        buf[i] = data % 10;
        buf[i] |= (((data / 10) % 10) << 4) & 0xf0;
        data =  data / 100;
        if (data == 0)
        {
            length = i;
            break;
        }
    }
    return length;
}

/******************************************************************
  * @函数说明：   bcd转整型
  * @输入参数：   uint8_t *buf BCD 数据
  *               uint8_t length buf的长度
  * @输出参数：   无 
  * @返回参数：   u32 数据  
  * @修改记录：   
******************************************************************/
static uint32_t bcdToU32(uint8_t *buf, uint8_t length)
{
    uint32_t data = 0;
    uint8_t i;
    uint8_t temp;
    
    for (i = 0; i < length; i++)
    {
        temp = (buf[i] & 0x0f) % 10;
        temp += (((buf[i] >> 4) % 10) * 10);
        data += temp * pow(10, (i) * 2);
    }
    return data;
}

void meter_reportElec(epInfo_t *epInfo, uint8_t result, uint8_t type,uint32_t elec)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_METER_DEVICE_ID | (SERVER_CMD_ELECTRICITY_REPORT & 0xff));
    cmdSetStringVal(&cmd, epInfo->IEEEAddr,8);
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    cmdSet8bitVal(&cmd,result);
    cmdSet8bitVal(&cmd,type);
    cmdSet32bitVal(&cmd, elec);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void meter_reportWarn(epInfo_t *epInfo, uint8_t type)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_METER_DEVICE_ID | (SERVER_CMD_ELECTRICITY_REPORT & 0xff));
    cmdSetStringVal(&cmd, epInfo->IEEEAddr,8);
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    cmdSet8bitVal(&cmd,type);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void meter_report(epInfo_t *epInfo, METER_INFO_GATHER_T *meterInfo,uint8_t reportCMD)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_METER_DEVICE_ID | (reportCMD & 0xff));
    cmdSetStringVal(&cmd, epInfo->IEEEAddr,8);
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    
    log_debug("meter report %x \r\n", reportCMD);
    switch(reportCMD)
    {
        case SERVER_CMD_STATE_REPORT:
        {
            cmdSet8bitVal(&cmd, 0X00);
            
            if ((meterInfo->state & 0x2) > 0)
            {
                 cmdSet8bitVal(&cmd, 0X01);
            }
            else
            {
                cmdSet8bitVal(&cmd, 0X00);
            }
        }
        break;
        case SERVER_CMD_MODE_REPORT:
        {
            cmdSet8bitVal(&cmd, 0X00);
            
            if ((meterInfo->state & 0x4) > 0)
            {
                 cmdSet8bitVal(&cmd, 0X01);
            }
            else
            {
                cmdSet8bitVal(&cmd, 0X00);
            }
        }
        break;
        
        case SERVER_CMD_RECHARGE:
        {
            uint8_t data[26];
            
            memset(data, 0, 26);
			
            meterInfo->totalRecharge += meterInfo->rechargeValue;
            meterInfo->RechargeNumber += 1;
            meterInfo->offWarnValue = OFF_WARN_VALUE; //5kw/h
            meterInfo->lightWarnValue = 0;

			//囤积限量
			if(!meterInfo->cornerValue)
				meterInfo->cornerValue = 999999;

			//赊欠限量
			meterInfo->creditValue = 0;
            log_debug(" totalRecharge %d\r\n", meterInfo->totalRecharge);
			log_debug(" cornerValue %d\r\n", meterInfo->cornerValue);

            u32ToBcd(data, meterInfo->totalRecharge);
            u32ToBcd(&data[4], (uint32_t)meterInfo->RechargeNumber & 0xffff);

            u32ToBcd(&data[6], meterInfo->lightWarnValue);
            u32ToBcd(&data[10], meterInfo->offWarnValue);
            u32ToBcd(&data[14], meterInfo->cornerValue);
            u32ToBcd(&data[18], meterInfo->creditValue);
            u32ToBcd(&data[22], meterInfo->limitingValue);
            
            log_debug_array(data,26,NULL);
            meter_write(epInfo, ELEC_METER_W_RECHARGE, data, 26);
            return ;
        }
        break;
    }
    
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}





uint8_t meter_fcs(uint8_t *packet,uint16_t len)
{
	uint16_t cnt = 0;
	uint16_t sum = 0;

	ASSERT(packet != NULL && len > 0);

	for(cnt = 0; cnt < len; cnt++)
	{
		sum	+= packet[cnt];
	}

	return (sum & 0xFF);
}

void meter_SendSerialCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t len,uint8_t* buf)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_DOORLOCK_UART_MSG);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, len);//数据长度
	cmdSetStringVal(&cmd,buf,len);//数据
   
    zbMakeMsgEnder(&cmd);
    
	usleep(1000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}


/*********************************************************************
* GLOBAL FUNCTIONS
*/

int meter_read(epInfo_t *epInfo,uint8_t cmds)
{
	int8_t cnt = 0;
	int ret = FAILED;
	uint8_t index = 0;
	uint8_t pktlen = 0;
	uint8_t *dataIdent = NULL;
	uint8_t *packet = NULL;

	uint8_t temaddr[6] = {0};
	
	ASSERT((cmds < MAX_FUN_SIZE) && (epInfo != NULL));

	dataIdent = DataIdentList[cmds];

	do{
		packet = malloc(PACKET_DEFAULT_SIZE+4);
		if(!packet)
			break;
		
		packet[index++] = ELECMETER_FIRST_WKUP;
		packet[index++] = ELECMETER_FIRST_WKUP;
		packet[index++] = ELECMETER_FIRST_WKUP;
		packet[index++] = ELECMETER_FIRST_WKUP;

		packet[index++] = ELECMETER_SOF;

		/*所有数据读取都采用广播地址*/
		memcpy(&packet[index],broadcast_addr,ADDR_DEFAULT_SIZE);

		index += 6;
		
		packet[index++] = ELECMETER_SOF;
		packet[index++] = CONTROL_FIELD_FUN_READ;
		packet[index++] = 4;
		
		for(cnt = 3 ;cnt >= 0;cnt--)
		{
			packet[index++] = ((dataIdent[cnt]+0x33)&0xFF);
		}
		
		packet[index++] = meter_fcs(&packet[4],index-5);//0XFE x 4  + fcs x 1
		
		packet[index++] = ELECMETER_EOF;	

		meter_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,index,packet);

		ret = SUCCESS;
		
	}while(0);
	
	if(packet)
		free(packet);

	return ret;
}

int meter_write(epInfo_t *epInfo,uint8_t cmds,uint8_t *data,uint8_t len)
{
	int8_t cnt = 0;
	int ret = FAILED;
	uint8_t index = 0;
	uint8_t pktlen = 0;
	uint8_t *dataIdent = NULL;
	uint8_t *packet = NULL;
	uint8_t temaddr[6] = {0};
	
	ASSERT(cmds < MAX_FUN_SIZE && epInfo != NULL);

	dataIdent = DataIdentList[cmds];

	do{
		packet = malloc(PACKET_DEFAULT_SIZE+len+8+4);//L =04H（密码）+04H（操作者代码）+m(数据长度)+0XFE x 4
		if(!packet)
			break;

		memset(packet,0,sizeof(packet))	;

		packet[index++] = ELECMETER_FIRST_WKUP;
		packet[index++] = ELECMETER_FIRST_WKUP;
		packet[index++] = ELECMETER_FIRST_WKUP;
		packet[index++] = ELECMETER_FIRST_WKUP;

		packet[index++] = ELECMETER_SOF;

		serial_fifo_t *serial_fifo = HandleMange_GetHadnle(fifoManage, epInfo->IEEEAddr);

		if(serial_fifo == NULL || (serial_fifo != NULL&&(!memcmp(serial_fifo->meter_addrs,temaddr,6))))
		{
			memcpy(&packet[index],broadcast_addr,ADDR_DEFAULT_SIZE);
		}else{
			memcpy(&packet[index],serial_fifo->meter_addrs,ADDR_DEFAULT_SIZE);
		}

		index += 6;
		
		packet[index++] = ELECMETER_SOF;
		packet[index++] = CONTROL_FIELD_FUN_WRITE;
		packet[index++] = 4+4+4+len;

		//填充标识符
		for(cnt = 3 ;cnt >= 0;cnt--)
		{
			packet[index++] = ((dataIdent[cnt]+0x33)&0xFF);
		}

		//填充权限
		packet[index++] = ((0x02+0x33)&0xFF);

		//填充密码
		packet[index++] = ((0x00+0x33)&0xFF);
		packet[index++] = ((0x00+0x33)&0xFF);
		packet[index++] = ((0x00+0x33)&0xFF);

		//填充操作员
		packet[index++] = ((0x00+0x33)&0xFF);
		packet[index++] = ((0x00+0x33)&0xFF);
		packet[index++] = ((0x00+0x33)&0xFF);
		packet[index++] = ((0x00+0x33)&0xFF);

		//填充需要写入的数据
		//for(cnt = len-1 ;cnt >= 0;cnt--)
		//{
		//	packet[index++] = ((data[cnt]+0x33)&0xFF);
		//}
        
        for (cnt = 0; cnt < len; cnt++)
        {
            packet[index++] = ((data[cnt]+0x33)&0xFF);
        }
        
        
		
		//packet[index] = meter_fcs(packet,index-5);//0XFE x 4  + fcs x 1
		//packet[++index] = ELECMETER_EOF;	
        
        packet[index++] = meter_fcs(&packet[4],index-5);//0XFE x 4  + fcs x 1
        packet[index++] = ELECMETER_EOF;	
        
        log_debug("\r\n#################################### \r\n");
        log_debug_array(packet,index,NULL);
        
		//zblist_add(epInfo,packet,index);
        log_debug("index %d \r\n", index);
        log_debug("\r\n#################################### \r\n");
		meter_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,index,packet);
		ret = SUCCESS;
	}while(0);

	if(packet)
		free(packet);
		
	return ret;
}


int meter_GetDataIdent(serial_fifo_t *fifo,uint8_t *dataIdent,uint8_t len)
{
	int8_t cnt = 0;
	if (!len)
    {
        return -1;
    }

    if ((fifo->idx + len) <= fifo->len)
    {

		for(cnt = 3;cnt >=0;cnt--)
		{
			dataIdent[cnt] = ((fifo->data[fifo->idx++]-0x33)&0xff);
		}
    	
       	return 0;
    }
    
   return -1;
}

int meter_matchDataIdent(uint8_t *dataIdent,uint8_t len)
{
	int8_t cnt = 0;
	int8_t ret = 0;
	for(cnt = 0;cnt < MAX_FUN_SIZE;cnt++)
	{
		ret = memcmp(dataIdent,DataIdentList[cnt],len);
		if(!ret)
		{
			return cnt;
		}
	}

	return -1;
}

int meter_ResolveReadMsg(epInfo_t *epInfo,serial_fifo_t *fifo)
{
	uint8_t dataIdent[4] = {0};
	int8_t cnt = 0;
	int8_t ret = 0;
	uint8_t datalen = 0;
	ASSERT(epInfo != NULL && fifo != NULL);	

	datalen = fifo->data[fifo->idx++];
	
	ret = meter_GetDataIdent(fifo,dataIdent,4);
	log_debug("\r\n**********************************\r\n");
	log_debug("DataIdent:%x:%x:%x:%x\n",dataIdent[0],dataIdent[1],dataIdent[2],dataIdent[3]);

	if(!ret)
	{
		ret = meter_matchDataIdent(dataIdent,4);
		log_debug("ret = %d\n",ret);
        
		if(ret != -1)
		{
			//memcpy(broadcast_addr, fifo->packet.aAddress,ADDR_DEFAULT_SIZE);
			switch(ret)
			{
				case ELEC_METER_RW_ADDR:
				{
					for(cnt = 0;cnt < 6;cnt++)
					{
						fifo->meter_addrs[cnt] = meter_addrs[cnt] = ((fifo->data[fifo->idx++]-0x33)&0xff);
					}

					log_debug("dev Addr:%x:%x:%x:%x:%x:%x\n",meter_addrs[0],meter_addrs[1],meter_addrs[2],meter_addrs[3],meter_addrs[4],meter_addrs[5]);
				}
				break;
		    case ELEC_METER_RW_TOTAL_ELEC:  //总电量
		    {
		        uint32_t value = bcdToU32(&fifo->packet.aData[4], 4);
		        log_debug(" Total elec %d \r\n", value);
		        meter_reportElec(epInfo, 0, 0, value);
		    }
		    break;
		    
		    case ELEC_METER_R_TOTAL_BUY_ELEC:
		    {
		        uint32_t value = bcdToU32(&fifo->packet.aData[4], 4);
		        log_debug(" TOTAL_BUY_ELEC %d \r\n", value);
		        meter_reportElec(epInfo, 0, 1, value);
		    }
		    break;
		    
		    case ELEC_METER_R_SURPLUS_ELEC: //剩余电量
		    {
		        uint32_t value = bcdToU32(&fifo->packet.aData[4], 4);
		        log_debug(" SURPLUS_ELEC %d \r\n", value);
		        meter_reportElec(epInfo, 0, 2, value);
		    }
		    break;

				case ELEC_METER_R_OVERDRAFT_ELEC:
				{
						uint32_t value = bcdToU32(&fifo->packet.aData[4], 4);
		        log_debug(" OVERDRAFT_ELEC %d \r\n", value);
		        meter_reportElec(epInfo, 0, 3, value);
				}
				break;
				
		    case ELEC_METER_W_REMOTE_CTRL:
		    {
		        
		    }
		    break;
		    
		    case ELEC_METER_R_WARN:
		    {
		        meter_reportWarn(epInfo, 2);
		    }
		    break;
		    case ELEC_METER_R_DATA:
		    {
		        fifo->meterInfo.totalElectricity =  bcdToU32(&fifo->packet.aData[4], 4);
		        fifo->meterInfo.surplusElectricity =  bcdToU32(&fifo->packet.aData[8], 4);
		        fifo->meterInfo.overdraftElectricity =  bcdToU32(&fifo->packet.aData[12], 4);
		        fifo->meterInfo.lastrRecharge =  bcdToU32(&fifo->packet.aData[16], 4);
		        fifo->meterInfo.totalRecharge =  bcdToU32(&fifo->packet.aData[20], 4);
		        fifo->meterInfo.RechargeNumber =  bcdToU32(&fifo->packet.aData[24], 2);
		        fifo->meterInfo.lightWarnValue =  bcdToU32(&fifo->packet.aData[26], 4);
		        fifo->meterInfo.offWarnValue =  bcdToU32(&fifo->packet.aData[30], 4);
		        fifo->meterInfo.cornerValue = bcdToU32(&fifo->packet.aData[34], 4);
		        fifo->meterInfo.creditValue = bcdToU32(&fifo->packet.aData[38], 4);
		        fifo->meterInfo.limitingValue = bcdToU32(&fifo->packet.aData[42], 4);
		        fifo->meterInfo.state = fifo->packet.aData[46];
		        fifo->meterInfo.state |= (fifo->packet.aData[46] << 8);
		        
		        log_debug("meterInfo.totalElectricity = %d\r\n", fifo->meterInfo.totalElectricity);
		        log_debug("meterInfo.surplusElectricity = %d\r\n", fifo->meterInfo.surplusElectricity);
						log_debug("meterInfo.overdraftElectricity = %d\r\n", fifo->meterInfo.overdraftElectricity);
						log_debug("meterInfo.lastrRecharge = %d\r\n", fifo->meterInfo.lastrRecharge);
						log_debug("meterInfo.totalRecharge = %d\r\n", fifo->meterInfo.totalRecharge);
						log_debug("meterInfo.RechargeNumber = %d\r\n", fifo->meterInfo.RechargeNumber);
						log_debug("meterInfo.lightWarnValue = %d\r\n", fifo->meterInfo.lightWarnValue);
						log_debug("meterInfo.offWarnValue = %d\r\n", fifo->meterInfo.offWarnValue);
						log_debug("meterInfo.cornerValue = %d\r\n", fifo->meterInfo.cornerValue);
						log_debug("meterInfo.creditValue = %d\r\n", fifo->meterInfo.creditValue);
						log_debug("meterInfo.limitingValue = %d\r\n", fifo->meterInfo.limitingValue);
						log_debug("meterInfo.state = %d\r\n", fifo->meterInfo.state);
		       
		        log_debug("ELEC_METER_R_DATA \r\n");
		        
		        if (fifo->reportCount > 0)
		        {
		            uint32_t i;
		            
		            for (i = 0; i < fifo->reportCount; i++)
		            {
		                meter_report(epInfo, &fifo->meterInfo, fifo->reportCMD[i]);
		            }
		            fifo->reportCount = 0;
		        }
		    }
		    break;

				default:break;
				}
			}
	}
  log_debug("\r\n**********************************\r\n");
}

int meter_ResolveWriteMsg(epInfo_t *epInfo,serial_fifo_t *fifo)
{
	ASSERT(epInfo != NULL && fifo != NULL);
    
    log_debug("write ctrl %x\r\n", fifo->packet.nCtrlNumber);
    
    uint8_t i;
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    
    
    for (i = 0; i < fifo->respCount; i++)
    {
        cmdSet16bitVal(&cmd,WIFI_METER_DEVICE_ID | (fifo->writeRespCMD[i] & 0xff));
        cmdSetStringVal(&cmd, epInfo->IEEEAddr,8);
        cmdSet8bitVal(&cmd,epInfo->endpoint);
        
        switch(fifo->writeRespCMD[i])
        {
            case SERVER_CMD_MODE_REPORT:            
            case SERVER_CMD_STATE_REPORT:
            {
                if (fifo->packet.nCtrlNumber & CONTROL_FIELD_SLAVE_CFM)
                {
                    cmdSet8bitVal(&cmd,0x01);
                    cmdSet8bitVal(&cmd,0x01);
                    makeMsgEnder(&cmd);
                    cmdMsgSend(cmd.data,cmd.idx);
                }
                else
                {
                    meter_read(epInfo, ELEC_METER_R_DATA);
                    fifo->reportCMD[fifo->reportCount++] = fifo->writeRespCMD[i];
                }
                
            }
            break;

            case SERVER_CMD_RESET_REPORT:
            {
                if (fifo->packet.nCtrlNumber & CONTROL_FIELD_SLAVE_CFM)
                {
                    cmdSet8bitVal(&cmd,0x01);
                }
                else
                {
                    cmdSet8bitVal(&cmd,0x00);
                }
                makeMsgEnder(&cmd);
                cmdMsgSend(cmd.data,cmd.idx);
            }
            break;
            
            case SERVER_CMD_RECHARGE_REPORT:
            {
                
                cmdSet8bitVal(&cmd,0x00);
                if (fifo->packet.nCtrlNumber & CONTROL_FIELD_SLAVE_CFM)
                {
                    cmdSet8bitVal(&cmd,0x01);
                    cmdSet32bitVal(&cmd, fifo->meterInfo.surplusElectricity);
                }
                else
                {
                    cmdSet8bitVal(&cmd,0x00);
                    cmdSet32bitVal(&cmd, fifo->meterInfo.totalRecharge-fifo->meterInfo.totalElectricity);
                }
                
                //cmdSet32bitVal(&cmd, fifo->meterInfo.totalRecharge-fifo->meterInfo.totalElectricity);

                makeMsgEnder(&cmd);
                cmdMsgSend(cmd.data,cmd.idx);
            }
            break;
        }
    }
    fifo->respCount = 0;
	
}

int meter_ProtocolResolve(epInfo_t *epInfo,serial_fifo_t *fifo)
{
	uint8_t index = 0;
	uint8_t sof1 = 0; 
	uint8_t sof2 = 0;
	uint8_t ctrl = 0;
	uint8_t len = 0;
	int8_t cnt = 0;
	ASSERT(epInfo != NULL && fifo != NULL);
	
	sof1 = fifo->data[fifo->idx++];

	for(cnt = 0;cnt < 6;cnt++)
	{
		fifo->meter_addrs[cnt] = meter_addrs[cnt]= fifo->data[fifo->idx++];
		//meter_addrs[cnt] = fifo->data[fifo->idx++];
	}

	log_debug("dev Addr:%x:%x:%x:%x:%x:%x\n",meter_addrs[0],meter_addrs[1],meter_addrs[2],meter_addrs[3],meter_addrs[4],meter_addrs[5]);
	
	sof1 = fifo->data[fifo->idx++];
	ctrl = fifo->data[fifo->idx++];

	if(!(ctrl & CONTROL_FIELD_DIR))
	{
		log_err("Msg from host\n");
		return;
	}
    /*
	if(ctrl & CONTROL_FIELD_SLAVE_CFM)
	{
		log_err("Slave Exception respone\n");
		return ;
	}*/

    log_debug("ctrl number %x\r\n", (ctrl&CONTROL_FIELD_FUNCTION));
	switch((ctrl&CONTROL_FIELD_FUNCTION))
	{
		case CONTROL_FIELD_FUN_READ:
		{
			meter_ResolveReadMsg(epInfo,fifo);
		}
		break;
		case CONTROL_FIELD_FUN_WRITE:
		{
			meter_ResolveWriteMsg(epInfo,fifo);
		}
		break;
		default:
		break;
	}
}

//串口字节流处理
int meter_SerialMsgProcess(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t len = 0;
	uint8_t chr = 0;

	ASSERT(epInfo != NULL && cmd != NULL);
	log_debug("\r\n**********************************\r\n");
	log_debug("meter_SerialMsgProcess++\n");

	if (fifoManage == NULL)
	{
		fifoManage = HandleManage_Create(100, sizeof(serial_fifo_t), 8);
	}

	serial_fifo_t *serial_fifo = HandleMange_GetHadnle(fifoManage, epInfo->IEEEAddr);

	log_debug("serial fifo %x \r\n", serial_fifo);
	cmdGet8bitVal(cmd,&len);
	while(len--)
	{
		cmdGet8bitVal(cmd,&chr);
		switch(serial_fifo->state)
		{
			case SOF_STATE:
			{
				if(chr == 0x68)
				{
					//memset(serial_fifo,0,sizeof(serial_fifo));
          			serial_fifo->idx = 0;
					serial_fifo->state = ADDR_STATE;
					serial_fifo->data[serial_fifo->idx++] = chr;
					serial_fifo->tempDataLen = 6;
				}
				else if(chr == 0XFF)
				{
					return ;
				}
			}
			break;
			case ADDR_STATE:
			{	
				serial_fifo->data[serial_fifo->idx++] = chr;
                
        		serial_fifo->packet.aAddress[6-serial_fifo->tempDataLen] = chr;
                
				serial_fifo->tempDataLen--;
				if(serial_fifo->tempDataLen == 0)
					serial_fifo->state = SOF_STATE2;
			}
			break;
			case SOF_STATE2:
			{
				if(chr == 0x68)
				{
						serial_fifo->data[serial_fifo->idx++] = chr;
						serial_fifo->state = CTRL_STATE;
				}
				else
				{
					//memset(serial_fifo,0,sizeof(serial_fifo));
					serial_fifo->state = SOF_STATE;
					//return;
				}
			}
			break;
			case CTRL_STATE:
			{
        		serial_fifo->packet.nCtrlNumber = chr;
				serial_fifo->data[serial_fifo->idx++] = chr;
				serial_fifo->state = LEN_STATE;
			}
			break;
			case LEN_STATE:
			{
        		serial_fifo->packet.nDataLength = chr;
                
				serial_fifo->len = chr;
				serial_fifo->data[serial_fifo->idx++] = chr;
				
				serial_fifo->tempDataLen =chr ;
		        if (serial_fifo->len == 0)
		        {
		            serial_fifo->state = FCS_STATE;
		        }
		        else
		        {
		            serial_fifo->state = DATA_STATE;
		        }
		        log_debug("data length %d\r\n", serial_fifo->len);
			}
			break;
			case DATA_STATE:
			{
				serial_fifo->data[serial_fifo->idx++] = chr;
        		serial_fifo->packet.aData[serial_fifo->packet.nDataLength - serial_fifo->tempDataLen] = ((chr - 0x33) & 0xff);
				serial_fifo->tempDataLen--;
				if(serial_fifo->tempDataLen == 0)
					serial_fifo->state = FCS_STATE;
			}
			break;
			case FCS_STATE:
			{
				uint8_t fcs = chr;
				if(meter_fcs(serial_fifo->data,serial_fifo->idx) == fcs)
				{
					log_info("Meter Succeful\n");
					serial_fifo->len = serial_fifo->idx+1;
					serial_fifo->idx = 0;
					meter_ProtocolResolve(epInfo, serial_fifo);
				}
				
				//memset(serial_fifo,0,sizeof(serial_fifo));
				serial_fifo->state = SOF_STATE;
			}
			break;   
			case END_STATE:
				
			break;
			default:
            break;
		}	
	}
	log_debug("meter_SerialMsgProcess-- %x\n",serial_fifo->state);
  log_debug("\r\n**********************************\r\n");
}
/******************************************************************
  * @函数说明：   服务端命令处理
  * @输入参数：   hostCmd *cmd,CMD
  *               epInfo_t *epInfo 端点信息
  * @输出参数：   
  * @返回参数：           
  * @修改记录：   
******************************************************************/
void meter_serverProcess(uint16_t nCMD, epInfo_t *epInfo, hostCmd *cmd)
{
    log_debug("\r\n****************\r\n");
    
    log_debug("cmd %x\r\n", nCMD & 0xff);
    if (fifoManage == NULL)
    {
        fifoManage = HandleManage_Create(100, sizeof(serial_fifo_t), 8);
    }
    
    serial_fifo_t *serial_fifo = HandleMange_GetHadnle(fifoManage, epInfo->IEEEAddr);
        
    switch(nCMD & 0xff)
    {
        case SERVER_CMD_ELECTRICITY_QUERY:
        {
            uint8_t type;
            cmdGet8bitVal(cmd, &type);
            if (type == 00)
            {
                meter_read(epInfo, ELEC_METER_RW_TOTAL_ELEC);
            }
            else if (type == 1)
            {
                meter_read(epInfo, ELEC_METER_R_TOTAL_BUY_ELEC);
            }
            else if (type == 2)
            {
                meter_read(epInfo, ELEC_METER_R_SURPLUS_ELEC);
            }
			else if(type == 3)
			{
				meter_read(epInfo, ELEC_METER_R_OVERDRAFT_ELEC);
			}
        }
        break;
        
        case SERVER_CMD_STATE_CTRL:
        {
            uint8_t mode;
            cmdGet8bitVal(cmd, &mode);
            if(mode == 0)
            {
                mode = 0x1b;
            }
            else
            {
                mode = 0x1a;
            }
            
            meter_write(epInfo, ELEC_METER_W_REMOTE_CTRL, &mode, 1);
            serial_fifo->writeRespCMD[serial_fifo->respCount++] = SERVER_CMD_STATE_REPORT;
        }
        break;
        
        case SERVER_CMD_STATE_QUERY:
        {
            meter_read(epInfo, ELEC_METER_R_DATA);
            serial_fifo->reportCMD[serial_fifo->reportCount++] = SERVER_CMD_STATE_REPORT;
        }
        break;
        case SERVER_CMD_STATE_REPORT:
        {
        
        }
        break;
        case SERVER_CMD_MODE_CTRL:
        {
            uint8_t mode;
            cmdGet8bitVal(cmd, &mode);
            if(mode == 0)
            {
                mode = 0x3b;
            }
            else
            {
                mode = 0x3a;
            }
            meter_write(epInfo, ELEC_METER_W_REMOTE_CTRL, &mode, 1);
            serial_fifo->writeRespCMD[serial_fifo->respCount++] = SERVER_CMD_MODE_REPORT;
        }
        break;
        case SERVER_CMD_MODE_QUERY:
        {
            meter_read(epInfo, ELEC_METER_R_DATA);
            serial_fifo->reportCMD[serial_fifo->reportCount++] = SERVER_CMD_MODE_REPORT;
        }
        break;
        case SERVER_CMD_MODE_REPORT:
        {
        
        }
        break;
        case SERVER_CMD_RECHARGE:
        {
            cmdGet32bitVal(cmd, &serial_fifo->meterInfo.rechargeValue);
            log_debug("recharge %d\r\n", serial_fifo->meterInfo.rechargeValue);
            meter_read(epInfo, ELEC_METER_R_DATA);
            serial_fifo->reportCMD[serial_fifo->reportCount++] = SERVER_CMD_RECHARGE;
            serial_fifo->writeRespCMD[serial_fifo->respCount++] = SERVER_CMD_RECHARGE_REPORT;
        }
        break;
        case SERVER_CMD_RECHARGE_REPORT:
        {
        
        }
        break;
        case SERVER_CMD_RESET:
        {
            uint8_t mode = 0xaa;
            meter_write(epInfo, ELEC_METER_W_CLEAR, &mode, 1);
            serial_fifo->writeRespCMD[serial_fifo->respCount++] = SERVER_CMD_RESET_REPORT;
        }
        break;
        case SERVER_CMD_RESET_REPORT:
        {
            
        }
        break;
    }
    log_debug("\r\n****************\r\n");
}


