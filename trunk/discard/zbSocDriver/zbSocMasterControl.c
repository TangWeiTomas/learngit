/**************************************************************************************************
 * Filename:       zbSocMasterControl.c
 * Author:             edward
 * E-Mail:          ouxiangping@feixuekj.cn
 * Description:    特殊设备，主控设备管理器
 *
 *  Copyright (C) 2014 fei xue keji Company - http://www.feixuekj.cn
 *
 * Version:         1.00  (2014-11-30,10:03)    :   Create the file.
 *
 *
 *************************************************************************/

//#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <fcntl.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>

#include "Zigbee_device_Heartbeat_Manager.h"
#include "interface_vDeviceList.h"
#include "zbSocUart.h"
#include "zbSocCmd.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "errorCode.h"
#include "interface_devicelist.h"
#include "logUtils.h"
#include "Polling.h"
#include "PackageUtils.h"
#include "interface_srpcserver.h"
#include "One_key_match.h"
#include "Out_Of_Power.h"
#include "zbSocMasterControl.h"

#define MASTER_CONTROL_VIRTUAL_DEVICE_SIZE			16
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_SIZE	14

//虚拟插卡取电设备
#define MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH	 0x01
//虚拟门磁
#define MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT	 0X02 
//虚拟开关操作
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE2	 0X03
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE3	 0X04 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE4	 0X05 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE5	 0X06 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE6	 0X07 

#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE7	 0X09 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE8	 0X0a 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE9	 0X0b 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE10	 0X0c 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE11	 0X0d 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE12	 0X0e 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE13	 0X0f 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE14	 0X10 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE15	 0X11 


#define MASTER_CONTROL_ENDPOINT		0x08

const int MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH[MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_SIZE] = {
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE2,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE3,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE4,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE5,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE6,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE7,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE8,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE9,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE10,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE11,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE12,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE13,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE14,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE15
};

//龙溪主控设备注册
void zbSoc_MasterControlRegister(epInfoExtended_t *epInfoEx)
{	
	epInfo_t *epinfo;
	uint32_t context = 0;
	
    epInfo_t newData;
	epInfo_t virtualDevice;//虚拟设备

	//如果是主控设备，则添加虚拟节点设备
	if(epInfoEx->epInfo->deviceID == ZB_DEV_MASTER_CONTROL)
	{
		//保存新的数据
		memcpy(&newData,epInfoEx->epInfo,sizeof(epInfo_t));

		//更新
		if(epInfoEx->type == EP_INFO_TYPE_UPDATED) //更新现有的设备
		{
			//查找符合要求的数据
			while((epinfo = devListGetNextDev(&context)) != NULL)
			{
				//删除所有符合要求的数据
				if((memcmp(newData.IEEEAddr,epinfo->IEEEAddr,Z_EXTADDR_LEN) == 0)&&(epinfo->deviceID != ZB_DEV_MASTER_CONTROL))
				{
					memcpy(&virtualDevice,epinfo,sizeof(epInfo_t));
					
					//devListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
					if(epinfo->deviceID == ZB_DEV_POWER_SWITCH)			//插卡取电设备更新
					{
						virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH;
						virtualDevice.deviceID = ZB_DEV_POWER_SWITCH;
						devListModifyRecordByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
						vdevListModifyByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
					}
					else if(epinfo->deviceID == ZB_DEV_DOOR_SENSOR)
					{
						virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT;
						virtualDevice.deviceID = ZB_DEV_DOOR_SENSOR;
						devListModifyRecordByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
						vdevListModifyByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
					}
					else if(epinfo->deviceID == ZB_DEV_ONOFF_SWITCH)
					{
						virtualDevice.endpoint = epinfo->endpoint;
						virtualDevice.deviceID = epinfo->deviceID;
						devListModifyRecordByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
						vdevListModifyByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
					}
				}
			}
		}
		else if(epInfoEx->type == EP_INFO_TYPE_NEW) //添加新设备
		{
			//删除原有的虚拟设备信息
			while((epinfo = devListGetNextDev(&context)) != NULL)
			{
				//删除所有符合要求的数据
				if((memcmp(newData.IEEEAddr,epinfo->IEEEAddr,8) == 0)&&(epinfo->deviceID != ZB_DEV_MASTER_CONTROL))
				{
					devListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
					vdevListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
				}
			}
			
			//添加虚拟设备
			memcpy(&virtualDevice,&newData,sizeof(epInfo_t));
			epInfoEx->epInfo = &virtualDevice;

			//1.添加取电开关
			virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH;
			virtualDevice.deviceID = ZB_DEV_POWER_SWITCH;
			devListAddDevice(&virtualDevice);
			zbSoc_DevRegisterReporting(epInfoEx);

			//2.添加门磁感应
			virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT;
			virtualDevice.deviceID = ZB_DEV_DOOR_SENSOR;
			devListAddDevice(&virtualDevice);
			zbSoc_DevRegisterReporting(epInfoEx);

			//3.添加14路开关
			for(context = 0; context < MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_SIZE;context++)
			{
				virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH[context];
				virtualDevice.deviceID = ZB_DEV_ONOFF_SWITCH;
				devListAddDevice(&virtualDevice);
				zbSoc_DevRegisterReporting(epInfoEx);
			}
		}
		
	}
	else
	{
		zbSoc_DevRegisterReporting(epInfoEx);
	}
}


void zbSoc_MasterControlSetCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t deviceType,uint8_t switchCmd)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_LXZK_DEV_SET);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_UINT16);//Data Type

    cmdSet8bitVal(&cmd, deviceType);//设备类型，主控的第几路设备
	cmdSet8bitVal(&cmd, switchCmd);//设备状态
   
    zbMakeMsgEnder(&cmd);
    
//	usleep(300000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}

//检测是否是虚拟设备
uint8_t zbSoc_MasterControlCheck(uint8_t* ieeeAddr,uint8_t endpoint)
{
	epInfo_t *epInfo = NULL;
	epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,MASTER_CONTROL_ENDPOINT);
	
	if(epInfo!=NULL)
	{
		if(epInfo->deviceID == ZB_DEV_MASTER_CONTROL)
		{
			return true;
		}
	}
	
	return false;
}

void zbSoc_MasterControlQueryOnOffStateCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,uint8_t deviceType)
{
    hostCmd cmd;
    cmd.idx =0;

//    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
//    cmdSet8bitVal(&cmd, 0);					//len 预留位
//    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
//    cmdSet8bitVal(&cmd, MT_APP_MSG);		//CMD1
//    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
//    //发送给所有的路由设备,都开放网络
//    cmdSet16bitVal_lh(&cmd, dstAddr);
//    cmdSet8bitVal(&cmd, endpoint);			//Dest APP Endpoint
//    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
//    cmdSet8bitVal(&cmd, 0);					//DataLen 预留位
//    cmdSet8bitVal(&cmd, addrMode);			//Addr mode
//    cmdSet8bitVal(&cmd, 0x00);				//Zcl Frame control
//    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
//    cmdSet8bitVal(&cmd, ZCL_CMD_READ);		//ZCL COMMAND ID
//    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_LX_MASTER);	//Attr ID //读取主控设备状态

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

    cmdSet16bitVal_lh(&cmd, ATTRID_LXZK_DEV_GET);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_UINT8);//Data Type

    cmdSet8bitVal(&cmd, deviceType);//设备类型，主控的第几路设备
//	cmdSet8bitVal(&cmd, switchCmd);//设备状态
   
    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}


uint8_t zbSoc_MasterControlSetOnOffState(epInfo_t *epInfo,uint8_t switchcmd)
{
	log_debug("zbSoc_MasterControlSetOnOffState++\n");

//	vepInfo_t *vepInfo = NULL;
	epInfo_t *pvepInfo = NULL;
	
	ASSERT(epInfo != NULL);
	
	//memcpy(&mEpInfo,epInfo,sizeof(epInfo_t));
	
	//判断当前的设备是否主控设备模拟的
	pvepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_ENDPOINT);

	if(pvepInfo != NULL)
	{
//		pvepInfo = &(vepInfo->epInfo);
		if(pvepInfo->deviceID == ZB_DEV_MASTER_CONTROL)//找到设备为主控板设备
		{
			log_debug("is ZB_DEV_MASTER_CONTROL\n");
			//操作主控设备
			zbSoc_MasterControlSetCmd(pvepInfo->nwkAddr,pvepInfo->endpoint,afAddr16Bit,epInfo->endpoint,switchcmd);
		}
		else//找到，但是非主板设备
		{
			log_debug("Not ZB_DEV_MASTER_CONTROL\n");
			zbSoc_SetGenOnOffState(switchcmd,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
	}
	else	//没找到DevieID为主控板设备
	{
		log_debug("Not ZB_DEV_MASTER_CONTROL\n");
		zbSoc_SetGenOnOffState(switchcmd,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
	}
	
	log_debug("zbSoc_MasterControlSetOnOffState--\n");
	return 0;
}


uint8_t zbSoc_MasterControlQueryOnOffState(epInfo_t *epInfo)
{
	epInfo_t mEpInfo;

//	vepInfo_t *vepInfo = NULL;
	epInfo_t *pvepInfo = NULL;
	
	ASSERT(epInfo != NULL);
	
	//memcpy(&mEpInfo,epInfo,sizeof(epInfo_t));

	pvepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_ENDPOINT);
	
	if(pvepInfo != NULL)
	{
//		pvepInfo = &(vepInfo->epInfo);
		if(pvepInfo->deviceID == ZB_DEV_MASTER_CONTROL)//找到设备为主控板设备
		{
			//操作主控设备
			zbSoc_MasterControlQueryOnOffStateCmd(pvepInfo->nwkAddr,pvepInfo->endpoint,afAddr16Bit,epInfo->endpoint);
		}
		else//找到，但是非主板设备
		{
			zbSoc_QueryLightValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
	}
	else	//没找到DevieID为主控板设备
	{
		zbSoc_QueryLightValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
	}
	
	return 0;
}

//虚拟设备，设备状态上报解析
uint8_t zbSoc_MasterControlReportResolve(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	uint8_t deviceType;
	uint8_t deviceState;
	uint8_t deviceCnt = 0;
	uint8_t cnt = 0;
	epInfo_t *devInfo = NULL;

	ASSERT(epInfo != NULL && cmd != NULL);
	
	log_debug("zbSoc_MasterControlReportResolve++\n");

	cmdGet16bitVal_lh(cmd, &attrID);
    cmdGet8bitVal(cmd, &dataType);

	//单个设备状态上报
	if((attrID == ATTRID_BASIC_LX_MASTER) && (dataType == ZCL_DATATYPE_UINT16))
	{
		//注意字节序
		cmdGet8bitVal(cmd, &deviceState);//状态
		cmdGet8bitVal(cmd, &deviceType);//第几路
		
		devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,deviceType);

		if((devInfo==NULL )|| (devInfo->registerflag == false))
			return -1;

		//更新设备中的RSSI
		devInfo->onlineDevRssi = epInfo->onlineDevRssi;
		
		if(deviceType == MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH)
		{
			//接收到返回，关闭重发机制
            zblist_remove(devInfo->nwkAddr,devInfo->endpoint);
			//控制取电开关的状态变化上报
			SRPC_PowerSwitchCtrlInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState,devInfo->onlineDevRssi);
		}
		else if(deviceType == MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT)
		{
			SRPC_ComAlarmStateInd(devInfo->IEEEAddr,devInfo->endpoint,devInfo->deviceID,deviceState);
			if(deviceState == On)//主控门磁的状态与实际的相反
			{
				Out_Of_Power_Start(OUT_OF_POWER_TIME);
			}
		}
		else
		{
			//接收到返回，关闭重发机制
            zblist_remove(devInfo->nwkAddr,devInfo->endpoint);
			//开关节点的开关状态变化上报
            SRPC_SwitchStateInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState,devInfo->onlineDevRssi);
		}
		zbSoc_ProcessEvent(devInfo,deviceState);
		
	}

/*
	if((attrID == ZB_ATTID_MASTER_CONTROL) && (dataType == ZCL_DATATYPE_UINT16))
	{
		
		
		devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,deviceType);

		if((devInfo==NULL )|| (devInfo->registerflag == false))
			return -1;

		//更新设备中的RSSI
		devInfo->onlineDevRssi = epInfo->onlineDevRssi;
		
		if(deviceType == MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH)
		{
			//接收到返回，关闭重发机制
            zblist_remove(devInfo->nwkAddr,devInfo->endpoint);
			//控制取电开关的状态变化上报
			SRPC_PowerSwitchCtrlInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState,devInfo->onlineDevRssi);
		}
		else if(deviceType == MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT)
		{
			SRPC_ComAlarmStateInd(devInfo->IEEEAddr,devInfo->endpoint,devInfo->deviceID,deviceState);
			if(deviceState == On)//主控门磁的状态与实际的相反
			{
				Out_Of_Power_Start(OUT_OF_POWER_TIME);
			}
		}
		else
		{
			//接收到返回，关闭重发机制
            zblist_remove(devInfo->nwkAddr,devInfo->endpoint);
			//开关节点的开关状态变化上报
            SRPC_SwitchStateInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState,devInfo->onlineDevRssi);
		}
		zbSoc_ProcessEvent(devInfo,deviceState);
	}
	
	*/

	log_debug("zbSoc_MasterControlReportResolve--\n");
	return 0;
}

//读取反馈上报
uint8_t zbSoc_MasterControlReadRspResolve(hostCmd *cmd,epInfo_t *epInfo)
{
	uint8_t mCount =0;
	uint8_t datalength = 0;
	uint8_t deviceState[16] = {0};
	epInfo_t *devInfo = NULL;
//	epInfo_t mEpInfo;
	
	if(epInfo == NULL)
		return -1;

	log_debug("zbSoc_MasterControlReadRspResolve--\n");

//	memcpy(&mEpInfo,epInfo,sizeof(epInfo_t));
	
    cmdGet8bitVal(cmd, &datalength);

	if (datalength <= MASTER_CONTROL_VIRTUAL_DEVICE_SIZE)
		cmdGetStringVal(cmd,deviceState,datalength);
	else
		return -1;

	//插卡取电设备
	
	devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH);
	if(devInfo != NULL)
	{
		//更新设备RSSI
		devInfo->onlineDevRssi = epInfo->onlineDevRssi;
		//更新设备状态
		vdevListSetPowerSwitchState(devInfo,deviceState[0]);
		
		SRPC_PowerSwitchCtrlInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState[0],devInfo->onlineDevRssi);
		//zbSoc_ProcessEvent(devInfo,deviceState[0]);
	}
	
	devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT);
	if(devInfo != NULL)
	{
		//更新设备RSSI
		devInfo->onlineDevRssi = epInfo->onlineDevRssi;
		vdevListSetAlarmState(devInfo,deviceState[1]);
		
		SRPC_ComAlarmStateInd(devInfo->IEEEAddr,devInfo->endpoint,devInfo->deviceID,deviceState[1]);
//		zbSoc_ProcessEvent(devInfo,ZCL_DATATYPE_UINT8,&deviceState[1]);
	}

	for(mCount =0;mCount<MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_SIZE;mCount++)
	{
		devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH[mCount]);
		if(devInfo != NULL)
		{
			//更新设备RSSI
			devInfo->onlineDevRssi = epInfo->onlineDevRssi;
			vdevListSetSwitchState(devInfo,deviceState[mCount+2]);
		
			SRPC_SwitchStateInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState[mCount+2],devInfo->onlineDevRssi);
//			zbSoc_ProcessEvent(devInfo,ZCL_DATATYPE_UINT8,&deviceState[mCount+2]);
		}
	}
	log_debug("zbSoc_MasterControlReadRspResolve--\n");
	return 0;
}