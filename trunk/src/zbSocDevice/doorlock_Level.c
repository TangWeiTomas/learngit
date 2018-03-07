/***********************************************************************************
 * 文 件 名   : doorlock_Level.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年10月12日
 * 文件描述   : 力维门锁,串口信息透传数据处理
 * 版权说明   : Copyright (c) 2008-2016   无锡飞雪网络 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include "doorlock_Level.h"
#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"

#include "zbSocPrivate.h"
#include "interface_srpcserver_defs.h"
#include "doorlock.h"
#include "GwComDef.h"
#include "doorlock_zbSocCmds.h"

typedef struct
{
	uint8_t 	doorlockstu;		//门锁状态
	uint8_t 	openType;			//开门类型
	uint16_t 	userid;				//用户ID
	uint8_t 	batAlarmFalg;		//电池报警
	uint16_t 	dBattery;			//门锁电量
	uint8_t 	doublelock;			//反锁
	uint8_t 	timevail;			//时间有效性
	uint8_t 	time[6];			//时间
}Level_Stu_t;

typedef enum{
	ZBDL_CLOSE = 0x00,
	ZBDL_OPENCLOSE = 0x01,
	ZBDL_OPEN	   = 0xff
}Level_Open_Types;

typedef enum
{
	DOOR_CLOSE 		 = 0X00,
	DOOR_OPEN  		 = 0X01,
	DOOR_ALWAYS_OPEN = 0X02,
}Level_DoorState_Type;

typedef struct
{
	tu_evtimer_t *doorLevelTimer;
	epInfo_t epInfo;
	Level_Stu_t stu;
}Level_time_Types_t;

//协议相关
#define DOOR_CMD_USR_MNG							0x0e
#define ZBDL_MSG_SOC								0XAA

//力维门锁功能命令定义
#define DOORLOCK_LEVEL_CMD_OPEN_DEVICE  			0x01
#define DOORLOCK_LEVEL_CMD_READ_DEVICE_INFO			0x07
#define DOORLOCK_LEVEL_CMD_DEVICE_INFO				0X08
#define DOORLOCK_LEVEL_CMD_DEVICE_ALARM				0x0B
#define DOORLOCK_LEVEL_CMD_USER_MNG					0x0E
#define DOORLOCK_LEVEL_CMD_USR_REG_INFO				0x11
#define DOORLOCK_LEVEL_CMD_DEVICE_OPEN_RECORD		0x1C
#define DOORLOCK_LEVEL_CMD_STATUS_INFO				0x1D

//力维门锁用户管理功能定义(密码/指纹/门卡) 0x0E命令
#define LEVEL_USR_TYPE_PWD_ID						0X00	//密码
#define LEVEL_USR_TYPE_CARD_ID						0X04	//门卡
#define LEVEL_USR_TYPE_FINGER_ID					0X08	//指纹

//用户操作
#define LEVEL_OPT_TYPE_ONLAY_DEL					0X00	//单独删除 
#define LEVEL_OPT_TYPE_ONLAY_ADD					0X01  //单独增加
#define LEVEL_OPT_TYPE_DEL							0X02	//连续删除
#define LEVEL_OPT_TYPE_ALL_DEL						0X03  //全部删除

uint8_t doorLevel_OpenCB(epInfo_t *epInfo,uint8_t status);
uint8_t doorLevel_PowerValueCB(epInfo_t *epInfo);
uint8_t doorLevel_SetPINCodeCB(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen);
uint8_t doorLevel_GetDeviceInfo(epInfo_t *epInfo);
uint8_t doorLevel_SetRFIDCodeCB(epInfo_t *epInfo,uint8_t optType,uint16_t userid,uint8_t idlen,uint8_t *id);
uint8_t doorLevel_SetFINGERCodeCB(epInfo_t *epInfo,uint8_t optType,uint16_t userid);
uint8_t doorLevel_SetPollRateCB(epInfo_t *epInfo,uint16_t times);

zbSocDoorLockAppCallbacks_t zbSocDoorLockLVAppCallbacks = 
{
	doorLevel_OpenCB,
	doorLevel_PowerValueCB,
	doorLevel_GetDeviceInfo,
	doorLevel_SetPINCodeCB,
	doorLevel_SetRFIDCodeCB,
	doorLevel_SetFINGERCodeCB,
	NULL,
	NULL,
	NULL,
	doorLevel_SetPollRateCB
};

zbSocDoorLockCallbacks_t	zbSocDoorLock_LevelCallBack_t = 
{
	 ZB_DEV_LEVEL_DOORLOCK,
	 &zbSocDoorLockLVAppCallbacks 
};
//存放服务器下发下来的门锁状态命令
static Level_DoorState_Type doorStateFromServer =	 DOOR_CLOSE;

const char *RecodeType[] = {
	"授权卡",
	"时间卡",
	"安装卡",
	"后备卡",
	"数据卡",
	"退房卡",
	"终止卡",
	"挂失卡",
	"会议卡",
	"应急卡",
	"总卡",
	"大门卡",
	"区域卡",
	"楼栋卡",
	"楼层卡",
	"宾客卡",//15
	"无效",
	"无效",
	"普通卡",//18
	"无效",
	"无效",
	"密码卡",//21
	"指纹卡",
	"无效",//23
	"远程开门",
	"无效卡开门",
	"无效",//
	"系统启动",
	"记录卡",
	"零时宾客卡",
	"门内把手开门",
	"机械锁开门"
};

static Level_Stu_t  level_stu;
static uint8_t Level_Operate = LEVEL_USR_TYPE_PWD_ID ;

/*****************************************************************************
 * 函 数 名  : doorLevel_setOnOff
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月12日
 * 函数功能  : 生成level门锁开门命令
 * 输入参数  : hostCmd *cmd             数据缓冲区
               Level_Open_Types status  开门类型
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_setOnOffCmd(hostCmd *cmd,Level_Open_Types status)
{
	uint8_t cmdOpt = status;
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, DOORLOCK_LEVEL_CMD_OPEN_DEVICE);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	if(status == 0x02)
		cmdOpt = 0xff; //常开

	//开门参数
	cmdSet8bitVal(cmd, cmdOpt);

	cmdSetCheckSum(cmd);
}

/*****************************************************************************
 * 函 数 名  : doorLevel_getInfo
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月12日
 * 函数功能  : 生成获取门锁信息指令
 * 输入参数  : hostCmd *cmd  数据缓存区
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_getInfoCmd(hostCmd *cmd)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, DOORLOCK_LEVEL_CMD_READ_DEVICE_INFO);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//参数，可以不用设置
	cmdSet8bitVal(cmd, 0x00);

	cmdSetCheckSum(cmd);
}

static void doorLevel_settimeCmd(hostCmd *cmd,uint8_t years,uint8_t month,uint8_t dates,uint8_t hours,uint8_t minute,uint8_t seconds,uint8_t week)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x12);
	cmdSet8bitVal(cmd, 0x0a);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	//保留参数1
	cmdSet8bitVal(cmd, 0x00);

	//参数，时间
	cmdSet8bitVal(cmd, years);
	cmdSet8bitVal(cmd, month);
	cmdSet8bitVal(cmd, dates);
	cmdSet8bitVal(cmd, hours);
	cmdSet8bitVal(cmd, minute);
	cmdSet8bitVal(cmd, seconds);
	cmdSet8bitVal(cmd, week);

	cmdSetCheckSum(cmd);

}

static void doorLevel_getRegisterInfoCmd(hostCmd *cmd,uint8_t registerType)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, 0x10);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//列表类型
	cmdSet8bitVal(cmd, registerType);

	cmdSetCheckSum(cmd);
}

static int doorLevel_SetUsrMngCmd(hostCmd *cmd,uint8_t usrType,uint8_t optType,uint16_t usrid,uint8_t *passwd)
{
	uint8_t mUsrtType = 0;
	uint8_t mData[4] = {0};

	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x13);
	cmdSet8bitVal(cmd, DOORLOCK_LEVEL_CMD_USER_MNG);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	if(optType <0 && optType > 3)
		return FAILED;

	if(usrType >LEVEL_USR_TYPE_FINGER_ID)
		return FAILED;

	mUsrtType = usrType;
	mUsrtType |= optType;
	cmdSet8bitVal(cmd, mUsrtType);
	switch(optType)
	{
		case LEVEL_OPT_TYPE_ONLAY_DEL://单独删除
		{
			cmdSet16bitVal(cmd, usrid);
			cmdSetStringVal(cmd,&mData[0],4);
		}
		break;
		case LEVEL_OPT_TYPE_ONLAY_ADD://单独增加
		{
			if(mUsrtType != LEVEL_USR_TYPE_CARD_ID && mUsrtType != LEVEL_USR_TYPE_FINGER_ID)
			{
				cmdSet16bitVal(cmd, usrid);
				//cmdSetStringVal(cmd,passwd,4);

				uint8_t pwd = passwd[0]<<4 | passwd[1];
				cmdSet8bitVal(cmd,pwd);
				pwd = passwd[2]<<4 | passwd[3];
				cmdSet8bitVal(cmd,pwd);
				pwd = passwd[4]<<4 | passwd[5];
				cmdSet8bitVal(cmd,pwd);
				pwd = passwd[6]<<4 | passwd[7];
				cmdSet8bitVal(cmd,pwd);	
			}
			else
			{
				return YY_STATUS_UNSUPPORT_PARAM;
			}
		}
		break;
		case LEVEL_OPT_TYPE_DEL://连续删除
		{
			cmdSet16bitVal(cmd, usrid);
			cmdSetStringVal(cmd,&mData[0],4);
		}
		break;
		case LEVEL_OPT_TYPE_ALL_DEL://连续删除
		{
			cmdSet16bitVal(cmd, usrid);
			cmdSetStringVal(cmd,&mData[0],4);
		}
		break;
		default:
			return FAILED;
	}

	//保留
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	cmdSetCheckSum(cmd);
	return SUCCESS;
}


static void doorLevel_usrMngCmd(hostCmd *cmd,uint8_t usrType,uint8_t optType,uint32_t usrid,uint8_t *passwd)
{

	uint8_t usr = 0;
	uint8_t opt = 0;
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x13);
	cmdSet8bitVal(cmd, 0x0e);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//参数1
	cmdSet8bitVal(cmd, usrType&0x0c);
	cmdSet8bitVal(cmd, optType&0x03);

	usr = usrType&0x0c;
	opt = optType&0x03;
	
	cmdSetCheckSum(cmd);
}

/*****************************************************************************
 * 函 数 名  : doorLevel_SendSerialCmd
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月12日
 * 函数功能  : 发送力维透传数据
 * 输入参数  : uint16_t dstAddr  目标地址
               uint8_t endpoint  目标端口号
               uint8_t addrMode  发送类型
               uint8_t len       数据长度
               uint8_t* buf      数据
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_SendSerialCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t len,uint8_t* buf)
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


static void doorLevel_doorInfoInd(epInfo_t *epInfo,uint8_t *times,uint16_t dversion,uint16_t sversion,uint8_t doorbellstate,uint8_t opentype)
{
	log_debug("doorLevel_doorInfoInd");
	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_INFO_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11-D17 控制结果
    cmdSetStringVal(&cmd,times,7);
	//D18-D19 门锁硬件版本
	cmdSet16bitVal(&cmd,dversion);
	//D20-D21 门锁软件版本
	cmdSet16bitVal(&cmd,sversion);
	//D22 门铃状态
	cmdSet8bitVal(&cmd,doorbellstate);
	//D23 支持开锁方式
	cmdSet8bitVal(&cmd,opentype);
    
    makeMsgEnder(&cmd);
    
    cmdMsgSend(cmd.data,cmd.idx);
}


/*****************************************************************************
 * 函 数 名  : doorLevel_doorStateInd
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 门锁开关状态上报
 * 输入参数  : epInfo_t *epInfo  节点信息
               uint8_t state       当前状态
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void doorLevel_doorStateInd(epInfo_t *epInfo,uint8_t state)
{
	log_debug("doorLevel_doorStateInd++\n");
	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_STATUS_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,state);
    //D12 信号质量
    cmdSet8bitVal(&cmd,epInfo->onlineDevRssi);
    makeMsgEnder(&cmd);
    
    cmdMsgSend(cmd.data,cmd.idx);
}

//门锁开门日志上报
void doorLevel_realRecordInd(epInfo_t *epInfo,uint8_t state,uint8_t cardtype,uint8_t *uid,uint8_t *times)
{
	log_debug("doorLevel_realRecordInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_RECORD_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁状态
    cmdSet8bitVal(&cmd,state);
    //D12 开门类型
    cmdSet8bitVal(&cmd,cardtype);
    //D13-D16 UID0-UID3
    cmdSetStringVal(&cmd,uid,4);
    //D17-D22 门锁时间
    cmdSetStringVal(&cmd,times,6);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
   
}
//门锁电量状态上报
void doorLevel_alarmPowerInd(epInfo_t *epInfo,uint8_t doorPowerState,uint8_t wirelessPowerState)
{
	log_debug("doorLevel_alarmPowerInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_POWER_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁电压
    cmdSet8bitVal(&cmd,doorPowerState);
    //D12 无线电压
    cmdSet8bitVal(&cmd,wirelessPowerState);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void doorLevel_alarmOperationInd(epInfo_t *epInfo,uint8_t alarmType,uint8_t *times)
{
	log_debug("doorLevel_alarmOperationInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_ALARMOPT_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 非法操作类型
    cmdSet8bitVal(&cmd,alarmType);
    //D12-D18 非法操作时间
    cmdSetStringVal(&cmd,times,7);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static void doorLevel_registerInfoInd(epInfo_t *epInfo,uint8_t regType,uint8_t*uid,uint8_t udisize)
{
	log_debug("doorLevel_registerInfoInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_LEVEL_DOOR_REG_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 类型
    cmdSet8bitVal(&cmd,regType);
	//UID登记信息
    cmdSet8bitVal(&cmd,udisize);
    //D12-D18 非法操作时间
    cmdSetStringVal(&cmd,uid,udisize);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void doorLevel_UsrMngStateInd(epInfo_t *epInfo,uint8_t state)
{
	log_debug("doorLevel_alarmOperationInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ADD_LEVEL_DOOR_USR_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 非法操作类型
    cmdSet8bitVal(&cmd,state);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


//0x1c门锁开门实时记录
static void doorLevel_timerGetInfoHandle(void *args)
{
	ASSERT(args != NULL);
	log_debug("doorLevel_timerGetInfoHandle++\n");

	Level_time_Types_t *Leveltimer = (Level_time_Types_t*)args;
	doorLevel_getInfoReq(&(Leveltimer->epInfo));
	
	tu_evtimer_free(Leveltimer->doorLevelTimer);
	
	free(Leveltimer);
	
	log_debug("doorLevel_timerGetInfoHandle++\n");
}

static void doorLevel_timerGetInfo(epInfo_t *epInfo)
{
	ASSERT(epInfo != NULL);
	
	Level_time_Types_t *LevelTime = malloc(sizeof(Level_time_Types_t));
	
	if (LevelTime == NULL)
	{
		log_err("malloc failed\n");
		return;
	}
	
	memcpy(&(LevelTime->epInfo),epInfo,sizeof(epInfo_t));
	
	LevelTime->doorLevelTimer = tu_evtimer_new(main_base_event_loop);
	tu_set_evtimer(LevelTime->doorLevelTimer,DEVICE_LIWEI_DOOR_QUERY_TIME,false,doorLevel_timerGetInfoHandle,LevelTime);
}

static void doorLevel_timerDoorStateIndHandle(void *args)
{
	ASSERT(args != NULL);
	log_debug("doorLevel_timerDoorStateIndHandle++\n");

	Level_time_Types_t *Leveltimer = (Level_time_Types_t*)args;
	Level_Stu_t *stu =  &Leveltimer->stu;

	stu->doorlockstu = DOOR_CLOSE;

	SRPC_DoorLockCtrlIndCB(&Leveltimer->epInfo,SUCCESS,stu->doorlockstu,stu->openType,stu->userid,stu->timevail,stu->time);
	//doorLevel_doorStateInd(&Leveltimer->epInfo,DOOR_CLOSE);
	
	tu_evtimer_free(Leveltimer->doorLevelTimer);
	
	free(Leveltimer);
	
	log_debug("doorLevel_timerDoorStateIndHandle++\n");
}

//定时去上报门锁开关状态
void doorLevel_timerDoorStateInd(epInfo_t *epInfo)
{
	ASSERT(epInfo != NULL);
	
	Level_time_Types_t *LevelTime = malloc(sizeof(Level_time_Types_t));
	
	if (LevelTime == NULL)
	{
		log_err("malloc failed\n");
		return;
	}
	
	memcpy(&(LevelTime->epInfo),epInfo,sizeof(epInfo_t));
	memcpy(&LevelTime->stu,&level_stu,sizeof(Level_Stu_t));
	
	LevelTime->doorLevelTimer = tu_evtimer_new(main_base_event_loop);
	tu_set_evtimer(LevelTime->doorLevelTimer,DEVICE_LIWEI_DOOR_QUERY_TIME,false,doorLevel_timerDoorStateIndHandle,LevelTime);
}

/*****************************************************************************
 * 函 数 名  : doorLevel_precessDoorState
 * 负 责 人  : Edward
 * 创建日期  : 2016年11月29日
 * 函数功能  : 处理门锁当前状态逻辑
 * 输入参数  : uint8_t cardType  门锁开门类型
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static uint8_t doorLevel_precessDoorState(epInfo_t *epInfo,uint8_t doorOperateType,uint8_t doorOpenType)
{

	log_debug("doorLevel_precessDoorState++\n");
	uint8_t doorstate = DOOR_CLOSE;
	

	if(((doorOperateType >=RECORD_TYPE_0_CARD) && (doorOperateType <=RECORD_TYPE_18_CARD)) ||
		(doorOperateType == RECORD_TYPE_18_CARD)||(doorOperateType == RECORD_TYPE_29_CARD))
	{
		level_stu.openType = DOORLOCK_TYPE_CARD;
		doorStateFromServer = DOOR_OPEN;
	}
	else if(doorOperateType == RECORD_TYPE_21_PWD)
	{
		level_stu.openType = DOORLOCK_TYPE_PWD;
		doorStateFromServer = DOOR_OPEN;
	}
	else if(doorOperateType == RECORD_TYPE_22_FINGER)
	{
		level_stu.openType = DOORLOCK_TYPE_FINGER;
		doorStateFromServer = DOOR_OPEN;
	}
	else if(doorOperateType == RECORD_TYPE_24_REMOTE || doorOperateType == RECORD_TYPE_28_CARD)
	{
		level_stu.openType = DOORLOCK_TYPE_REMOTE;
	}
	else if(doorOperateType == RECORD_TYPE_31_KEY)
	{
		level_stu.openType = DOORLOCK_TYPE_KEY;
		doorStateFromServer = DOOR_OPEN;
	}
	else
	{
		level_stu.openType = DOORLOCK_TYPE_OTHER;
		doorStateFromServer = DOOR_OPEN;
	}
	
	if(doorOperateType == RECORD_TYPE_28_CARD)//记录卡
	{
		level_stu.doorlockstu = DOOR_CLOSE;
		//doorstate = DOOR_CLOSE;//门关闭状态
	}
	else
	{
		//获取门锁状态
		switch(doorStateFromServer)
		{
			case DOOR_CLOSE://关门
			{
				level_stu.doorlockstu = DOOR_CLOSE;
			}	
			break;
			case DOOR_ALWAYS_OPEN://常开
			{
				level_stu.doorlockstu = DOOR_ALWAYS_OPEN;//常开
			}
			break;
			case DOOR_OPEN://普通开门
			{
				level_stu.doorlockstu = DOOR_OPEN;//开启
				//自动去上报门锁状态
				doorLevel_timerDoorStateInd(epInfo);
			}
			break;
			default:
			{
				level_stu.doorlockstu = DOOR_CLOSE;//开启
				//启动定时获取门锁状态
				doorLevel_timerGetInfo(epInfo);
			}
			break;
		}
	}

	log_debug("doorstate = %d\n",level_stu.doorlockstu);
	log_debug("doorLevel_precessDoorState++\n");
	return level_stu.doorlockstu;
}

/*****************************************************************************
 * 函 数 名  : doorLevel_realRecordReport
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 门锁开门日志上报-0x1C
 * 输入参数  : epInfo_t *epInfo  节点信息
               hostCmd *cmd      串口透传数据
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_realRecordReport(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t area = 0; 		//区域
	uint8_t unit = 0;		//楼栋单元
	uint8_t floor = 0 ;		//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;		//套房
	uint8_t cardType = 0 ; 	//卡片类型
	uint8_t uid[4] = {0};	//卡片信息
	uint8_t year = 0;		//开门时间
	uint8_t month = 0;		//
	uint8_t dates = 0;		//
	uint8_t hours = 0;
	uint8_t minute = 0;
	uint8_t power = 0;		//电压状态

	uint16_t dBattery = 0;
	uint16_t wBattery = 0;
	
	uint8_t doorstate = 0;
	uint8_t doorOperateType = 0; //操作类型
	uint8_t times[6] = {0};
	uint8_t doorPowerAlarm = 0;
	uint8_t wiressPowerAlarm = 0;

	ASSERT((epInfo != NULL) && (cmd != NULL));

	/*楼层信息*/
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//有用信息
	cmdGet8bitVal(cmd, &cardType);
	
	log_debug("记录类型: %s\n",RecodeType[cardType & 0x1F]);
	log_debug("读卡时门锁处于%s状态\n",(cardType & 0x20)==0?"常闭":"常开");
	log_debug("%s\n",(cardType & 0x40)==0?"常规开门":"常开");
	log_debug("%s\n",(cardType & 0x80)==0?"没反锁开门":"反锁开门");
	
	//卡号
	cmdGet8bitVal(cmd, &uid[0]);
	cmdGet8bitVal(cmd, &uid[1]);
	cmdGet8bitVal(cmd, &uid[2]);
	cmdGet8bitVal(cmd, &uid[3]);

	log_debug("UID0:UID1:UID2:UID3 = %d : %d : %d : %d \n",uid[0],uid[1],uid[2],uid[3]);
	
	//开门时间
	cmdGet8bitVal(cmd, &year);
	cmdGet8bitVal(cmd, &month);
	cmdGet8bitVal(cmd, &dates);
	cmdGet8bitVal(cmd, &hours);
	cmdGet8bitVal(cmd, &minute);

	//无线模块电压状态
	cmdGet8bitVal(cmd, &power);

	//电池电压值
	cmdGet16bitVal(cmd,&dBattery);
	cmdGet16bitVal(cmd,&wBattery);
	
	level_stu.userid = uid[3];

	//门锁开门时间判断
	if(year & 0x80)//非法时间
	{
		log_debug("无效时间\n");
		times[0] = 0;//无效
		level_stu.timevail = 0x00;
	}
	else
	{
		log_debug("有效时间\n");
		level_stu.timevail = 0x01;
		level_stu.time[0] = year & 0x7F;
		level_stu.time[1] = month & 0x0F;
		level_stu.time[2] = dates & 0x1F;
		level_stu.time[3] = hours & 0x1F;
		level_stu.time[4] = minute & 0x3F;
		level_stu.time[5] = 0;
	}

	//门锁电压判断
	if(month & 0x80)
	{
		log_debug("门锁电压低\n");
		level_stu.batAlarmFalg = DOORLOCK_POWER_ST_LOW;
	}
	else
	{
		log_debug("门锁电压正常\n");
		level_stu.batAlarmFalg = DOORLOCK_POWER_ST_NORMAL;
	}

	//门锁电池电量

	if(dBattery != 0X00)
		level_stu.dBattery = dBattery;
	else
		level_stu.dBattery = 0x00;
	
	log_debug("门锁电压:%x\n",dBattery);
	log_debug("门锁电压:%d mV\n",level_stu.dBattery);

	doorOperateType = (cardType & 0x1F);

	//处理门锁状态逻辑
	doorstate = doorLevel_precessDoorState(epInfo,doorOperateType,cardType & 0x40);

	//开门上报
	SRPC_DoorLockCtrlIndCB(epInfo,SUCCESS,level_stu.doorlockstu,level_stu.openType,level_stu.userid,level_stu.timevail,level_stu.time);

	//上报电池电量低报警
	if(level_stu.batAlarmFalg)
	{
		//SRPC_DoorLockAlarmIndCB(epInfo,level_stu.batAlarmFalg,level_stu.time,level_stu.timevail);
		SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_LOW_BATTERY,level_stu.timevail,level_stu.time);
	}
	
	//上报电量消息
	if(level_stu.dBattery)
	{
		SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,level_stu.batAlarmFalg,DOORLOCK_POWER_TYPE_VALUE,level_stu.dBattery);
		vdevListSetDoorBattery(epInfo, DOORLOCK_POWER_TYPE_VALUE, level_stu.dBattery);
	}
	else
	{
		SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,level_stu.batAlarmFalg,DOORLOCK_POWER_TYPE_STATUS,level_stu.batAlarmFalg);
	}

	vdevListSetDoorBatteryStu(epInfo, DOORLOCK_POWER_TYPE_STATUS, level_stu.batAlarmFalg);
	vdevListSetDoorState(epInfo,level_stu.doorlockstu);
	vdevListSetDoorAlarm(epInfo, level_stu.batAlarmFalg);

#if 0
	zbDevDoorLockTime.endpoint = 0x08;
	zbDevDoorLockTime.nwkaddr = mEpInfo.nwkAddr;

	zbDevDoorLockTime.zbDevDoorLockTimer = tu_evtimer_new(main_base_event_loop);
	tu_set_evtimer(zbDevDoorLockTime.zbDevDoorLockTimer,10000,false,zbDevDoorLock_getstatus_timerhandler,&zbDevDoorLockTime);
#endif

}

/*****************************************************************************
 * 函 数 名  : doorLevel_doorInfoReport
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 当前门锁的信息--0x08
 * 输入参数  : epInfo_t *epInfo  节点信息
               hostCmd *cmd      透传串口数据
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_doorInfoReport(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t area = 0; 	//区域
	uint8_t unit = 0;		//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房
	
	uint8_t year = 0;		//
	uint8_t month = 0;	//
	uint8_t dates = 0;	//
	uint8_t hours = 0;
	uint8_t minute = 0;
	uint8_t seconds = 0;
	uint8_t week = 0;
	uint16_t dverson = 0;
	uint16_t sversion = 0;
	uint8_t times[7] = {0};

	uint8_t doorState = 0;
	uint8_t doorPowerAlarm = 0;
	uint8_t wiressPowerAlarm = 0;
	uint8_t dooropentype = 0;
	uint8_t doorbell = 0;
	uint8_t doubleLock = 0;
	
	ASSERT((epInfo != NULL) && (cmd != NULL));
	
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//获取门锁时间
	cmdGet8bitVal(cmd, &year);
	cmdGet8bitVal(cmd, &month);
	cmdGet8bitVal(cmd, &dates);
	cmdGet8bitVal(cmd, &hours);
	cmdGet8bitVal(cmd, &minute);
	cmdGet8bitVal(cmd, &seconds);
	cmdGet8bitVal(cmd, &week);

	times[0] = year & 0x7f;
	times[1] = month & 0x0f;
	times[2] = dates & 0x1f;
	times[3] = hours & 0x1f;
	times[4] = minute & 0x3f;
	times[5] = seconds & 0x3f;
	times[6] = week & 0x07;

	//软件硬件版本
	cmdGet16bitVal(cmd,&dverson);
	cmdGet16bitVal(cmd,&sversion);

	log_debug("硬件版本信息:%d  软件版本信息:%d\n",dverson,sversion);
	
	cmdGet8bitVal(cmd, &doorState);

	//只有大于这个版本才支持电量及反锁信号检测
	
	cmdGet16bitVal(cmd,&level_stu.dBattery);
	
	if(level_stu.dBattery != 0x00)
	{
		cmdGet8bitVal(cmd,&doubleLock);
		level_stu.doublelock = (doubleLock & 0x80)==0?DOORLOCK_DLSTU_UNLOCK:DOORLOCK_DLSTU_LOCKED;
		vdevListSetDoorDLockStu(epInfo, level_stu.doublelock);
		vdevListSetDoorBattery(epInfo, DOORLOCK_POWER_TYPE_VALUE, level_stu.dBattery);
	}

	if(doorState & 0x80)
	{
		level_stu.batAlarmFalg = DOORLOCK_POWER_ST_LOW;//报警状态
	}
	else
	{
		level_stu.batAlarmFalg = DOORLOCK_POWER_ST_NORMAL;
	}

	//允许开锁方式
	dooropentype = doorState&0x0f; 

 	//门铃状态
	if(doorState & 0x10)
		doorbell = 0x01;
	else
		doorbell = 0x00;

	//门锁状态
	if(doorState & 0x20)
		level_stu.doorlockstu = DOOR_OPEN;
	else
		level_stu.doorlockstu = DOOR_CLOSE;

	level_stu.dBattery = vdevListGetDoorBattery(epInfo);

	//获取反锁信息
	level_stu.doublelock = vdevListGetDoorDLockStu(epInfo);

	if(level_stu.dBattery == 0XFFFF)
		SRPC_DoorLockPowerIndCB(epInfo, SUCCESS,level_stu.batAlarmFalg,DOORLOCK_POWER_TYPE_STATUS, level_stu.batAlarmFalg);
	else
		SRPC_DoorLockPowerIndCB(epInfo, SUCCESS,level_stu.batAlarmFalg,DOORLOCK_POWER_TYPE_VALUE, level_stu.dBattery);

	SRPC_DoorLockDeviceInfoIndCB(epInfo,SUCCESS,level_stu.doorlockstu ,level_stu.batAlarmFalg,level_stu.doublelock,epInfo->onlineDevRssi);

	if(level_stu.batAlarmFalg)
		SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_LOW_BATTERY,level_stu.timevail,level_stu.time);
		//SRPC_DoorLockAlarmIndCB(epInfo,level_stu.batAlarmFalg,level_stu.time,level_stu.timevail);

	//开门方式
	//SRPC_DoorLocDoorOpenPatternIndCB(epInfo,SUCCESS,dooropentype,dooropentype);
	
	vdevListSetDoorState(epInfo,level_stu.doorlockstu);
	vdevListSetDoorBatteryStu(epInfo,DOORLOCK_POWER_TYPE_STATUS, level_stu.batAlarmFalg);
	
	vdevListSetDoorDLockStu(epInfo, level_stu.doublelock);
	vdevListSetDoorAlarm(epInfo, level_stu.batAlarmFalg);
	
}

/*****************************************************************************
 * 函 数 名  : doorLevel_alarmOperationReport
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 非法操作数据上报-0X0B
 * 输入参数  : epInfo_t *epInfo  节点信息
               hostCmd *cmd      串口透传数据
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void doorLevel_alarmOperationReport(epInfo_t *epInfo,hostCmd *cmd)
{

	ASSERT((epInfo != NULL) && (cmd != NULL));
	uint8_t area = 0; 	//区域
	uint8_t unit = 0;		//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房

	uint8_t alarmOptType = 0;
	uint8_t doubleLockCtl = 0;
	uint8_t doubleLockStu = 0;
	uint8_t year = 0;		
	uint8_t month = 0;	
	uint8_t dates = 0;	
	uint8_t hours = 0;
	uint8_t minute = 0;
	uint8_t seconds = 0;
	uint8_t week = 0;
	uint8_t times[7] = {0};

	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//操作类型
	cmdGet8bitVal(cmd, &alarmOptType);

	//反锁状态
	doubleLockCtl	= 	alarmOptType & 0x10;
	doubleLockStu	=	(alarmOptType & 0x08)==0x00?DOORLOCK_DLSTU_UNLOCK:DOORLOCK_DLSTU_LOCKED;

	log_debug("反锁信息:%x %x\n",doubleLockCtl,doubleLockStu);

	//报警类型
	alarmOptType = alarmOptType&0x07;

	//获取门锁时间
	cmdGet8bitVal(cmd, &year);
	cmdGet8bitVal(cmd, &month);
	cmdGet8bitVal(cmd, &dates);
	cmdGet8bitVal(cmd, &hours);
	cmdGet8bitVal(cmd, &minute);
	cmdGet8bitVal(cmd, &seconds);
	cmdGet8bitVal(cmd, &week);
	
	times[0] = year & 0x7f;
	times[1] = month & 0x0f;
	times[2] = dates & 0x1f;
	times[3] = hours & 0x1f;
	times[4] = minute & 0x3f;
	times[5] = seconds & 0x3f;
	times[6] = week & 0x07;

	if(doubleLockCtl)
	{
		//上报反锁状态
		vdevListSetDoorDLockStu(epInfo, doubleLockStu);
		doorLevel_getInfoReq(epInfo);

		doubleLockStu = doubleLockStu==DOORLOCK_DLSTU_LOCKED?0x02:0x03;
		//反锁状态
		zbSoc_ProcessEvent(epInfo,doubleLockStu);
	}
	else
	{
		//暴力开锁
		if(alarmOptType == 0x07)
			alarmOptType = DOORLOCK_ALARM_VILENCE;
  		SRPC_DoorLockAlarmIndCB(epInfo, alarmOptType, DOORLOCK_TIME_VAIL, times);
	}
}

static void doorLevel_getRegisterInfoReport(epInfo_t *epInfo,hostCmd *cmd)
{
	ASSERT((epInfo != NULL) && (cmd != NULL));

	uint8_t area = 0; 	//区域
	uint8_t unit = 0;		//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房

	
	uint8_t uidsize = 0;
	uint8_t regType = 0;
	uint8_t *uid = NULL;
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//操作类型
	cmdGet8bitVal(cmd, &regType);

	if(cmd->size >= (cmd->idx - 2))
	{
		uidsize = (cmd->size - cmd->idx - 2);
	}

	uid = &(cmd->data[cmd->idx]);
	
	doorLevel_registerInfoInd(epInfo,regType,uid,uidsize);
	
}

void doorLevel_allSetCmdReport(epInfo_t *epInfo,hostCmd *cmd)
{
	ASSERT((epInfo != NULL) && (cmd != NULL));

	log_debug("doorLevel_allSetCmdReport++\n");
	uint8_t area = 0; 	//区域
	uint8_t unit = 0; 	//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房


	uint8_t cmdid = 0;
	uint8_t result = 0;
	
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	cmdGet8bitVal(cmd, &cmdid);
	cmdGet8bitVal(cmd, &result);

	log_debug("cmdid = %x ,result = %d\n",cmdid,result);

	switch(cmdid)
	{
		case DOOR_CMD_USR_MNG:
		{
			//result = result==0?0:1;
			//doorLevel_UsrMngStateInd(epInfo,result);		
			if(Level_Operate == LEVEL_USR_TYPE_PWD_ID)
				SRPC_DoorLockPINCodeIndCB(epInfo,result);
			else if(Level_Operate == LEVEL_USR_TYPE_CARD_ID)
				SRPC_DoorLockRFIDCodeIndCB(epInfo,result);
			else if(Level_Operate == LEVEL_USR_TYPE_FINGER_ID)
				SRPC_DoorLockFingerCodeIndCB(epInfo,result);
		}
		break;
		default:break;
	}
	
	log_debug("doorLevel_allSetCmdReport--\n");
}

void doorLevel_setOnOffReq(epInfo_t *epInfo,uint8_t optype)
{
	log_debug("doorLevel_setOnOff++\n");
	ASSERT(epInfo != NULL);

	hostCmd cmd;

	memset(&cmd,0,sizeof(cmd));
	
	doorStateFromServer = optype;

	//组包
	doorLevel_setOnOffCmd(&cmd,optype);

	//发送数据
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	
	log_debug("doorLevel_setOnOff--\n");
}

void doorLevel_getInfoReq(epInfo_t *epInfo)
{
	log_debug("doorLevel_getInfo++\n");
	ASSERT(epInfo != NULL);

	hostCmd cmd;

	memset(&cmd,0,sizeof(cmd));
	doorLevel_getInfoCmd(&cmd);
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	
	log_debug("doorLevel_getInfo--\n");
}

void doorLevel_settimeReq(epInfo_t *epInfo,uint8_t years,uint8_t month,uint8_t dates,uint8_t hours,uint8_t minute,uint8_t seconds,uint8_t week)
{
//	uint8_t year = 0;		//
//	uint8_t month = 0;	//
//	uint8_t dates = 0;	//
//	uint8_t hours = 0;
//	uint8_t minute = 0;
//	uint8_t seconds = 0;
//	uint8_t week = 0;

	log_debug("doorLevel_settime++\n");

	ASSERT(epInfo != NULL);
	
	hostCmd cmd;
	memset(&cmd,0,sizeof(cmd));
	doorLevel_settimeCmd(&cmd, years, month, dates, hours, minute, seconds, week);
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	
	log_debug("doorLevel_settime--\n");
}


/*****************************************************************************
 * 函 数 名  : doorLevel_getRegisterInfo
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 获取门锁登记信息-0X10
 * 输入参数  : epInfo_t *epInfo    节点信息
               uint8_t registerType  登记列表类型
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void doorLevel_getRegisterInfoReq(epInfo_t *epInfo,uint8_t registerType)
{

	log_debug("doorLevel_settime++\n");

	ASSERT(epInfo != NULL);
	
	hostCmd cmd;
	memset(&cmd,0,sizeof(cmd));
	doorLevel_getRegisterInfoCmd(&cmd, registerType);
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	log_debug("doorLevel_settime--\n");
}

int doorLevel_usrMngReq(epInfo_t *epInfo,uint8_t usrType,uint8_t optType,uint32_t usrid,uint8_t *passwd)
{
	int ret = SUCCESS;
	log_debug("doorLevel_settime++\n");

	ASSERT(epInfo != NULL);
	
	hostCmd cmd;
	memset(&cmd,0,sizeof(cmd));
	ret = doorLevel_SetUsrMngCmd(&cmd,  usrType, optType, usrid,passwd);
	if(!ret)
	{
		doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	}
	log_debug("doorLevel_settime--\n");
	return ret;
}

#if DEVICE_LIWEI_DOOR_OPEN_CNT
void doorLevel_getOpenCntReq(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID

    cmdSet16bitVal_lh(&cmd, 0x0038);//Attr ID
   
    zbMakeMsgEnder(&cmd);
    
	usleep(1000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}

#endif


int doorLevel_SerialNetInd(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t dataType;
	uint8_t sof;
	uint8_t datalen;
	uint8_t pkglen;
	uint8_t cmdtype;

	if(epInfo == NULL)
		return FAILED;

	cmdGet8bitVal(cmd, &dataType);

	//接收到返回，关闭重发机制
  	zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

	if(dataType != ZCL_DATATYPE_OCTET_STR)
		return FAILED;

	cmdGet8bitVal(cmd, &datalen);	//数据总长度
	cmdGet8bitVal(cmd, &sof);		//起始标志
	cmdGet8bitVal(cmd, &pkglen);	//命令长度
	cmdGet8bitVal(cmd, &cmdtype);	//命令类型
	
	switch(cmdtype)
	{
		case DOORLOCK_LEVEL_CMD_DEVICE_OPEN_RECORD://实时开门记录
		{
			log_debug("实时开门记录上报\n");
			doorLevel_realRecordReport(epInfo,cmd);
    	}
		break;
		case DOORLOCK_LEVEL_CMD_DEVICE_INFO://门锁状态
		{
			doorLevel_doorInfoReport(epInfo,cmd);
		}
		break;
		case DOORLOCK_LEVEL_CMD_DEVICE_ALARM://报警
		{
			doorLevel_alarmOperationReport(epInfo,cmd);
		}
		break;
		/*
		case DOORLOCK_LEVEL_CMD_USR_REG_INFO://获取等级信息
		{
			doorLevel_getRegisterInfoReport(epInfo,cmd);
		}
		break;
		*/
		case DOORLOCK_LEVEL_CMD_STATUS_INFO://命令状态返回
		{
			doorLevel_allSetCmdReport(epInfo,cmd);
		}
		break;
		default:break;
	}
	
	return SUCCESS;
}

void doorLevel_ShortPollRateInd(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t dataType;
	uint16_t shortPollRate = 0;
	cmdGet8bitVal(cmd, &dataType);

	if(dataType == ZCL_DATATYPE_UINT16)
	{
		cmdGet16bitVal_lh(cmd, &shortPollRate);
		shortPollRate = shortPollRate * 250;
		SRPC_DoorLockShortPollRateIndCB(epInfo,SUCCESS,shortPollRate);
	}
	else
	{
		SRPC_DoorLockShortPollRateIndCB(epInfo,FAILED,0x00);
	}
}

//力维门锁，串口接收处理函数
int doorLevel_ReviceMsgProcess_Event(epInfo_t *epInfo,hostCmd *cmd)
{
	uint16_t attrID;
	uint8_t dataType;
	uint8_t sof;
	uint8_t datalen;
	uint8_t pkglen;
	uint8_t cmdtype;

	ASSERT(epInfo != NULL);

	log_debug("doorLevel_ReviceMsgProcess_Event++\n");

	cmdGet16bitVal_lh(cmd, &attrID);
	switch(attrID)
	{
	//数据透传
	case ATTRID_BASIC_LOCK_UNLOCK:
		doorLevel_SerialNetInd(epInfo,cmd);
		break;
	
	//设置datarequest间隔上报
	case ATTRID_POLL_CONTROL_SHORT_POLL_INTERVAL:
		doorLevel_ShortPollRateInd(epInfo,cmd);
		break;
	
	default:
	break;
	}

	log_debug("doorLevel_ReviceMsgProcess_Event--\n");
	return SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//							对外接口
//////////////////////////////////////////////////////////////////////////////

uint8_t doorLevel_OpenCB(epInfo_t *epInfo,uint8_t status)
{
		doorLevel_setOnOffReq(epInfo,status);
		return SUCCESS;
}

uint8_t doorLevel_PowerValueCB(epInfo_t *epInfo)
{
	log_debug("doorLevel_PowerValueCB++\n");	
	uint8_t batteryType = 0;
	uint16_t battery = 0;
	uint8_t batteryStu = 0;
	if(epInfo != NULL)
	{
		//batteryType = vdevListGetDoorBatteryType(epInfo);
		//if(batteryType == 0xFF)
		//{
			doorLevel_getInfoReq(epInfo);
		//}
		//else
		//{
		//	battery = vdevListGetDoorBattery(epInfo);
		//	batteryStu = vdevListGetDoorBatteryStu(epInfo);
		//	log_debug("电池容量:0x%x 支持状态:0x%x 报警状态:0x%x\n",battery,batteryType,batteryStu)
		//	if(batteryType & DOORLOCK_POWER_TYPE_VALUE_BIT)
		//	{
		//		SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,batteryStu,DOORLOCK_POWER_TYPE_VALUE,battery);
		//	}
		//	else if(batteryType & DOORLOCK_POWER_TYPE_PERCENT_BIT)
		//	{
		//		SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,batteryStu,DOORLOCK_POWER_TYPE_PERCENT,battery);
		//	}
		//	else if(batteryType & DOORLOCK_POWER_TYPE_STATUS_BIT)
		//	{
		//		SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,batteryStu,DOORLOCK_POWER_TYPE_STATUS,batteryStu);
		//	}

			//低电量报警
		//	if(batteryStu == DOORLOCK_POWER_ST_LOW)
		//		SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_LOW_BATTERY,0x00,NULL);
			
	//	}
		return SUCCESS;
	}

	log_debug("doorLevel_PowerValueCB--\n");	

	return FAILED;
}

uint8_t doorLevel_GetDeviceInfo(epInfo_t *epInfo)
{
	 doorLevel_getInfoReq(epInfo);
	 return SUCCESS;
}

uint8_t doorLevel_SetPINCodeCB(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen)
{
	uint8_t usrType = LEVEL_USR_TYPE_PWD_ID;//用户类型
	uint8_t optType = 0;//操作类型

	if(epInfo == NULL)
		return FAILED;

	if(pwdOpert == DOORLOCK_USR_ADD)
		optType = LEVEL_OPT_TYPE_ONLAY_ADD;
	else if(pwdOpert == DOORLOCK_USR_DEL)
		optType = LEVEL_OPT_TYPE_ONLAY_DEL;
	else if(pwdOpert == DOORLOCK_USR_CLEAR)
		optType = LEVEL_OPT_TYPE_ALL_DEL;
	else
		return FAILED;

	if(pwdOpert == DOORLOCK_USR_ADD && pwdlen != 8)
			return FAILED;

	/*力维密码只能存储2位*/
	if(pwdOpert == DOORLOCK_USR_ADD && (userid!=0x01 && userid != 0x02))
		return YY_STATUS_DOWNLOAD_PROGRESS;

	Level_Operate = LEVEL_USR_TYPE_PWD_ID ;
	
  return doorLevel_usrMngReq(epInfo, usrType, optType, userid, pwd);
}

uint8_t doorLevel_SetRFIDCodeCB(epInfo_t *epInfo,uint8_t optType,uint16_t userid,uint8_t idlen,uint8_t *id)
{
	uint8_t usrType = LEVEL_USR_TYPE_CARD_ID;//用户类型
	uint8_t usroptType = 0;//操作类型

	if(epInfo == NULL)
		return FAILED;

	if(optType == DOORLOCK_USR_ADD)
			return YY_STATUS_DOWNLOAD_PROGRESS;
	else if(optType == DOORLOCK_USR_DEL)
		usroptType = LEVEL_OPT_TYPE_ONLAY_DEL;
	else if(optType == DOORLOCK_USR_CLEAR)
		usroptType = LEVEL_OPT_TYPE_ALL_DEL;
	else
		return FAILED;
	Level_Operate = LEVEL_USR_TYPE_CARD_ID ;
  return doorLevel_usrMngReq(epInfo, usrType, usroptType, userid, NULL);
}

uint8_t doorLevel_SetFINGERCodeCB(epInfo_t *epInfo,uint8_t optType,uint16_t userid)
{
	uint8_t usrType = LEVEL_USR_TYPE_FINGER_ID;//用户类型
	uint8_t usroptType = 0;//操作类型

	if(epInfo == NULL)
		return FAILED;

	if(optType == DOORLOCK_USR_ADD)
			return YY_STATUS_DOWNLOAD_PROGRESS;
	else if(optType == DOORLOCK_USR_DEL)
		usroptType = LEVEL_OPT_TYPE_ONLAY_DEL;
	else if(optType == DOORLOCK_USR_CLEAR)
		usroptType = LEVEL_OPT_TYPE_ALL_DEL;
	else
		return FAILED;
	
	Level_Operate = LEVEL_USR_TYPE_FINGER_ID;
  return doorLevel_usrMngReq(epInfo, usrType, usroptType, userid, NULL);
}

uint8_t doorLevel_SetPollRateCB(epInfo_t *epInfo,uint16_t times)
{
	doorlock_SetShortPollRate(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,times);
}

