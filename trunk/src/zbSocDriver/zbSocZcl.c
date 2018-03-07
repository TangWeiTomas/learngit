/**************************************************************************
 * Filename:       zbSocZcl.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    ZCL协议层处理程序
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/

/*********************************************************************
* INCLUDES
*/
#include "zbSocZcl.h"
#include "interface_vDeviceList.h"
/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/

/*********************************************************************
* TYPEDEFS
*/

typedef struct
{
  uint16_t zoneStatus;     // current zone status - bit map
  uint8_t  extendedStatus; // bit map, currently set to All zeroes ( reserved )
  uint8_t  zoneID;         // allocated zone ID
  uint16_t delay;          // delay from change in ZoneStatus attr to transmission of change notification cmd
} zclZoneChangeNotif_t;

/*********************************************************************
* LOCAL FUNCTIONS
*/

static ZStatus_t zclSS_HdlIncoming( hostCmd *cmd,uint8_t zclFrameLen, uint16_t clusterID, uint16_t nwkAddr, uint8_t endpoint );
/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/

       
/*********************************************************************
* LOCAL VARIABLES
*/

static zclLibPlugin_t zclLibPluginList[] = 
{
	{ZCL_CLUSTER_ID_SS_IAS_ZONE,ZCL_CLUSTER_ID_SS_IAS_WD,zclSS_HdlIncoming},
};


/*******************************************************************************
 * @fn      zclSS_ProcessInCmd_ZoneStatus_ChangeNotification
 *
 * @brief   Process in the received StatusChangeNotification Command.
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   pCBs - pointer to callback functions
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclSS_ProcessInCmd_ZoneStatus_ChangeNotification( hostCmd *cmd,epInfo_t *epInfo )
{
	zclZoneChangeNotif_t msg;
	uint8_t zoneStatus = 0;
	uint8_t zonebattery = 0;

	log_debug("zclSS_ProcessInCmd_ZoneStatus_ChangeNotification++\n");
	
	cmdGet16bitVal_lh(cmd, &msg.zoneStatus);
	cmdGet8bitVal(cmd, &msg.extendedStatus);
	cmdGet8bitVal(cmd, &msg.zoneID);
	cmdGet16bitVal_lh(cmd, &msg.delay);

	zoneStatus 	= msg.zoneStatus & SS_IAS_ZONE_STATUS_ALARM1_ALARMED;
	zonebattery =	(msg.zoneStatus & SS_IAS_ZONE_STATUS_BATTERY_LOW)!=0?SS_IAS_ZONE_STATUS_BATTERY_LOW_ALARM:SS_IAS_ZONE_STATUS_BATTERY_LOW_UNALARM;

	vdevListSetAlarmState(epInfo, zoneStatus);
	vdevListSetAlarmBatteryAlarm(epInfo, zonebattery);

	SRPC_ComAlarmStateInd(epInfo->IEEEAddr,epInfo->endpoint,epInfo->deviceID,zoneStatus);
	log_debug("zclSS_ProcessInCmd_ZoneStatus_ChangeNotification--\n");
  return ( SUCCESS );
}

/*********************************************************************
* @fn          zclSS_ProcessInZoneStatusCmds
*
* @brief       处理Zone状态上报
*
* @param       pSimpleDescRsp - SimpleDescRsp containing epInfo of new EP.
*
* @return      ZStatus_t
*/

static ZStatus_t zclSS_ProcessInZoneStatusCmds(hostCmd *cmd,epInfo_t *epInfo)
{
	ZStatus_t stat;
	uint8_t transSeqNum, commandID,rssi;
	
	log_debug("zclSS_ProcessInZoneStatusCmds++\n");
	
	if(cmd == NULL || epInfo == NULL)
		return FAILED;
	
  	cmdGet8bitVal(cmd, &transSeqNum);
  	cmdGet8bitVal(cmd, &commandID);
  	cmdGet8bitVal(cmd, &rssi);
	
	epInfo->onlineDevRssi = rssi;
	
	switch (commandID)
	{
		case COMMAND_SS_IAS_ZONE_STATUS_CHANGE_NOTIFICATION:
			 stat = zclSS_ProcessInCmd_ZoneStatus_ChangeNotification( cmd, epInfo );
		break;
		case COMMAND_SS_IAS_ZONE_STATUS_ENROLL_REQUEST:

		break;
		default:break;
	}
	
	log_debug("zclSS_ProcessInZoneStatusCmds++\n");

	return stat;
}


/*********************************************************************
* @fn          zclSS_HdlIncoming
*
* @brief       Security and Safety (SS) Clusters process function
*
* @param       pSimpleDescRsp - SimpleDescRsp containing epInfo of new EP.
*
* @return      ZStatus_t
*/
static ZStatus_t zclSS_HdlIncoming( hostCmd *cmd,uint8_t zclFrameLen, uint16_t clusterID, uint16_t nwkAddr, uint8_t endpoint )
{
	epInfo_t *epInfo = NULL;

	log_debug("zclSS_HdlIncoming++\n");
	epInfo = vdevListGetDeviceByNaEp(nwkAddr, endpoint);

	if(epInfo == NULL)
		return FAILED;

	//更新在线信息
	if(vdevListSetDevOnlineState(epInfo,true) != 0)
	{
		ZbSocHeartbeat_HeartPacketSend();
	  	//更新一下心跳包的定时器任务
	  	zbSoc_Heartbeat_DeviceList_Report_refresh();
	}

	//处理消息
	switch(clusterID)
	{
		case ZCL_CLUSTER_ID_SS_IAS_ZONE:
		{
			zclSS_ProcessInZoneStatusCmds(cmd,epInfo);
		}
		break;
		case ZCL_CLUSTER_ID_SS_IAS_ACE:
		{

		}
		break;
		case ZCL_CLUSTER_ID_SS_IAS_WD:
		{
			
		}
		break;
		default:break;
	}

	log_debug("zclSS_HdlIncoming--\n");
	return SUCCESS;
}

/*********************************************************************
* @fn          zclFindplugin
*
* @brief       根据clusterID获取处理函数
*
* @param       clusterID - .
*
* @return      回调函数
*/

zclLibPlugin_t *zclFindplugin(uint16_t clusterID)
{
	uint8_t index = 0;
	uint8_t size = ARRAY_SIZE(zclLibPluginList);
	
	for(index = 0;index < ARRAY_SIZE(zclLibPluginList);index++)
	{
			if((clusterID >= zclLibPluginList[index].startClusterID) &&(clusterID <= zclLibPluginList[index].startClusterID))
			{
				return &zclLibPluginList[index];
			}
	}

	return NULL;
}


/*********************************************************************/

