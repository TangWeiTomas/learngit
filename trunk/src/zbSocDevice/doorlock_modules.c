/**************************************************************************
 * Filename:       doorlock_modules.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    普通门锁模块控制接口
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/


/*********************************************************************
* INCLUDES
*/
#include "doorlock_modules.h"

#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"
	
#include "zbSocPrivate.h"
//#include "interface_srpcserver_defs.h"

#include "GwComDef.h"
#include "doorlock.h"
#include "doorlock_zbSocCmds.h"

/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/

/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* LOCAL FUNCTIONS
*/
uint8_t zbSoc_DoorLock_modulesOpenCB(epInfo_t *epInfo,uint8_t status);
uint8_t zbSoc_DoorLock_modulesPowerValueCB(epInfo_t *epInfo);
uint8_t zbSoc_DoorLock_modulesSetPollRateCB(epInfo_t *epInfo,uint16_t times);
uint8_t zbSoc_DoorLock_modulesDeviceInfoCB(epInfo_t *epInfo);

/*********************************************************************
* LOCAL VARIABLES
*/
	zbSocDoorLockAppCallbacks_t zbSocDoorLock_modulesAppCallbacks = 
	{
		zbSoc_DoorLock_modulesOpenCB,
		zbSoc_DoorLock_modulesPowerValueCB,
		zbSoc_DoorLock_modulesDeviceInfoCB,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		zbSoc_DoorLock_modulesSetPollRateCB,	
	};

/*********************************************************************
* GLOBAL VARIABLES
*/

zbSocDoorLockCallbacks_t	zbSocDoorLock_modulesCallBack_t = 
{
	 ZB_DEV_ONOFF_DOORLOCK,
	 &zbSocDoorLock_modulesAppCallbacks 
};

uint8_t zbSoc_DoorLock_modulesOpenCB(epInfo_t *epInfo,uint8_t status)
{
	/*
	uint8_t PinCodeLen = 0;
	uint8_t doorlockCmd = COMMAND_CLOSURES_LOCK_DOOR;

	if(status == DOORLOCK_CMD_CLOSE)
		doorlockCmd = COMMAND_CLOSURES_LOCK_DOOR;
	else if(status == DOORLOCK_CMD_OPEN)
		doorlockCmd = COMMAND_CLOSURES_TOGGLE_DOOR;
	else
		doorlockCmd = COMMAND_CLOSURES_UNLOCK_DOOR;
	*/
	
	zbSoc_SetGenOnOffState(status,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
	//doorLock_SetLockOrUnLock(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,doorlockCmd,PinCodeLen,NULL);
	return SUCCESS;
}

uint8_t zbSoc_DoorLock_modulesPowerValueCB(epInfo_t *epInfo)
{
	log_debug("zbSoc_DoorLock_YgsPowerValueCB++\n");	

	uint16_t value = 0;
	uint8_t batalarm = 0;
	
	if(epInfo != NULL)
	{
		zbSoc_QueryDoorLockPowerValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		/*
		 value = vdevListGetDoorBattery(epInfo);
		 if(value == 0XFFFF)
		 {
				zbSoc_DoorLock_YgsDeviceInfoCB(epInfo);
		 }
		 else
		 {
		 		batalarm = vdevListGetDoorAlarm(epInfo);
				SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,batalarm,DOORLOCK_POWER_TYPE_VALUE,value);
		 } 
		*/
		 return SUCCESS;
	}
	
	log_debug("zbSoc_DoorLock_YgsPowerValueCB--\n");	
	
	return FAILED;
}

uint8_t zbSoc_DoorLock_modulesDeviceInfoCB(epInfo_t *epInfo)
{
	uint8_t doorlockStu = 0;
	uint8_t batalarm  =0;
	ASSERT(epInfo!=NULL);
		
	doorlockStu = vdevListGetDoorState(epInfo);
	if(doorlockStu == 0xFF)
	{
		doorlockStu = DOORLOCK_STATUS_CLOSE;
		vdevListSetDoorState(epInfo, doorlockStu);
		vdevListSetDoorAlarm(epInfo, DOORLOCK_POWER_ST_NORMAL);
	}

	batalarm = vdevListGetDoorAlarm(epInfo);
	
	SRPC_DoorLockDeviceInfoIndCB(epInfo,SUCCESS,doorlockStu,batalarm,DOORLOCK_DLSTU_UNSUP,epInfo->onlineDevRssi);
	
	return SUCCESS;
}

uint8_t zbSoc_DoorLock_modulesSetPollRateCB(epInfo_t *epInfo,uint16_t times)
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

void  zbSoc_DoorLock_modulesLockStu(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t dataType;
	uint8_t status;

	ASSERT(epInfo!=NULL && cmd !=NULL);
	
	cmdGet8bitVal(cmd, &dataType); //get data type;

	//接收到返回，关闭重发机制
	zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

	cmdGet8bitVal(cmd, &status);
	
	SRPC_DoorLockCtrlIndCB(epInfo,SUCCESS,status,DOORLOCK_TYPE_REMOTE,0x00,0x00,NULL);
	//控制门锁节点的状态变化上报
	//SRPC_DoorLockCtrlInd(epInfo->IEEEAddr,epInfo->endpoint,status,epInfo->onlineDevRssi);

	//更新设备状态
	vdevListSetDoorState(epInfo,status);
	zbSoc_ProcessEvent(epInfo,status);
}

void  zbSoc_DoorLock_modulesPowerStu(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t dataType;
	uint8_t powerValue;
	uint16_t pwrValue = 0;
	uint8_t alarmStatus = 0;
	
	ASSERT(epInfo!=NULL && cmd !=NULL);
	
	cmdGet8bitVal(cmd, &dataType); //get data type;
	cmdGet8bitVal(cmd, &powerValue);
	
	//门锁节点的电量值上报
	log_debug("SRPC_DoorLockPowerValueInd :PowerValue= %d\n",powerValue);
	
	//SRPC_DoorLockPowerValueInd(epInfo->IEEEAddr,epInfo->endpoint,powerValue,epInfo->onlineDevRssi);
	pwrValue = powerValue / 100 * 6;

	if(pwrValue < 3600)
			alarmStatus = DOORLOCK_POWER_ST_LOW;
	else
			alarmStatus = DOORLOCK_POWER_ST_NORMAL;

	SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,alarmStatus,DOORLOCK_POWER_TYPE_VALUE,pwrValue);

	if(alarmStatus)
			SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_LOW_BATTERY,0x00,NULL);
	
	//更新设备状态值
	vdevListSetDoorBattery(epInfo,DOORLOCK_POWER_TYPE_VALUE,powerValue);
	vdevListSetDoorAlarm(epInfo,alarmStatus);
	
}

void  zbSoc_DoorLock_modulesSpeedStu(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t dataType;
	uint16_t mValue = 0;

	ASSERT(epInfo!=NULL && cmd !=NULL);

	cmdGet8bitVal(cmd, &dataType); //get data type;
	cmdGet16bitVal_lh(cmd,&mValue);
	//SRPC_SetDoorLockSpeedCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS,mValue);
}


void zbSoc_DoorLock_modulesShortPollRateInd(epInfo_t *epInfo,hostCmd *cmd)
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

/*********************************************************************
* @fn          zbSoc_DoorLock_modulesIncomming
*
* @brief       处理门锁模块上报数据
*
* @param       epInfo - SimpleDescRsp containing epInfo of new EP.
* @param       cmd 	- SimpleDescRsp containing epInfo of new EP.
*
* @return      index of device or 0xFF if no room in list
*/
int zbSoc_DoorLock_modulesIncomming(epInfo_t *epInfo,hostCmd *cmd)
{
	uint16_t attrID;

	ASSERT(epInfo!=NULL && cmd !=NULL);
	
	cmdGet16bitVal_lh(cmd, &attrID);

	switch(attrID)
	{
		case ATTRID_BASIC_LOCK_UNLOCK:
			zbSoc_DoorLock_modulesLockStu(epInfo,cmd);
			break;
		case ATTRID_BASIC_POWER_VALUE:
			zbSoc_DoorLock_modulesPowerStu(epInfo,cmd);
			break;
		case ATTRID_BASIC_MOTOR_SPEED:
			zbSoc_DoorLock_modulesSpeedStu(epInfo,cmd);
			break;
		case ATTRID_POLL_CONTROL_SHORT_POLL_INTERVAL:
			zbSoc_DoorLock_modulesShortPollRateInd(epInfo,cmd);
			break;
		default:break;
	}

	return SUCCESS;
}


/*********************************************************************
* @fn          funtion_name
*
* @brief       Add device to descovery list.
*
* @param       pSimpleDescRsp - SimpleDescRsp containing epInfo of new EP.
*
* @return      index of device or 0xFF if no room in list
*/

/*********************************************************************/

