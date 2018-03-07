/***********************************************************************************
 * 文 件 名   : doorLock_Yaotai.c
 * 负 责 人   : Edward
 * 创建日期   : 2017年2月21日
 * 文件描述   : 遥泰门锁功能接口
 * 版权说明   : Copyright (c) 2008-2017   无锡飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
	
/*********************************************************************
* INCLUDES
*/
#include "doorlock_Level.h"
#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"
	
#include "zbSocPrivate.h"
#include "interface_srpcserver_defs.h"

#include "GwComDef.h"
#include "doorlock.h"
#include "doorlock_zbSocCmds.h"
/*********************************************************************
* MACROS
*/

typedef enum
{
	YT_DOORLOCK_OPEN  			= 0x80,
	YT_DOORLOCK_ADD_PWD  		= 0x81,
	YT_DOORLOCK_DEL_PWD			= 0x82,		
	YT_DOORLOCK_UNUSE_PWD		= 0x83,
	YT_DOORLOCK_USE_PWD			= 0x84,
	YT_DOORLOCK_ALARM				= 0x85,	
}YT_DOORLOCK_CMD_TYPE;

typedef enum
{
	DOOR_CLOSE 		 = 0X00,
	DOOR_OPEN  		 = 0X01,
}YT_DOORLOCK_STATE_TYPE;

typedef struct
{
	uint8_t result;
	uint8_t doorLockSt;
	uint8_t openType;
	uint16_t userid;
	uint8_t timevail;
	uint8_t times[6];
}DoorLock_Open_Ind_Types_t;

typedef struct
{
	tu_evtimer_t *doorLevelTimer;
	epInfo_t epInfo;
	DoorLock_Open_Ind_Types_t doorstu;
}DoorLock_time_Types_t;

/*********************************************************************
* CONSTANTS
*/

/*********************************************************************
* TYPEDEFS
*/

#define DOORLOCK_YT_REQ_SOF				0x5A
#define DOORLOCK_YT_ADD_PWD_CMD		0x81
#define DOORLOCK_YT_DEL_PWD_CMD		0x82

#define DOORLOCK_YT_USER_ID_BASE	0x1000
/*********************************************************************
* LOCAL FUNCTIONS
*/
uint8_t zbSoc_DoorLockYT_OpenCB(epInfo_t *epInfo,uint8_t status);
uint8_t zbSoc_DoorLockYT_PowerValueCB(epInfo_t *epInfo);
uint8_t zbSoc_DoorLockYT_SetPINCodeCB(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen);
void zbSoc_DoorLockYT_TimerDoorLockInd(epInfo_t *epInfo,DoorLock_Open_Ind_Types_t *args);
uint8_t zbSoc_DoorLockYT_GetDeviceInfoCB(epInfo_t *epInfo);
uint8_t zbSoc_DoorLockYT_SetPollRateCB(epInfo_t *epInfo,uint16_t times);
void zbSoc_DoorLockYT_ShortPollRateInd(epInfo_t *epInfo,hostCmd *cmd);

/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* LOCAL VARIABLES
*/

zbSocDoorLockAppCallbacks_t zbSocDoorLockYTAppCallbacks = 
{
	zbSoc_DoorLockYT_OpenCB,
	zbSoc_DoorLockYT_PowerValueCB,
	zbSoc_DoorLockYT_GetDeviceInfoCB,
	zbSoc_DoorLockYT_SetPINCodeCB,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	zbSoc_DoorLockYT_SetPollRateCB
};
	
zbSocDoorLockCallbacks_t	zbSocDoorLock_YaoTaiCallBack_t = 
{
	 ZCL_HA_DEVICEID_FXZB_DOORLOCK_YAOTAI,
	 &zbSocDoorLockYTAppCallbacks 
};

/*********************************************************************
* GLOBAL VARIABLES
*/

/********************************************************************/

static uint8_t DoorLock_cmdSetFCS(hostCmd *cmd)
{
    uint8_t fcs=0;
    if((cmd->idx + 1) < MaxPacketLength)
    {
        uint16_t cnt;
        for(cnt=0; cnt<cmd->idx; cnt++)
        {
            fcs ^= cmd->data[cnt];
        }

        cmd->data[cmd->idx++] = fcs;

        return false;
    }
    else
    {
        return true;
    }
}

/*数据透传*/
static void doorLock_SendSerialCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t len,uint8_t* buf)
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

static void doorLockYT_OpenReport(epInfo_t *epInfo,hostCmd *cmd)
{
	
	uint8_t pkglen;
	uint8_t times[7] = {0};
	uint16_t RoomID = 0;
	uint8_t DoorState = 0;
	uint8_t PowerState = 0;
	uint8_t doorOperateType = 0;

	uint8_t usrid[4] = {0};

	log_debug("doorLockYT_OpenReport++\n");
	
	DoorLock_Open_Ind_Types_t openTypes;
	
	cmdGet8bitVal(cmd, &pkglen);	//命令长度
	cmdGet16bitVal(cmd,&RoomID);
	cmdGet8bitVal(cmd, &DoorState);
	cmdGet8bitVal(cmd, &PowerState);

	openTypes.doorLockSt = DOORLOCK_STATUS_OPEN;
	
	if(RoomID == 0x00)
	{
		openTypes.openType = DOORLOCK_TYPE_REMOTE;
		openTypes.userid = 0x00;
	}
	else
	{
		openTypes.openType = DOORLOCK_TYPE_PWD;
		openTypes.userid = RoomID - DOORLOCK_YT_USER_ID_BASE ;;
	}
	
	openTypes.result = SUCCESS;
	openTypes.timevail = DOORLOCK_TIME_UNVAIL;

	SRPC_DoorLockCtrlIndCB(epInfo,openTypes.result,openTypes.doorLockSt,openTypes.openType,openTypes.userid,openTypes.timevail,openTypes.times);
	SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,PowerState,DOORLOCK_POWER_TYPE_STATUS,PowerState);

	if(PowerState)
		SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_LOW_BATTERY,0x00,NULL);
	
	vdevListSetDoorAlarm(epInfo, PowerState);
	vdevListSetDoorState(epInfo,DOOR_CLOSE);
	vdevListSetDoorBattery(epInfo,DOORLOCK_POWER_TYPE_STATUS ,PowerState);
	
	zbSoc_DoorLockYT_TimerDoorLockInd(epInfo,&openTypes);
	log_debug("doorLockYT_OpenReport--\n");
}

//static int doorLockYT_SetUsrMngCmd(hostCmd *cmd,uint8_t usrType,uint8_t optType,uint16_t usrid,uint8_t *passwd)
static int doorLockYT_SetUsrMngCmd(hostCmd *cmd,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *passwd,uint8_t pwdlen)
{
	uint8_t cnt = 0;
	uint8_t mUsrtType = 0;
	uint8_t mData[4] = {0};
	
	cmdSet8bitVal(cmd, DOORLOCK_YT_REQ_SOF);

	if(pwdOpert < DOORLOCK_USR_ADD && pwdOpert > DOORLOCK_USR_CLEAR)
		return FAILED;

	switch(pwdOpert)
	{
		case DOORLOCK_USR_ADD:
		{
			cmdSet8bitVal(cmd, DOORLOCK_YT_ADD_PWD_CMD);
		
			if(pwdlen < 4 || pwdlen > 10)
			{
				return FAILED;
			}

			if(pwdlen % 2 != 0)
			{
				return FAILED;
			}

			cmdSet8bitVal(cmd, 2+pwdlen/2);

			cmdSet16bitVal(cmd,userid);

			uint8_t pwd  = 0;
			for(cnt =0;cnt<(pwdlen/2);cnt++)
			{
				 pwd = passwd[cnt*2]<<4 | passwd[cnt*2+1];
				 cmdSet8bitVal(cmd,pwd);
			}		
		}
		
		break;
		case DOORLOCK_USR_DEL:
		{
			cmdSet8bitVal(cmd, DOORLOCK_YT_DEL_PWD_CMD);
			cmdSet8bitVal(cmd, 0x02);
			cmdSet16bitVal(cmd,userid);
		}
		break;
		case DOORLOCK_USR_CLEAR:
			cmdSet8bitVal(cmd, DOORLOCK_YT_DEL_PWD_CMD);
			cmdSet8bitVal(cmd, 0x02);
			cmdSet16bitVal(cmd,0x0000);
		break;
		default:
			return FAILED;
		break;
	}

	DoorLock_cmdSetFCS(cmd);
	
	//保留
	cmdSet8bitVal(cmd, 0x0D);
	cmdSet8bitVal(cmd, 0x0A);

	return SUCCESS;
}


static void doorLockYT_AddUsrIndMsg(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t pkglen = 0;
	uint16_t RoomID = 0;
	uint8_t passwd[10] = {0};
	uint8_t repasswd1[10] = {0};
	uint8_t repasswd2[10] = {0xFF};

	log_debug("doorLockYT_AddUsrIndMsg++\n");

	cmdGet8bitVal(cmd, &pkglen);	//命令长度
	cmdGet16bitVal(cmd,&RoomID);
	cmdGetStringVal(cmd,passwd,pkglen-2);

	if(!memcmp(passwd,repasswd1,pkglen-2)||!memcmp(passwd,repasswd2,pkglen-2))
	{
		SRPC_DoorLockPINCodeIndCB(epInfo,FAILED);
	}
	else
	{
		SRPC_DoorLockPINCodeIndCB(epInfo,SUCCESS);
	}
		log_debug("doorLockYT_AddUsrIndMsg--\n");
}

static void doorLockYT_DelUsrIndMsg(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t pkglen = 0;
	uint16_t RoomID = 0;
	uint8_t passwd[10] = {0};

	cmdGet8bitVal(cmd, &pkglen);	//命令长度
	cmdGet16bitVal(cmd,&RoomID);
	cmdGetStringVal(cmd,passwd,pkglen-2);

	SRPC_DoorLockPINCodeIndCB(epInfo,SUCCESS);
}

static void doorLockYT_AlarmUsrIndMsg(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t pkglen = 0;
	uint16_t RoomID = 0;
	uint8_t times[7] = {0};
	uint8_t alarmOptType = 4;

	cmdGet8bitVal(cmd, &pkglen);	//命令长度

	SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_VILENCE,DOORLOCK_TIME_UNVAIL,times);
	//doorLevel_alarmOperationInd(epInfo,alarmOptType,times);
}

static void zbSoc_DoorLockYT_timerDoorStateIndHandle(void *args)
{
	ASSERT(args != NULL);
	log_debug("zbSoc_DoorLockYT_timerDoorStateIndHandle++\n");

	DoorLock_time_Types_t *times = (DoorLock_time_Types_t*)args;
	DoorLock_Open_Ind_Types_t *open =  (DoorLock_Open_Ind_Types_t*)&times->doorstu;

	if(open != NULL)
	{
		SRPC_DoorLockCtrlIndCB(&times->epInfo,SUCCESS,DOOR_CLOSE,open->openType,open->userid,open->timevail,open->times);
	}
	
	tu_evtimer_free(times->doorLevelTimer);
	
	free(times);
	
	log_debug("zbSoc_DoorLockYT_timerDoorStateIndHandle++\n");
}

void zbSoc_DoorLockYT_TimerDoorLockInd(epInfo_t *epInfo,DoorLock_Open_Ind_Types_t *doorstu)
{
	ASSERT(epInfo != NULL);
	
	DoorLock_time_Types_t *times = malloc(sizeof(DoorLock_time_Types_t));
	
	if (times == NULL)
	{
		log_err("malloc failed\n");
		return;
	}
	
	memcpy(&(times->epInfo),epInfo,sizeof(epInfo_t));
	memcpy(&times->doorstu,doorstu,sizeof(DoorLock_Open_Ind_Types_t));
	
	times->doorLevelTimer = tu_evtimer_new(main_base_event_loop);
	tu_set_evtimer(times->doorLevelTimer,DEVICE_LIWEI_DOOR_QUERY_TIME,false,zbSoc_DoorLockYT_timerDoorStateIndHandle,times);
}

void doorLockYT_setOnOffReq(epInfo_t *epInfo,uint8_t optype)
{
	log_debug("doorLockYT_setOnOffReq++\n");
	ASSERT(epInfo != NULL);

	uint8_t cmds[] = {0x5A,0x80,0x00,0xDA,0x0D,0x0A};
	
	//发送数据
	doorLock_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,6,cmds);

	log_debug("doorLockYT_setOnOffReq--\n");
}

/*
void doorLockYT_usrMngReq(epInfo_t *epInfo,uint8_t usrType,uint8_t optType,uint32_t usrid,uint8_t *passwd)
{
	log_debug("doorLockYT_usrMngReq++\n");

	ASSERT(epInfo != NULL);

	int ret = -1;
	
	hostCmd cmd;
	memset(&cmd,0,sizeof(cmd));

	ret = doorLockYT_SetUsrMngCmd(&cmd,  usrType, optType, usrid,passwd);

	if(!ret)
	{
		doorLock_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	}
	else
	{
		doorLevel_UsrMngStateInd(epInfo,01);
	}
	
	log_debug("doorLockYT_usrMngReq--\n");
}
*/

int doorLockYT_SerialNetIndComing(epInfo_t *epInfo,hostCmd *cmd)
{
	uint16_t attrID;
	uint8_t dataType;
	uint8_t sof;
	uint8_t datalen;
	uint8_t cmdtype;
	
	if(epInfo == NULL)
		return FAILED;
		
	log_debug("doorLockYT_SerialNetIndComing++\n");

	cmdGet8bitVal(cmd, &dataType);

	if(dataType != ZCL_DATATYPE_OCTET_STR)
		return FAILED;
	
	cmdGet8bitVal(cmd, &datalen);	//数据总长度
	cmdGet8bitVal(cmd, &sof);			//起始标志
	cmdGet8bitVal(cmd, &cmdtype);	//命令类型
	
	switch(cmdtype)
	{
		case YT_DOORLOCK_OPEN:
			doorLockYT_OpenReport(epInfo,cmd);
		break;
		case YT_DOORLOCK_ADD_PWD:
			doorLockYT_AddUsrIndMsg(epInfo,cmd);
		break;
		case YT_DOORLOCK_DEL_PWD:
			doorLockYT_DelUsrIndMsg(epInfo,cmd);
		break;	
		case YT_DOORLOCK_UNUSE_PWD:

		break;
		case YT_DOORLOCK_USE_PWD:

		break;
		case YT_DOORLOCK_ALARM:
			doorLockYT_AlarmUsrIndMsg(epInfo,cmd);
		break;
		default:break;
	}
	
	log_debug("doorLockYT_SerialNetIndComing--\n");
	return SUCCESS;
}

/*消息处理*/
int doorLockYT_MessageHandle_Event(epInfo_t *epInfo,hostCmd *cmd)
{
	uint16_t attrID;
	uint8_t dataType;
	uint8_t sof;
	uint8_t datalen;
	uint8_t cmdtype;
	
	ASSERT(epInfo != NULL);
		
	log_debug("doorLockYT_MessageHandle_Event++\n");

	 zblist_remove(epInfo->nwkAddr,epInfo->endpoint);
	 
	cmdGet16bitVal_lh(cmd, &attrID);

	log_debug("attrID:%d\n",attrID);

	switch(attrID)
	{
		case ATTRID_BASIC_UART_MSG:
			doorLockYT_SerialNetIndComing(epInfo,cmd);
			break;
		case ATTRID_POLL_CONTROL_SHORT_POLL_INTERVAL:
			zbSoc_DoorLockYT_ShortPollRateInd(epInfo,cmd);
			break;
		default:break;
	}
	
	log_debug("doorLockYT_MessageHandle_Event--\n");
	return SUCCESS;
}

void zbSoc_DoorLockYT_ShortPollRateInd(epInfo_t *epInfo,hostCmd *cmd)
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

uint8_t zbSoc_DoorLockYT_OpenCB(epInfo_t *epInfo,uint8_t status)
{
	log_debug("zbSoc_DoorLockYT_OpenCB++\n");	
	
	uint8_t cmds[] = {0x5A,0x80,0x00,0xDA,0x0D,0x0A};
	doorLock_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,sizeof(cmds),cmds);
	
	log_debug("zbSoc_DoorLockYT_OpenCB--\n");	
	return SUCCESS;
}

uint8_t zbSoc_DoorLockYT_GetDeviceInfoCB(epInfo_t *epInfo)
{
	uint8_t doorlockStu = 0;
	uint8_t batalarm  =0;
	if(epInfo != NULL)
	{
		doorlockStu = vdevListGetDoorState(epInfo);
		if(doorlockStu == 0xFF)
		{
			doorlockStu = DOORLOCK_STATUS_CLOSE;
			vdevListSetDoorState(epInfo, doorlockStu);
			vdevListSetDoorAlarm(epInfo, DOORLOCK_POWER_ST_NORMAL);
		}
	
		batalarm = vdevListGetDoorAlarm(epInfo);
		
		SRPC_DoorLockDeviceInfoIndCB(epInfo,SUCCESS,doorlockStu,batalarm,DOORLOCK_DLSTU_UNSUP,epInfo->onlineDevRssi);
	}
	
	return SUCCESS;
}

uint8_t zbSoc_DoorLockYT_PowerValueCB(epInfo_t* epInfo)
{
	log_debug("zbSoc_DoorLockYT_PowerValueCB++\n");	

	uint16_t value = 0;
	uint8_t batalarm = 0;
	if(epInfo != NULL)
	{
		value = vdevListGetDoorBattery(epInfo);
		if(value != 0xFFFF)
		{
			batalarm = vdevListGetDoorAlarm(epInfo);
			SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,batalarm,DOORLOCK_POWER_TYPE_STATUS,value);	
		}
		else
		{
			vdevListSetDoorBattery(epInfo, DOORLOCK_POWER_TYPE_STATUS, DOORLOCK_POWER_ST_NORMAL);
			SRPC_DoorLockPowerIndCB(epInfo,FAILED,0x00,DOORLOCK_POWER_TYPE_STATUS,DOORLOCK_POWER_ST_NORMAL);
		}
	}
	
	log_debug("zbSoc_DoorLockYT_PowerValueCB--\n");	

	return SUCCESS;
}


uint8_t zbSoc_DoorLockYT_SetPINCodeCB(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen)
{
	ASSERT(epInfo != NULL);

	int ret = 0;
	hostCmd cmd;
	uint16_t dstUserId = 0;
	
	memset(&cmd,0,sizeof(cmd));
	
	dstUserId = userid + DOORLOCK_YT_USER_ID_BASE ;

	ret = doorLockYT_SetUsrMngCmd(&cmd,  pwdType, pwdOpert, dstUserId,pwd,pwdlen);

	if(!ret)
	{
		doorLock_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	}
	else
	{
		SRPC_DoorLockPINCodeIndCB(epInfo,FAILED);
	}
	
	return ret;
}

uint8_t zbSoc_DoorLockYT_SetPollRateCB(epInfo_t *epInfo,uint16_t times)
{
	doorlock_SetShortPollRate(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,times);
}


/*********************************************************************/
