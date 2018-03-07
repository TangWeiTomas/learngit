/**************************************************************************
 * Filename:       doorlock_ygs.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 * Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2017-03-23,13:00)    :   Create the file.
 *************************************************************************/

/*********************************************************************
* INCLUDES
*/
#include <time.h>
#include "doorlock_ygs.h"
#include "doorlock.h"
#include "GwComDef.h"
#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"
#include "interface_srpcserver_defs.h"
#include "zbSocCmd.h"
#include "errorCode.h"
#include "zbSocPrivate.h"
#include "doorlock_zbSocCmds.h"
#include "GwComDef.h"

/*********************************************************************
* MACROS
*/
#define BUILD_UINT16(loByte, hiByte) \
						((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

//报警状态
#define YGS_ALARM_DOUBLE_LOCK_LOCKED		0X01
#define YGS_ALARM_DOUBLE_LOCK_UNLOCK		0x03
#define YGS_ALARM_LOW_POWR					0x04

//#define FALSE			0x01
//#define TRUE			0X00

#define YGS_PASSWD_TYPE_ADMIN			0X00
#define YGS_PASSWD_TYPE_NORMAL		0X01
#define YGS_PASSWD_TYPE_TIMES			0X02
#define YGS_PASSWD_TYPE_CYCLE			0X03

#define ATTRID_BASIC_SERIALNET_OVER_EVENT										0xFFFF		//透传功能

/*门锁透传指令*/
#define YGS_SERIALNET_CMD_IND				0X51
#define YGS_SERIALNET_CMD_REQ				0x50

#define PKG_MIN_SIZE 0x07

/*串口数据及协议处理*/
#define PKG_HEAD_SIZE 		5

#define PKG_HEAD_INDEX		0
#define PKG_BCC_INDEX			1
#define PKG_LEN_INDEX			2
#define PKG_CMD_INDEX			3
#define PKG_PAYLOAD_INDEX	4

/*转义字符处理*/
#define PKG_HEAD 					0X7E
#define PKG_TAIL					0X7F
#define PKG_ESCAPE	 			0X7D
#define PKG_ESCAPED_CODE 	0x20

/*********************************************************************
* CONSTANTS
*/
typedef enum
{
	DL_SN_CMD_PWD_CTR 				= 0X01,
	DL_SN_CMD_ALARM_IND 			= 0X02,
	DL_SN_CMD_MODE_CTR				= 0X03,
	DL_SN_CMD_GET_DINFO 			= 0X04,
	DL_SN_CMD_PWD_VAILD 			= 0X05,
	DL_SN_CMD_TIME_CTR				= 0X06,
	DL_SN_CMD_PWD_RECORD			= 0X07,
	DL_SN_CMD_PWD_GATHER			= 0X08,
	DL_SN_CMD_PWD_ALL_DEL 		= 0X09,
	DL_SN_CMD_MODEFI_ATTR 		= 0X0A,
	DL_SN_CMD_PWD_ERR_TIMES 	= 0X0B,
	DL_SN_CMD_PWD_ERR_IND 		= 0X0C,
	DL_SN_CMD_PWD_ERR_ST_IND	= 0X0D,
	DL_SN_CMD_CALC_PWD				= 0X0E,
	DL_SN_CMD_LOCAL_OPT_IND 	= 0X0F,
	DL_SN_CMD_PWD_ACTIVATE		= 0X10,
	DL_SN_CMD_TIME_ACTIVATE 	= 0X11,
}DL_SerilaNet_Cmd_t;

typedef enum
{
	DL_CMD_OPEN 							= 0X01,
	DL_CMD_OPEN_AWAYS 				= 0X02,
	DL_CMD_SET_SYS_TIME 			= 0x03,
	DL_CMD_STATE							= 0x06,
	DL_CMD_ALARM							= 0X07,
	DL_CMD_INFO 							= 0X08,
	DL_CMD_CAR								= 0X18,
	DL_CMD_DEL_CAR						= 0X19,
	DL_CMD_ZB_INIT						= 0X30,
	DL_CMD_ZB_EXIT_NWK				= 0X31,
	DL_CMD_ZB_ENTER_NWK 			= 0X32,
	DL_CMD_ZB_ST_IND					= 0x33,
	DL_CMD_GET_VERSION				= 0x40,
	DL_CMD_TIME_REQ 					= 0x41,
	DL_CMD_SET_PWD						= 0x42,
	DL_CMD_DEL_PWD						= 0x43,
	DL_CMD_CLR_PWD						= 0x44,
	DL_CMD_SERIAL_REQ 				= 0x50,
	DL_CMD_SERIAL_IND 				= 0x51,
	DL_CMD_READY							= 0xA0,
}DL_CMD_Types;
	
/*********************************************************************
* TYPEDEFS
*/

typedef struct 
{
	uint16_t userID;
	uint8_t  doorlockSt;  
  	uint8_t  operate;
	uint8_t  timeVail;
	uint8_t  upflags;
}zclDoorLockSt_t;

typedef struct
{
	uint8_t motorStu;
	uint8_t doubleLockStu;
	uint8_t blotStu;
	uint8_t cardIdCnt;
	uint16_t adVal;
	uint8_t batAlarmFalg;
	uint8_t activeStu;
}YGS_DoorLock_Status_t;

/*********************************************************************
* LOCAL FUNCTIONS
*/

uint8_t zbSoc_DoorLock_YgsOpenCB(epInfo_t *epInfo,uint8_t status);
uint8_t zbSoc_DoorLock_YgsSetPINCodeCB(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen);
uint8_t zbSoc_DoorLock_YgsPowerValueCB(epInfo_t *epInfo);
uint8_t zbSoc_DoorLock_YgsSetPollRateCB(epInfo_t *epInfo,uint16_t times);
uint8_t zbSoc_DoorLock_YgsDeviceInfoCB(epInfo_t *epInfo);

/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/

zbSocDoorLockAppCallbacks_t zbSocDoorLock_YgsAppCallbacks = 
{
	zbSoc_DoorLock_YgsOpenCB,
	zbSoc_DoorLock_YgsPowerValueCB,
	zbSoc_DoorLock_YgsDeviceInfoCB,
	zbSoc_DoorLock_YgsSetPINCodeCB,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	zbSoc_DoorLock_YgsSetPollRateCB,	
};

zbSocDoorLockCallbacks_t	zbSocDoorLock_YgsCallBack_t = 
{
	 ZCL_HA_DEVICEID_FXZB_DOORLOCK_YGS,
	 &zbSocDoorLock_YgsAppCallbacks 
};

/*********************************************************************
* LOCAL VARIABLES
*/

static zclDoorLockSt_t zclDoorLock;

/*********************************************************************/

static uint8_t zclFXDoorLock_calBCC( uint8_t *str, uint16_t len )
{
	uint8_t bcc = 0;
	while( len-- )
	{
		bcc ^= *str++;
	}
	return bcc;
}


static uint8_t doorlock_ygsPackage(uint8_t *dst,uint8_t cmd,uint8_t *src,uint8_t srclen)
{
	if(dst == NULL)
		return 0;

	dst[PKG_HEAD_INDEX] = PKG_HEAD;
	dst[PKG_BCC_INDEX]  = 0;
	dst[PKG_LEN_INDEX]  = srclen;
	dst[PKG_CMD_INDEX]  = cmd;
	
	if(srclen && (src != NULL))
		memcpy(&dst[PKG_PAYLOAD_INDEX],src,srclen);
		
	dst[PKG_HEAD_SIZE + srclen -1]	= PKG_TAIL;

	dst[PKG_BCC_INDEX]  = zclFXDoorLock_calBCC(&dst[PKG_LEN_INDEX],(PKG_HEAD_SIZE + srclen-3));
	
	return (PKG_HEAD_SIZE + srclen);

}


/*串口透传*/
static void doorlock_SerialNet(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t len,uint8_t* buf)
{
		hostCmd cmd;
		cmd.idx =0;

		cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
		cmdSet8bitVal(&cmd, 0);//len
		cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
		cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
		cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
		cmdSet16bitVal_lh(&cmd, dstAddr);
		cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
		cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
		cmdSet8bitVal(&cmd, 0);//DataLen 
		cmdSet8bitVal(&cmd, addrMode);//Addr mode
		cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
		cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
		cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

		cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_SERIALNET_OVER_EVENT);//Attr ID
		cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

		cmdSet8bitVal(&cmd, len);//数据长度
		cmdSetStringVal(&cmd,buf,len);//数据内容
	 
		zbMakeMsgEnder(&cmd);
		
		usleep(1000);//300ms
		zbSocCmdSend(cmd.data,cmd.idx);
}


// clock calibration 时钟校准
static uint8_t zbSoc_DoorLock_ClockCalibration(epInfo_t *epInfo)
{
	//	uint8_t cmds[] = {7E 50 0C 03 01 17 04 14 09 59 09 00 00 00 00 00 7F};
	uint8_t cmds[128] = {0};
	uint8_t cmdslen = 0;
	time_t timep;
	struct tm *p = NULL;
	uint8_t msg[12] = {0};
	

	if(epInfo == NULL)
		return FAILED;

	time(&timep);
	
	p = localtime(&timep);
	msg[0] = 0x01;
	msg[1] = (1900+p->tm_year-2000);
	msg[2] = 1+p->tm_mon;
	msg[3] = p->tm_mday;
	msg[4] = p->tm_hour;
	msg[5] = p->tm_min;
	msg[6] = p->tm_sec;

	cmdslen = doorlock_ygsPackage(cmds,DL_CMD_SET_SYS_TIME,msg,12);
	if(cmdslen > 0)
	{
		doorlock_SerialNet(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmdslen,cmds);
	}

	return SUCCESS;
}

uint8_t zbSoc_DoorLock_YgsOpenCB(epInfo_t *epInfo,uint8_t status)
{
	uint8_t PinCodeLen = 0;
	uint8_t doorlockCmd = COMMAND_CLOSURES_LOCK_DOOR;

	if(status == DOORLOCK_CMD_CLOSE)
		doorlockCmd = COMMAND_CLOSURES_LOCK_DOOR;
	else if(status == DOORLOCK_CMD_OPEN)
		doorlockCmd = COMMAND_CLOSURES_TOGGLE_DOOR;
	else
		doorlockCmd = COMMAND_CLOSURES_UNLOCK_DOOR;

	doorLock_SetLockOrUnLock(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,doorlockCmd,PinCodeLen,NULL);
	return SUCCESS;
}

uint8_t zbSoc_DoorLock_YgsDeviceInfoCB(epInfo_t *epInfo)
{
	uint8_t cmds[] = {0x7E,0x08 ,0x00,0x08,0x7f};

	doorlock_SerialNet(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,5,cmds);
	
	return SUCCESS;
}

uint8_t zbSoc_DoorLock_YgsSetPINCodeCB(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen)
{
	uint8_t ret = SUCCESS;
	
	zclDoorLockSetPINCode_t cmd;
	
	if(epInfo == NULL)
		return FAILED;

	switch(pwdOpert)
	{
		case DOORLOCK_USR_ADD:
		{
			if(pwdType == DOORLOCK_PWD_ADMIN)
			{
				cmd.userType = YGS_PASSWD_TYPE_ADMIN;
			}
			else
			{
				cmd.userType = YGS_PASSWD_TYPE_NORMAL;
			}
			
			cmd.userID = userid;
			
			//enable user by default 0:enable 1:disable
			cmd.userStatus = 0x00;

			if(pwdlen < 6 || pwdlen > 10)
			{
				ret = YY_STATUS_FAIL;
				break;
			}
			
			cmd.pPIN = malloc(pwdlen+1);
			if(cmd.pPIN != NULL)
			{
					cmd.pPIN[0] = pwdlen;
					memcpy(&cmd.pPIN[1],pwd,pwdlen);

					doorlock_SetPINCode(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,&cmd);
			}
			else
			{
				ret = YY_STATUS_FAIL;
			}

			free(cmd.pPIN);
		}
		break;
		case DOORLOCK_USR_DEL:
		{
				log_debug("doorlock_ClearPINCode\n");
				doorlock_ClearPINCode(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,userid);
		}
		break;
		case DOORLOCK_USR_CLEAR:
		{
				doorlock_ClearAllPINCode(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
		break;
		default:
		ret = YY_STATUS_FAIL;
		break;
	}	

	return ret;
}

uint8_t zbSoc_DoorLock_YgsPowerValueCB(epInfo_t *epInfo)
{
	log_debug("zbSoc_DoorLock_YgsPowerValueCB++\n");	

	uint16_t value = 0;
	uint8_t batalarm = 0;
	//epInfo_t * epInfo = vdevListGetDeviceByNaEp(dstAddr,endpoint);
	if(epInfo != NULL)
	{
		 value = vdevListGetDoorBattery(epInfo);
		 if(value == 0XFFFF)
		 {
			zbSoc_DoorLock_YgsDeviceInfoCB(epInfo);
		 }
		 else
		 {
	 		batalarm = vdevListGetDoorAlarm(epInfo);
			SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,batalarm,DOORLOCK_POWER_TYPE_VALUE,value);
			
			/*检查到低压报警*/
			if( batalarm == 0x01 )
			{
				SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_LOW_BATTERY,0x00,NULL);
			}		
		 } 

		 return SUCCESS;
	}
	
	log_debug("zbSoc_DoorLock_YgsPowerValueCB--\n");	
	
	return FAILED;
}

/*设置DataRequest间隔时间*/
uint8_t zbSoc_DoorLock_YgsSetPollRateCB(epInfo_t *epInfo,uint16_t times)
{
	uint8_t ret = YY_STATUS_SUCCESS;

	if(epInfo != NULL)
	{
		doorlock_SetShortPollRate(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,times);		
	}
	else
	{
		ret = YY_STATUS_FAIL;
	}

	return ret;
}

void zbSoc_DoorLock_YgsCtrlInd(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t dataType;
	uint8_t doorlockStatus = 0;
	uint8_t lastDoorlockStu = 0;
	log_debug("zbSoc_DoorLock_YgsCtrlInd++\n");
	cmdGet8bitVal(cmd, &dataType);
	
	if(dataType == ZCL_DATATYPE_ENUM8)
	{
		cmdGet8bitVal(cmd,&doorlockStatus);

		lastDoorlockStu = vdevListGetDoorState(epInfo);

		if(doorlockStatus == CLOSURES_LOCK_STATE_UNLOCKED && lastDoorlockStu != DOORLOCK_CMD_OPEN)
		{
			zclDoorLock.operate = DOORLOCK_TYPE_REMOTE; 
			zclDoorLock.doorlockSt = DOORLOCK_CMD_OPEN;
			zclDoorLock.userID = 0x0000;
			zclDoorLock.timeVail = 0x00;
		}
		else
		{
			if(zclDoorLock.doorlockSt != DOORLOCK_CMD_CLOSE)
				zclDoorLock.upflags = TRUE;
			else
				zclDoorLock.upflags = FALSE;
			
			zclDoorLock.doorlockSt = DOORLOCK_CMD_CLOSE;
		}
	
		vdevListSetDoorState(epInfo, zclDoorLock.doorlockSt);
		if(!zclDoorLock.upflags)
			SRPC_DoorLockCtrlIndCB(epInfo,SUCCESS,zclDoorLock.doorlockSt,zclDoorLock.operate,zclDoorLock.userID,zclDoorLock.timeVail,NULL);
	}
	
	log_debug("zbSoc_DoorLock_YgsCtrlInd--\n");
}

void zbSoc_DoorLock_YgsShortPollRateInd(epInfo_t *epInfo,hostCmd *cmd)
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

void zbSoc_DoorLock_YgsSerialNet(epInfo_t *epInfo,uint8_t len,hostCmd *cmd)
{
	uint8_t zbSocCmd = 0;

	log_debug("zbSoc_DoorLock_YgsSerialNet++\n");
	cmdGet8bitVal(cmd, &zbSocCmd);
	
	switch(zbSocCmd)
	{
		/*密码开门日志上报*/
		case DL_SN_CMD_PWD_RECORD:
		{
			uint8_t sta = 0;
			uint8_t openType = 0;
			uint8_t pwdtype = 0;
			uint8_t pwdNo =0;
			
			cmdGet8bitVal(cmd, &sta);
			cmdGet8bitVal(cmd, &openType);
			cmdGet8bitVal(cmd, &pwdtype);
			cmdGet8bitVal(cmd, &pwdNo);

			if(sta == 0xE2)
			{
				zclDoorLock.doorlockSt = DOORLOCK_CMD_OPEN;

				if(openType == 0xF1)
					zclDoorLock.operate = DOORLOCK_TYPE_CARD ;
				else if(openType == 0xF2)
					zclDoorLock.operate = DOORLOCK_TYPE_PWD ;
				else if(openType == 0xF3)
					zclDoorLock.operate = DOORLOCK_TYPE_FINGER ;
				else if(openType == 0xF3)
					zclDoorLock.operate = DOORLOCK_TYPE_OTHER ;
				else
					zclDoorLock.operate = DOORLOCK_TYPE_REMOTE ;

				zclDoorLock.userID = pwdNo;
				
				SRPC_DoorLockCtrlIndCB(epInfo,SUCCESS,zclDoorLock.doorlockSt,zclDoorLock.operate,zclDoorLock.userID,zclDoorLock.timeVail,NULL);
			}
		}
		break;
		/*密码增加/修改/删除状态上报*/
		case DL_SN_CMD_PWD_ACTIVATE:
		{
			SRPC_DoorLockPINCodeIndCB(epInfo,SUCCESS);
		}
		break;
		/*报警上报*/
		case DL_SN_CMD_ALARM_IND:
		{
				uint8_t alarm = 0;
				
				cmdGet8bitVal(cmd, &alarm);
				
				if(alarm == 0x02 || alarm == 0x04)
				{
					vdevListSetDoorAlarm(epInfo, DOORLOCK_POWER_ST_LOW);
					SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,DOORLOCK_POWER_ST_LOW,DOORLOCK_POWER_TYPE_STATUS,DOORLOCK_POWER_ST_LOW);
				}else if(alarm == 0x05){
					SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_PWD_LOCKED,0x00,NULL);
				}
		}
		break;
	}
	log_debug("zbSoc_DoorLock_YgsSerialNet--\n");
}

void zbSoc_DoorLock_YgsAlarmInd(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t lockAlarm = 0;
	uint16_t adval = 0;
	uint8_t lockstu = 0;
	uint8_t batAlarmFalg,doubleLockStu;
	
	cmdGet8bitVal(cmd, &lockAlarm);
	cmdGet16bitVal(cmd, &adval);

	lockstu = vdevListGetDoorState(epInfo);
	batAlarmFalg =  vdevListGetDoorAlarm(epInfo);

	log_debug("lockAlarm = %d\n",lockAlarm);
	//反锁报警
	if(lockAlarm == YGS_ALARM_DOUBLE_LOCK_LOCKED || lockAlarm == YGS_ALARM_DOUBLE_LOCK_UNLOCK)
	{
		lockAlarm = (lockAlarm==YGS_ALARM_DOUBLE_LOCK_LOCKED)?DOORLOCK_DLSTU_LOCKED:DOORLOCK_DLSTU_UNLOCK;
		vdevListSetDoorDLockStu(epInfo, lockAlarm);
		SRPC_DoorLockDeviceInfoIndCB(epInfo,SUCCESS,lockstu,batAlarmFalg,lockAlarm,epInfo->onlineDevRssi);
	}

	//低电量报警
	if(lockAlarm == YGS_ALARM_LOW_POWR)
	{
		vdevListSetDoorAlarm(epInfo, DOORLOCK_POWER_ST_LOW);
		SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,DOORLOCK_POWER_ST_LOW,DOORLOCK_POWER_TYPE_STATUS,DOORLOCK_POWER_ST_LOW);
	}
	
}

void zbSoc_DoorLock_YgsInfoInd(epInfo_t *epInfo,uint8_t len,hostCmd *cmd)
{
	uint8_t index = 0;
	uint8_t sta;
	YGS_DoorLock_Status_t doorLockStu;

	cmdGet8bitVal(cmd, &sta);
	
	if(sta == SUCCESS)
	{
		cmdGet8bitVal(cmd, &doorLockStu.motorStu);
		cmdGet8bitVal(cmd, &doorLockStu.doubleLockStu);
		cmdGet8bitVal(cmd, &doorLockStu.blotStu);
		cmdGet8bitVal(cmd, &doorLockStu.cardIdCnt);
		cmdGet16bitVal_lh(cmd, &doorLockStu.adVal);
		/*1:电压高于报警电压 0:电压低于报警电压*/
		cmdGet8bitVal(cmd, &doorLockStu.batAlarmFalg);
		cmdGet8bitVal(cmd, &doorLockStu.activeStu);

		//更新门锁状态
		vdevListSetDoorState(epInfo,doorLockStu.motorStu);
		/*反锁状态*/
		vdevListSetDoorDLockStu(epInfo, doorLockStu.doubleLockStu);
		vdevListSetDoorBattery(epInfo,DOORLOCK_POWER_TYPE_VALUE,doorLockStu.adVal);

		log_debug("doorLockStu.batAlarmFalg1 = %d\n",doorLockStu.batAlarmFalg);
		//设置报警信息
		/*1:电压高于报警电压 0:电压低于报警电压*/
		doorLockStu.batAlarmFalg = (doorLockStu.batAlarmFalg==0x00)?DOORLOCK_POWER_ST_LOW:DOORLOCK_POWER_ST_NORMAL;

		vdevListSetDoorAlarm(epInfo,doorLockStu.batAlarmFalg);
		vdevListSetDoorBatteryStu(epInfo, DOORLOCK_POWER_TYPE_STATUS, doorLockStu.batAlarmFalg);

		log_debug("doorLockStu.batAlarmFalg2 = %d\n",doorLockStu.batAlarmFalg);

		//上报门锁状态
		if(doorLockStu.batAlarmFalg)
			SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_LOW_BATTERY,0x00,NULL);

		SRPC_DoorLockDeviceInfoIndCB(epInfo,SUCCESS,doorLockStu.motorStu,doorLockStu.batAlarmFalg,doorLockStu.doubleLockStu,epInfo->onlineDevRssi);
		SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,doorLockStu.batAlarmFalg,DOORLOCK_POWER_TYPE_VALUE,doorLockStu.adVal);
	}
		
}

/*透传功能数据上报*/
void zbSoc_DoorLock_YgsSerialNetInd(epInfo_t *epInfo,hostCmd *cmd)
{
		uint8_t dataType;
		uint8_t datalen = 0;
		//uint8_t databuf[128] = {0};
		uint16_t deviceType = 0;

		uint8_t zbSocSof = 0;
		uint8_t zbSocBcc = 0;
		uint8_t zbSocLen = 0;
		uint8_t zbSocCmd = 0;

		log_debug("zbSoc_DoorLock_YgsSerialNetInd++\n");
		cmdGet8bitVal(cmd, &dataType);

		if(dataType != ZCL_DATATYPE_OCTET_STR)
			return;
		
		cmdGet8bitVal(cmd, &datalen);
		//cmdGetStringVal(cmd,databuf,datalen);

		if(datalen < PKG_MIN_SIZE)
			return;

		cmdGet16bitVal(cmd, &deviceType);
		cmdGet8bitVal(cmd, &zbSocSof);
		cmdGet8bitVal(cmd, &zbSocBcc);
		cmdGet8bitVal(cmd, &zbSocLen);
		cmdGet8bitVal(cmd, &zbSocCmd);

		switch(zbSocCmd)
		{
			case YGS_SERIALNET_CMD_IND:
			case YGS_SERIALNET_CMD_REQ:
			{
				zbSoc_DoorLock_YgsSerialNet(epInfo,zbSocLen,cmd);
			}
			break;
			//报警
			case DL_CMD_ALARM:
				zbSoc_DoorLock_YgsAlarmInd(epInfo,cmd);
				break;
			case DL_CMD_INFO :
			{
				zbSoc_DoorLock_YgsInfoInd(epInfo,zbSocLen,cmd);
			}
			break;
			
			//校准时钟操作
			case DL_CMD_TIME_REQ:
				zbSoc_DoorLock_ClockCalibration(epInfo);
				break;
			default:
				log_debug("Unsuppert CMD 0x%x\n",zbSocCmd);
			break;
		}
		
		log_debug("zbSoc_DoorLock_YgsSerialNetInd--\n");
}

void zbSoc_DoorLock_YgsSerialMsg_Event(epInfo_t *epInfo,hostCmd *cmd)
{
	uint16_t attrID;
	uint8_t dataType;

	log_debug("zbSoc_DoorLock_YgsSerialMsg_Event++\n");

	if(epInfo == NULL)
		return;

	cmdGet16bitVal_lh(cmd, &attrID);

	switch(attrID)
	{
		case ATTRID_CLOSURES_LOCK_STATE:
		{
			zbSoc_DoorLock_YgsCtrlInd(epInfo,cmd);
		}
		break;
		case ATTRID_POLL_CONTROL_SHORT_POLL_INTERVAL:
		{
			zbSoc_DoorLock_YgsShortPollRateInd(epInfo,cmd);
		}
		break;
		case ATTRID_BASIC_SERIALNET_OVER_EVENT:
		{
			zbSoc_DoorLock_YgsSerialNetInd(epInfo,cmd);
		}
		break;
		default:
			log_debug("Unsuppert attrID 0x%x\n",attrID);

			break;
	}
	
	log_debug("zbSoc_DoorLock_YgsSerialMsg_Event--\n");
}

/*********************************************************************/

