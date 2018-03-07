#include "scene_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <errno.h>
#include "interface_scenelist.h"

#include "zbSocCmd.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "errorCode.h"
#include "interface_devicelist.h"
#include "logUtils.h"
#include "PackageUtils.h"
#include "interface_srpcserver.h"
#include "DeviceCode.h"
#include "SuccessiveEvents.h"
#include "interface_vDeviceList.h"

uint8_t Device_ProcessEventByDeviceId(epInfo_t *epInfo,uint8_t *data,uint8_t lenght)
{
	log_debug("Device_ProcessEventByDeviceId++\n");

	if((epInfo == NULL)|| (data==NULL)|| (lenght == 0))
		return -1;
	
	log_debug("IEEE: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X , 0x%04X , 0x%02X\n",epInfo->IEEEAddr[0], epInfo->IEEEAddr[1], epInfo->IEEEAddr[2],
            epInfo->IEEEAddr[3], epInfo->IEEEAddr[4], epInfo->IEEEAddr[5],
            epInfo->IEEEAddr[6], epInfo->IEEEAddr[7], epInfo->nwkAddr,
            epInfo->endpoint);
            
	switch(epInfo->deviceID)
	{
		case ZB_DEV_ONOFF_DOORLOCK:
		{
			zbSoc_SetGenOnOffState(data[0],epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);	
		}
		break;
		case ZB_DEV_POWER_SWITCH:
		case ZB_DEV_ONOFF_PLUG:		//开关类设备
		case ZB_DEV_ONOFF_SWITCH:
		{

#if USE_MASTER_CONTROL
			zbSoc_MasterControlSetOnOffState(epInfo,data[0]);
#else
			zbSoc_SetGenOnOffState(data[0],epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
//			zbSoc_SetGenOnOffState(switchCmd,epInfo->nwkAddr,endPoint,afAddr16Bit);
#endif
		}
		break;
		case ZB_DEV_WIN_CURTAIN:
		{
			zbSoc_CurtainDevCtrlCmdReq(data[0],epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
		break;
		case ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE_FXKJ:
		case ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE:
			zbSoc_NewCurtainDevCtrlCmdReq(data[0],epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		break;
		case ZB_DEV_TEMP_HUM:break;
		case ZB_DEV_IR_SENSOR:
		case ZB_DEV_SMOKE_SENSOR:
		case ZB_DEV_MAGNETOMETER_SENSOR:
//		case ZB_DEV_GAS_SENSOR:break;
		case ZB_DEV_IRC_LEARN_CTRL:
		{
			log_debug("ZB_DEV_IRC_LEARN_CTRL\n");
			
			uint8_t  deviceType = data[0];//产品类型
			uint16_t TableIndex = data[1]<<8|data[2];//型号索引ID
			uint8_t  DeviceKeyLength = data[3];
			uint8_t *DeviceKey = NULL;
			uint8_t key = 0;;
			
			if((deviceType == IRC_DEVICE_TYPE_STB)||(deviceType == IRC_DEVICE_TYPE_IPTV)||(deviceType == IRC_DEVICE_TYPE_TV))
			{
				key = GetDevicePowerKey(deviceType);
				DeviceKey = &key;
			}
			else
			{
				DeviceKey = &data[4];
			}
			
		    hostCmd senddata;

			log_debug_array(data,lenght,NULL);

		    //组包
		    if(Create_Irc_Ctrl_Package(deviceType,TableIndex,DeviceKeyLength,DeviceKey,&senddata)==true)
		    {
		    	log_debug_array(senddata.data,senddata.idx,NULL);
		   	 	//发送
		    	zbSoc_IrcRemoteDevCtrlCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,senddata.idx,senddata.data);
			}
		}
		break;
		case ZB_DEV_ENVIRONMENT_CTRL:break;
		default:
			log_debug("No epInfo->deviceID\n");
		break;
	}
	
	log_debug("Device_ProcessEventByDeviceId--\n");
}

uint8_t Scene_ProcessEvent(uint8_t sceneid)
{
//	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	sceneRecord_t *scene = NULL;
	sceneMembersRecord_t *sceneMembers = NULL;

	log_debug("Scene_ProcessEvent++\n");
	
	scene = sceneListGetSceneByID(sceneid);
	
	if(scene != NULL)
	{
		successive_event_interval = 0;
		
		sceneMembers = scene->members;
		
		while(sceneMembers != NULL)
		{
			//在设备列表中查找设备
			epInfo = vdevListGetDeviceByIeeeEp(sceneMembers->IEEEAddr,sceneMembers->endpoint);

			if(epInfo != NULL)
			{
				//设备此时在线
				if(true == epInfo->onlineflag)//在线
				{
					successive_set_event(epInfo,sceneMembers->dataSegment,sceneMembers->length);
				}
			}
			
			sceneMembers = sceneMembers->next;
		}
		return 0;
	}
	return -1;
}

uint8_t scene_addScene(sceneRecord_t *sceneInfo)
{
	uint8_t sceneId = 0;
	sceneId = sceneListAddScene(sceneInfo);
	SRPC_DevSceneCfm(WIFI_ZIGBEE_ADD_DEV_GROUP_CFM,sceneId,YY_STATUS_SUCCESS);
	return 0;
}

uint8_t scene_ModifyRecordByID(sceneRecord_t *sceneInfo)
{
	bool rtn = false;
	sceneRecord_t *exsistingScene = NULL;
	
	log_debug("scene_ModifyRecordByID++\n");
	
	exsistingScene = sceneListGetSceneByID(sceneInfo->sceneId);

	if(exsistingScene != NULL)
	{
		rtn = sceneListModifyRecordByID(sceneInfo->sceneId,sceneInfo);
		
		log_debug("rtn = %d\n",rtn);

		if(rtn)
		{
			log_debug("YY_STATUS_SUCCESS\n");
			SRPC_DevSceneCfm(WIFI_ZIGBEE_ADD_DEV_GROUP_CFM,sceneInfo->sceneId,YY_STATUS_SUCCESS);
			return 0; 
		}
		else
		{
			log_debug("YY_STATUS_FAIL\n");
			SRPC_DevSceneCfm(WIFI_ZIGBEE_ADD_DEV_GROUP_CFM,sceneInfo->sceneId,YY_STATUS_FAIL);
			return -1;
		}
	}
	else
	{
		SRPC_DevSceneCfm(WIFI_ZIGBEE_ADD_DEV_GROUP_CFM,sceneInfo->sceneId,YY_STATUS_NODE_NO_EXIST);
		return -1;
	}
	return 0;
}

uint8_t	scene_DeleteScene(uint8_t sceneId)
{
	sceneRecord_t *exsistingScene;
	exsistingScene = sceneListGetSceneByID(sceneId);
	
	if(exsistingScene != NULL)
	{
		sceneListRemoveSceneByID(sceneId);
//		SceneList_deteleScene(exsistingScene);
		SRPC_DevSceneCfm(WIFI_ZIGBEE_DEL_DEV_GROUP_CFM,sceneId,YY_STATUS_SUCCESS);

		//删除相关联的事件
		//eventList_DelEventWithSceneID(sceneId);
		vEventList_RemoveEventBySceneID(sceneId);
		timeTaskList_DelTimeTaskWithSceneID(sceneId);
	}
	else
	{
		SRPC_DevSceneCfm(WIFI_ZIGBEE_DEL_DEV_GROUP_CFM,sceneId,YY_STATUS_NODE_NO_EXIST);
	}
}

uint8_t  scene_Controlscene(uint8_t sceneId)
{
	uint8_t ret = -1;
	ret  = Scene_ProcessEvent(sceneId);
	if(!ret)
	{
		SRPC_DevSceneCfm(WIFI_ZIGBEE_CTL_DEV_GROUP_CFM,sceneId,YY_STATUS_SUCCESS);
	}
	else
	{
		SRPC_DevSceneCfm(WIFI_ZIGBEE_CTL_DEV_GROUP_CFM,sceneId,YY_STATUS_NODE_NO_EXIST);	
	}	
	return ret;
}


/*
void Scene_ManagerInit(void)
{
    LIST_INIT(&SceneListHead);
	
	sceneRecord_t *sceneinfo;
	
	uint32_t context = 0;

	while((sceneinfo = sceneListGetNextScene(&context))!=NULL)
	{
		SceneList_addNewScene(sceneinfo);
	}
}
*/

