/***********************************************************************************
 * 文 件 名   : waterMeter.c
 * 负 责 人   : 
 * 创建日期   : 2017年02月23日
 * 文件描述   : 水表，串口透传数据处理
 * 版权说明   : Copyright (c) 2008-2016   无锡飞雪网络 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"
#include "math.h"
#include "zbSocPrivate.h"
#include "interface_srpcserver_defs.h"
#include "waterMeter_xx.h"
#include "waterMeterUartFormat_xx.h"


#define METER_TYPE 0X10

typedef enum /*控制码，主站。从站控制码 = 主站控制码 + 0x80*/
{
    UART_CTRL_NUMBER_BALANCE = 0,   //读取剩余金额
    UART_CTRL_NUMBER_RESIDUE_WATER,   //剩余水量
    UART_CTRL_NUMBER_RECHARGE,   //充值
    UART_CTRL_NUMBER_LADDER_PRICE,   //阶梯价格
    UART_CTRL_NUMBER_PRICE_MODE,   //价格模式
    UART_CTRL_NUMBER_ENABLE,   //价格模式
    UART_CTRL_NUMBER_WARN_VALUE,   //警告
    UART_CTRL_NUMBER_READ_ADDRESS,   //读地址
    UART_CTRL_NUMBER_READ_STATE,    //读状态
    UART_CTRL_NUMBER_READ_ALL,
    UART_CTRL_NUMBER_END,
}UART_CTRL_NUMBER;

/*控制码*数据标识*/
const uint8_t UART_DI_TABLE[][3] = /*数据标识的表*/
{
    {0X01, 0X13, 0X82}, //余额
    {0X01, 0X15, 0X82}, //剩余水量
    {0X04, 0X21, 0XA1}, //充值
    {0X04, 0X23, 0XA1}, //写入阶梯价格
    {0X04, 0X22, 0XA1}, //写入价格模式
    {0X04, 0X42, 0XA0}, //设备控制
    {0X04, 0X24, 0XA1}, //警告
    {0X03, 0X0A, 0X81}, //度读取设备地址
    {0X01, 0X38, 0XD1}, //读状态
    {0X01, 0X14, 0X82},//读取所有数据
};

typedef enum 
{
    WATER_METER_CMD_RESET = 1,      //恢复初上设置
    WATER_METER_CMD_RESET_REPORT,
    WATER_METER_CMD_QUERY_RESIDUE_WATER,  //剩余水量
    WATER_METER_CMD_RECHARGE,       //充值
    WATER_METER_CMD_RESIDUE_WATER_REPORT,
    WATER_METER_CMD_QUERY_PRICE_MODE,   //价格模式
    WATER_METER_CMD_PRICE_MODE,     
    WATER_METER_CMD_PRICE_MODE_REPORT,
    WATER_METER_CMD_QUERY_STATE_CTRL,
    WATER_METER_CMD_STATE_CTRL,         //状态控制
    WATER_METER_CMD_STATE_CTRL_REPORT,
    WATER_METER_CMD_WARN_REPORT,        //低水量报警
}WATER_METER_CMD_T;

uint8_t waterMeterAddress[PACKET_ADDRESS_LENGTH] = {0};

void waterMeter_SetCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t *pData, uint8_t nLength);
/******************************************************************
  * @函数说明：   查表设置数据包的数据标识
  * @输入参数：   UART_FORMAT_PACKET_T *pPacket 数据包，要先填入
  *               控制码
  * @输出参数：   无 
  * @返回参数：   bool 成功true 失败 false   
  * @修改记录：   
******************************************************************/
UART_FORMAT_PACKET_T waterMeter_InitPacket(UART_CTRL_NUMBER ctrlNum)
{
    uint8_t i;
    UART_FORMAT_PACKET_T packet;
    if (ctrlNum == UART_CTRL_NUMBER_READ_ADDRESS)
    {
        memset(packet.aAddress, 0xaa, PACKET_ADDRESS_LENGTH);   //使用广播地址
    }
    else 
    {
        memcpy(packet.aAddress, waterMeterAddress, PACKET_ADDRESS_LENGTH);
    }
    
    packet.nMeterType = METER_TYPE;
    packet.nLength = 3;
    
    packet.nCtrlNumber = UART_DI_TABLE[ctrlNum][0];
    packet.aData[0] = UART_DI_TABLE[ctrlNum][1];
    packet.aData[1] = UART_DI_TABLE[ctrlNum][2];

    packet.aData[2] = 0X01; //SER 
    
    return packet;
}

void waterMeter_SendPacket(epInfo_t *epInfo, UART_FORMAT_PACKET_T packet)
{
    uint8_t buff[sizeof(UART_FORMAT_PACKET_T) + 3];
    
    int length = uartFormat_BuilPacket(buff, &packet);
    
    if (length > 0)
    {
        waterMeter_SetCmd(epInfo->nwkAddr, epInfo->endpoint, afAddr16Bit, buff, (uint32_t)length);
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
       // log_debug("bcd %x \r\n", buf[i]);
        if (data == 0)
        {
            length = i;
           // log_debug("length = %d\r\n", i);
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
        log_debug("temp %d\r\n", temp);
        temp += (((buf[i] >> 4) % 10) * 10);
        log_debug("temp %d\r\n", temp);
        data += temp * pow(10, (i) * 2);
        log_debug(" bcd to 32 %x, %d\r\n", buf[i], data);
    }
    return data;
}

/******************************************************************
  * @函数说明：   查询水表的余额
  * @输入参数：   无
  * @输出参数：   无 
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_QueryBalance(epInfo_t *epInfo)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_BALANCE);
        
    waterMeter_SendPacket(epInfo, packet);
}

/******************************************************************
  * @函数说明：   查询剩余水量
  * @输入参数：   epInfo_t *epInfo 端点信息
  * @输出参数：   无 
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_QueryResidueWater(epInfo_t *epInfo)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_RESIDUE_WATER);
    
    waterMeter_SendPacket(epInfo, packet);
}
/******************************************************************
  * @函数说明：   充值
  * @输入参数：   epInfo_t *epInfo 端点信息
  *               int money 充值金额
  * @输出参数：   无 
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_Recharge(epInfo_t *epInfo, int money)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_RECHARGE);
    packet.aData[packet.nLength++] = 0x01; //购买序号
    packet.aData[packet.nLength++] = (uint8_t)(money & 0xff);
    packet.aData[packet.nLength++] = (uint8_t)((money >> 8) & 0xff);
    packet.aData[packet.nLength++] = (uint8_t)((money >> 16) & 0xff);
    packet.aData[packet.nLength++] = (uint8_t)((money >> 24)& 0xff);
 
    waterMeter_SendPacket(epInfo, packet);
}

/******************************************************************
  * @函数说明：   写入阶梯价格
  * @输入参数：   epInfo_t *epInfo 端点信息
  *               uint16_t price[5][2] 阶梯价格
  * @输出参数：   无 
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_LadderPrice(epInfo_t *epInfo)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_LADDER_PRICE);
    packet.aData[packet.nLength++] = 0x0B; //价格模式 不启用保底，阶梯周期为月，五个阶梯， 启用
    packet.aData[packet.nLength++] = 0x03; //价格模式，禁用透支，启用欠费关闭，启用警告
    
    packet.aData[packet.nLength++] = 0x00;  //计量周期总金额
    packet.aData[packet.nLength++] = 0x00;
    packet.aData[packet.nLength++] = 0x00;
    packet.aData[packet.nLength++] = 0x00;
    
    packet.aData[packet.nLength++] = 0x00;  //计量周期结算流量
    packet.aData[packet.nLength++] = 0x00;
    packet.aData[packet.nLength++] = 0x00;
    packet.aData[packet.nLength++] = 0x00;
    
    packet.aData[packet.nLength++] = 0x00;  //报警水量
    packet.aData[packet.nLength++] = 0x00;
    
    packet.aData[packet.nLength++] = 0x00;  //透支数量
    packet.aData[packet.nLength++] = 0x00;
    
    packet.aData[packet.nLength++] = 0x00;  //保底价
    packet.aData[packet.nLength++] = 0x00;
    
    packet.aData[packet.nLength++] = 0x00;  //保底水量
    packet.aData[packet.nLength++] = 0x00;

    uint8_t i;
    packet.aData[packet.nLength++] = (uint8_t)(100 & 0xff);
    packet.aData[packet.nLength++] = (uint8_t)(100 >> 8 & 0xff);    //价格
    packet.aData[packet.nLength++] = 0xff;
    packet.aData[packet.nLength++] = 0xff;   //水量
        
    for (i = 1; i < 4; i++)
    {
        packet.aData[packet.nLength++] = 0;//(uint8_t)(price[5][0] & 0xff);
        packet.aData[packet.nLength++] = 0;//(uint8_t)(price[5][0] >> 8 & 0xff);    //价格
        packet.aData[packet.nLength++] = 0;//(uint8_t)(price[5][1] & 0xff);
        packet.aData[packet.nLength++] = 0;//(uint8_t)(price[5][1] >> 8 & 0xff);    //水量
    }
    packet.aData[packet.nLength++] = 0;
    packet.aData[packet.nLength++] = 0;    //价格
    
    waterMeter_SendPacket(epInfo, packet);
}
/******************************************************************
  * @函数说明：   写入价格模式
  * @输入参数：   epInfo_t *epInfo 端点信息
  *               uint16_t price[5][2] 阶梯价格
  * @输出参数：   无 
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_DeviceCtrl(epInfo_t *epInfo, uint8_t enable)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_ENABLE);
    if (enable == 1)
    {
        packet.aData[packet.nLength++] = 0x55; //自由控制
    }
    else if (enable == 2)
    {
        packet.aData[packet.nLength++] = 0x00; //自由控制
    }
    else
    {
        packet.aData[packet.nLength++] = 0x99; //强制关阀 
    }
    packet.aData[packet.nLength++] = 0x00; 
    waterMeter_SendPacket(epInfo, packet);
}

/******************************************************************
  * @函数说明：   写入价格模式
  * @输入参数：   epInfo_t *epInfo 端点信息
  *               uint16_t price[5][2] 阶梯价格
  * @输出参数：   无 
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_PriceMode(epInfo_t *epInfo, uint8_t mode)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_PRICE_MODE);
    if (mode == 1)
    {
        packet.aData[packet.nLength++] = 0x0B; //价格模式 不启用保底，阶梯周期为月，五个阶梯， 启用
        packet.aData[packet.nLength++] = 0x03; //价格模式，禁用透支，启用欠费关闭，启用警告
    }
    else
    {
        packet.aData[packet.nLength++] = 0x00; //价格模式 
        packet.aData[packet.nLength++] = 0x00; //价格模式
    }
    waterMeter_SendPacket(epInfo, packet);
}
/******************************************************************
  * @函数说明：   写入价格模式
  * @输入参数：   epInfo_t *epInfo 端点信息
  *               uint16_t price[5][2] 阶梯价格
  * @输出参数：   无 
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_WarnValue(epInfo_t *epInfo, uint16_t waterValue)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_WARN_VALUE);

    packet.aData[packet.nLength++] = waterValue & 0xff; //设备控制，自由控制，开阀
    packet.aData[packet.nLength++] = (uint8_t)((waterValue >> 8) & 0xff); //
    packet.aData[packet.nLength++] = 0; 
    packet.aData[packet.nLength++] = 0; 
    packet.aData[packet.nLength++] = 0; 
    packet.aData[packet.nLength++] = 0; 
    packet.aData[packet.nLength++] = 0; 
    packet.aData[packet.nLength++] = 0; 
    waterMeter_SendPacket(epInfo, packet);
}
/******************************************************************
  * @函数说明：   查询水表的余额
  * @输入参数：   无
  * @输出参数：   无 
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_ReadAddress(epInfo_t *epInfo)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_READ_ADDRESS);
        
    waterMeter_SendPacket(epInfo, packet);
}
/******************************************************************
  * @函数说明：   读取定期开关周期，获取状态ST
  * @输入参数：   epInfo_t *epInfo 端点信息
  * @输出参数：   
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_ReadState(epInfo_t *epInfo)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_READ_STATE);
        
    waterMeter_SendPacket(epInfo, packet);
}
/******************************************************************
  * @函数说明：   读取所有数据
  * @输入参数：   epInfo_t *epInfo 端点信息
  * @输出参数：   
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_ReadAll(epInfo_t *epInfo)
{
    UART_FORMAT_PACKET_T packet = waterMeter_InitPacket(UART_CTRL_NUMBER_READ_ALL);
        
    waterMeter_SendPacket(epInfo, packet);
}

/******************************************************************
  * @函数说明：   上报服务器
  * @输入参数：   epInfo_t *epInfo, 端点信息
  *               hostCmd &cmd,  数据内容，只包含要发送的数据
  *               uint8_t reportCMD 上报的命令
  * @输出参数：   无
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_report(epInfo_t *epInfo, hostCmd *dataCMD, uint8_t reportCMD)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_WATER_METER_DEVICE_ID | (reportCMD & 0xff));
    cmdSetStringVal(&cmd, epInfo->IEEEAddr,8);
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    
    cmdSetStringVal(&cmd, dataCMD->data, dataCMD->idx);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}
/******************************************************************
  * @函数说明：   设置数据包并且发送
  * @输入参数：   uint16_t dstAddr, 目标地址
  *               uint8_t endpoint, 端口号
  *               uint8_t addrMode, 地址模式
  *               uint8_t *pData,   数据
  *               uint8_t nLength   数据长度
  * @输出参数：   无
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_ReceiveProcess(epInfo_t *epInfo, UART_FORMAT_PACKET_T *packet)
{
    uint8_t di[3], i;
    di[0] = packet->nCtrlNumber - 0x80;
    di[1] = packet->aData[0];
    di[2] = packet->aData[1];
    
    for (i = 0; i < UART_CTRL_NUMBER_END; i ++)
    {
        if (memcmp(di, UART_DI_TABLE[i], 3) == 0)
        {
            memcpy(waterMeterAddress, packet->aAddress, PACKET_ADDRESS_LENGTH); //更新表地址
            log_debug("receive packet %x\r\n",i);
            break;
        }
    }
    hostCmd cmd;
    cmd.idx = 0;
    
    switch(i)
    {
        case UART_CTRL_NUMBER_BALANCE:
        {
            uint32_t value;
            //uint32_t value = bcdToU32(&packet->aData[11], 4);
            memcpy(&value, &packet->aData[11], 4);
            cmdSet8bitVal(&cmd, 0);
            cmdSet32bitVal(&cmd, value);
            waterMeter_report(epInfo, &cmd, WATER_METER_CMD_RESIDUE_WATER_REPORT);
        }
        break;
        
        case UART_CTRL_NUMBER_RESIDUE_WATER:
        {
            uint32_t value = bcdToU32(&packet->aData[3], 4);
            cmdSet8bitVal(&cmd, 0);
            cmdSet32bitVal(&cmd, value);
            waterMeter_report(epInfo, &cmd, WATER_METER_CMD_RESIDUE_WATER_REPORT);
        }
        break;
        
        case UART_CTRL_NUMBER_RECHARGE:
        {
            waterMeter_QueryResidueWater(epInfo);
        }
        break;
        
        case UART_CTRL_NUMBER_LADDER_PRICE:
        {
            
        }
        break;
        
        case UART_CTRL_NUMBER_PRICE_MODE:
        {
            waterMeter_ReadAll(epInfo);
        }
        break;
        
        case UART_CTRL_NUMBER_ENABLE:
        {
            waterMeter_ReadState(epInfo);
        }
        break;
        
        case UART_CTRL_NUMBER_WARN_VALUE:
        {
            waterMeter_DeviceCtrl(epInfo, 1);
        }
        break;
        
        case UART_CTRL_NUMBER_READ_ADDRESS:
        {
            waterMeter_WarnValue(epInfo, 1);
        }
        break;
        
        case UART_CTRL_NUMBER_READ_STATE:
        {
            cmdSet8bitVal(&cmd, 0);
            if ((packet->aData[3] & 0x03) > 0x01)
            {
                cmdSet8bitVal(&cmd, 0x02);
            }
            else
            {
                cmdSet8bitVal(&cmd, packet->aData[3] & 0x03);
            }
            waterMeter_report(epInfo, &cmd, WATER_METER_CMD_STATE_CTRL_REPORT);
        }
        break;
        
        case UART_CTRL_NUMBER_READ_ALL:
        {
            cmdSet8bitVal(&cmd, 0);
            log_debug_array(&packet->aData[45], 3, NULL);
            if (packet->aData[46] & 0x01)
            {
                cmdSet8bitVal(&cmd, 1);
            }
            else
            {
                cmdSet8bitVal(&cmd, 0);
            }
            waterMeter_report(epInfo, &cmd, WATER_METER_CMD_PRICE_MODE_REPORT);
        }
        break;
        
        default:
        {
            
        }
        break;
    }
}


/******************************************************************
  * @函数说明：   设置数据包并且发送
  * @输入参数：   uint16_t dstAddr, 目标地址
  *               uint8_t endpoint, 端口号
  *               uint8_t addrMode, 地址模式
  *               uint8_t *pData,   数据
  *               uint8_t nLength   数据长度
  * @输出参数：   无
  * @返回参数：   无       
  * @修改记录：   
******************************************************************/
void waterMeter_SetCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t *pData, uint8_t nLength)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_UART_MSG);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type
    
    cmdSet8bitVal(&cmd, nLength);//长度
    cmdSetStringVal(&cmd, pData, nLength);
    //cmdSet8bitVal(&cmd, deviceType);//设备类型，主控的第几路设备
	//cmdSet8bitVal(&cmd, switchCmd);//设备状态
   
    zbMakeMsgEnder(&cmd);
    
    zbSocCmdSend(cmd.data,cmd.idx);
}
/******************************************************************
  * @函数说明：   状态上报，处理上报的透传信息
  * @输入参数：   hostCmd *cmd,CMD
  *               epInfo_t *epInfo 端点信息
  * @输出参数：   无
  * @返回参数：   无         
  * @修改记录：   
******************************************************************/
void waterMeter_StateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);
    uint16_t attrID;
	uint8_t dataType;
    uint8_t length;
	uint8_t cnt = 0;
    

	ASSERT(epInfo != NULL && cmd != NULL);
	log_debug("\r\n\r\n**************************************************\n");
	log_debug("water meter  ReportResolve++*************\n");
    

	cmdGet16bitVal_lh(cmd, &attrID);
    cmdGet8bitVal(cmd, &dataType);
    
    log_debug("attrID %d\r\n", attrID);
    log_debug("dataType %d\r\n",dataType);
	//单个设备状态上报
	if((attrID == ATTRID_BASIC_UART_MSG) && (dataType == ZCL_DATATYPE_OCTET_STR))
	{
        uint8_t *buf;
        uint8_t i;
		cmdGet8bitVal(cmd, &length);//长度
        buf = malloc(length);
        
        cmdGetStringVal(cmd, buf, length);
        
        UART_FORMAT_HANDLE_T handle = uartFormat_GetHandleForId(epInfo->nwkAddr);

        UART_FORMAT_PACKET_T *packet = NULL;
        for (i = 0; i < length; i++)
        {
            packet = uartFormat_Resolve(handle, buf[i]);
            if (packet != NULL)    //解析成功
            {
                log_debug("Packet Resolve sucess %x\r\n", packet->nCtrlNumber);
                waterMeter_ReceiveProcess(epInfo, packet);
            }
        }
	}
    log_debug("**************************************************\n\r\n\r\n");
}
/******************************************************************
  * @函数说明：   心跳处理
  * @输入参数：   hostCmd *cmd,CMD
  *               epInfo_t *epInfo 端点信息
  * @输出参数：   无
  * @返回参数：   无         
  * @修改记录：   
******************************************************************/
void waterMeter_HeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
    log_debug("**************************************************\n\r\n\r\n");
    log_debug("water meter Heart Report\r\n");
    waterMeter_ReadAddress(epInfo);
    log_debug("**************************************************\n\r\n\r\n");
}
/******************************************************************
  * @函数说明：   
  * @输入参数：   hostCmd *cmd,CMD
  *               epInfo_t *epInfo 端点信息
  * @输出参数：   
  * @返回参数：           
  * @修改记录：   
******************************************************************/
void waterMeter_ReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t status;
	uint8_t dataType;
	//epInfo_t * pepInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &status);
	cmdGet8bitVal(cmd, &dataType);//get data type;
	
	//获取版本
	if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID == ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}

	log_debug("attrID = %d,dataType = %d\n",attrID ,dataType);

	if((attrID == ATTRID_MASTER_CONTROL_READ_VALUE) && (dataType == ZCL_DATATYPE_CHAR_STR))
	{
		zbSoc_MasterControlReadRspResolve(cmd,epInfo);
	}
}




/******************************************************************
  * @函数说明：   服务端命令处理
  * @输入参数：   hostCmd *cmd,CMD
  *               epInfo_t *epInfo 端点信息
  * @输出参数：   
  * @返回参数：           
  * @修改记录：   
******************************************************************/
void waterMeter_serverProcess(uint16_t nCMD, epInfo_t *epInfo, hostCmd *cmd)
{
    log_debug("\r\n***********************************\r\n");
    log_debug("water server cmd %x\r\n", nCMD);
    log_debug("WATER_METER_CMD_STATE_CTRL %x\r\n", WATER_METER_CMD_STATE_CTRL);
    switch(nCMD & 0xff)
    {
        case WATER_METER_CMD_RESET:      //恢复设置
        {
            waterMeter_LadderPrice(epInfo);
        }
        break;

        case WATER_METER_CMD_QUERY_RESIDUE_WATER:  //剩余水量
        {
            //waterMeter_QueryResidueWater(epInfo);
            waterMeter_QueryBalance(epInfo);
        }
        break;
        case WATER_METER_CMD_RECHARGE:       //充值
        {
            uint32_t value;
            cmdGet32bitVal(cmd, &value);
            waterMeter_Recharge(epInfo, value);
            log_debug("recharge value %d\r\n", value);
        }
        break;

        case WATER_METER_CMD_QUERY_PRICE_MODE:   //价格模式
        {
            waterMeter_ReadAll(epInfo);
        }
        break;
        case WATER_METER_CMD_PRICE_MODE:    
        {
            uint8_t mode;
            cmdGet8bitVal(cmd, &mode);
            waterMeter_PriceMode(epInfo, mode);
        }
        break;

        case WATER_METER_CMD_QUERY_STATE_CTRL:
        {
            waterMeter_ReadState(epInfo);
        }
        break;
        
        case WATER_METER_CMD_STATE_CTRL:         //状态控制
        {
            uint8_t mode;
            cmdGet8bitVal(cmd, &mode);
            log_debug("State ctrl %x\r\n",mode);
            waterMeter_DeviceCtrl(epInfo, mode);
        }
        break;

    }
    log_debug("\r\n***********************************\r\n");
}