/***********************************************************************************
 * 文 件 名   : event_manager.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : 事件管理
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include "event_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>

#include <errno.h>
#include <string.h>

#include "interface_scenelist.h"
#include "interface_vDeviceList.h"
#include "interface_vEventList.h"
#include "zbSocCmd.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "errorCode.h"
#include "interface_devicelist.h"
#include "interface_eventlist.h"
#include "logUtils.h"
#include "PackageUtils.h"
#include "interface_srpcserver.h"
#include "SuccessiveEvents.h"

int event_checkEvent(ActionEvent_t *event)
{
	log_debug("event_checkEvent++\n");
	epInfo_t *epInfo;
	
	//判断设备是否存在
	epInfo = vdevListGetDeviceByIeeeEp(event->Condition.IEEEAddr,event->Condition.endpoint);
	//查询看下当前节点是否已经注册

	if(epInfo == NULL)
	{
		 log_debug("event_addEvent++1\n");
		 SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,event->Condition.IEEEAddr,event->Condition.endpoint,YY_STATUS_NODE_NO_EXIST);
		 return EVENT_FAILE;
	}
	else if(epInfo->registerflag != true)
	{
		 log_debug("event_addEvent++2\n");
		 SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_UNREGISTER);
		 return EVENT_FAILE;
	}
	else if(epInfo->onlineflag != true)
	{
		 log_debug("event_addEvent++3\n");
		 SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_OUTLINE);
		 return EVENT_FAILE;
	}
	else
	{
		//如果是场景任务，则必须查看场景里面是否有该触发条件，如果有则添加失败
		//避免死循环调用
		if(event->type == EVENT_TYPE_SCENEID_ACTION)
		{
			sceneMembersRecord_t *pMembers = NULL;
			sceneRecord_t *sceneinfo = sceneListGetSceneByID(event->sceneid);
			if(sceneinfo==NULL)
			{
				log_debug("event_addEvent++4\n");
				SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_FAIL);
				return EVENT_FAILE;
			}
			else
			{
				pMembers = sceneinfo->members;
				while(pMembers != NULL)
				{
					if((memcmp(pMembers->IEEEAddr,event->Condition.IEEEAddr,8)==0)&&(pMembers->endpoint==event->Condition.endpoint))
					{
						log_debug("event_addEvent++7\n");
						SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_FAIL);
						return EVENT_FAILE;
					}
					pMembers = pMembers->next;
				}
			}
		}
		else	//添加节点，不能自己触发自己
		{
			Event_t *members = event->members;
			while(members!=NULL)
			{
				if((memcmp(event->Condition.IEEEAddr,members->IEEEAddr,8)==0)&&(event->Condition.endpoint==members->endpoint))
				{
					log_debug("event_addEvent++8\n");
					SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_FAIL);
					return EVENT_FAILE;
				}
				members = members->next;
			}
		}
	}

	log_debug("event_checkEvent--\n");
	return EVENT_SUCCE;
}

int event_addEvent(ActionEvent_t *event)
{
	log_debug("event_addEvent++\n");

	if(!event_checkEvent(event))
	{
		eventListAddDevice(event);
		vEventList_addEvent(event);
		SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,event->Condition.IEEEAddr,event->Condition.endpoint,YY_STATUS_SUCCESS);
	}
		
	log_debug("event_addEvent--\n");
	return EVENT_SUCCE;
}

int event_ModifyRecordByEventID(ActionEvent_t *event)
{
	int ret = EVENT_SUCCE;
	ActionEvent_t *exsistingScene = NULL;
	
	log_debug("event_ModifyRecordByEventID++\n");

	if(!event_checkEvent(event))
	{
//		exsistingScene = eventListGetDeviceByIeeeEpID(event->Condition.IEEEAddr,event->Condition.endpoint,event->ActionEventID);
		exsistingScene = eventListGetDeviceByEventID(event->ActionEventID);
		
		if(exsistingScene != NULL)
		{
			//rtn = eventListModifyRecordByIeeeEpID(event->Condition.IEEEAddr,event->Condition.endpoint,event->ActionEventID,event);	
			ret = eventListModifyRecordByEventID(event->ActionEventID,event);
			if(ret)
			{
				//eventList_modifyEvent(event);
				vEventList_ModifyEvent(event);
				SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,event->Condition.IEEEAddr,event->Condition.endpoint,YY_STATUS_SUCCESS);
				return EVENT_SUCCE; 
			}
			else
			{
				SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,event->Condition.IEEEAddr,event->Condition.endpoint,YY_STATUS_FAIL);
				return EVENT_FAILE;
			}
		}
		else
		{
			eventListAddDevice(event);
			vEventList_addEvent(event);
			SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,event->Condition.IEEEAddr,event->Condition.endpoint,YY_STATUS_SUCCESS);
//			SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,event->ActionEventID,epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_NODE_NO_EXIST);
			return EVENT_SUCCE;
		}
		
	}
		
	return EVENT_FAILE;
}


uint8_t event_DeleteEvent(uint8_t ieeeAddr[8],uint8_t endpoint,uint8_t eventid)
{
	eventList_t *exsistingScene;
	log_debug("event_DeleteEvent++\n");
	do{
	
		exsistingScene = vEventList_GetEventByIeeEpID(ieeeAddr,endpoint,eventid);
		if(exsistingScene == NULL)
		{
			SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_DEL_CFM,eventid,ieeeAddr,endpoint,YY_STATUS_NODE_NO_EXIST);
			break;
		}
		
		//遍历定时器链表查看是否有该任务
		vEventList_RemoveEventByEventID(eventid);
		eventListRemoveDeviceByIeeeEpID(ieeeAddr,endpoint,eventid);

		SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_DEL_CFM,eventid,ieeeAddr,endpoint,YY_STATUS_SUCCESS);

	}while(0);
	log_debug("event_DeleteEvent--\n");
	return 0;
}

//控制事件任务启动或者关闭
uint8_t event_ControlTask(uint8_t ieeeAddr[8],uint8_t endpoint,uint8_t eventId,uint8_t eventState)
{
	bool rtn = false;
	ActionEvent_t *exsistingScene;
	
	if(eventId == 0)
	{
		SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_CTL_REQ,eventId,ieeeAddr,endpoint,YY_STATUS_NODE_NO_EXIST);
		return -1;
	}

	exsistingScene = eventListGetDeviceByIeeeEpID(ieeeAddr,endpoint,eventId);
	if(exsistingScene != NULL)
	{
		exsistingScene->EventState = eventState==ENABLE?ENABLE:DISABLE;

		log_debug("exsistingScene->EventState= %d\n",exsistingScene->EventState);

		rtn =eventListModifyRecordByIeeeEpID(ieeeAddr,endpoint,eventId,exsistingScene);
		if(rtn)
		{
			SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_CTL_REQ,eventId,ieeeAddr,endpoint,YY_STATUS_SUCCESS);
			
			exsistingScene->EventState = eventState==ENABLE?ENABLE:DISABLE;
			//修改链表中的事件
//			eventList_modifyEvent(exsistingScene);
			vEventList_ModifyEvent(exsistingScene);
			log_debug("exsistingScene->EventState= %d\n",exsistingScene->EventState);
			return 0; 
		}
		else
		{
			SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_CTL_REQ,eventId,ieeeAddr,endpoint,YY_STATUS_FAIL);
			return -1;
		}
	}
	else
	{
		SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_CTL_REQ,eventId,ieeeAddr,endpoint,YY_STATUS_NODE_NO_EXIST);
		return -1;
	}
	
	return 0;
}


int event_ProcessNodeEvent(eventList_t *event)
{

	epInfo_t *epInfo;
	Event_t *nodeMembers;
	ActionEvent_t *eventTasks;

	log_debug("timeTaskList_ProcessNodeEvent++\n");

	do{
		eventTasks= eventListGetDeviceByIeeeEpID(event->Condition.IEEEAddr,event->Condition.endpoint,event->evnetId);
		if(eventTasks == NULL || eventTasks->EventState == DISABLE)
		{
			log_debug("Event is not able\n");
			break;
		}
		
		successive_event_interval = 0;
		nodeMembers = eventTasks->members;
		while(nodeMembers != NULL)
		{
			epInfo = vdevListGetDeviceByIeeeEp(nodeMembers->IEEEAddr,nodeMembers->endpoint);
			if(epInfo != NULL)
			{
				if(true == epInfo->onlineflag)//在线
				{
					successive_set_event(epInfo,nodeMembers->dataSegment,nodeMembers->length);
					//Device_ProcessEventByDeviceId(epInfo,sceneMembers->dataSegment,sceneMembers->length);
				}
			}
			nodeMembers = nodeMembers->next;
		}

		return EVENT_SUCCE;
	}while(0);
	
	return EVENT_FAILE;
}


uint8_t event_ProcessActionEvent(epInfo_t *epInfo,uint8_t state)
{
	log_debug("event_ProcessActionEvent++\n");
	
	successive_event_interval = 0;

	eventList_t *event = vEventList_GetEventByIeeeEpSt(epInfo->IEEEAddr,epInfo->endpoint,state);

	if(event == NULL)
		return EVENT_FAILE;
	
	if(event->eventState == DISABLE)
		return EVENT_FAILE;

	if(event->type == EVENT_TYPE_SCENEID_ACTION)
	{
		Scene_ProcessEvent(event->sceneid);
	}
	else
	{
		event_ProcessNodeEvent(event);
	}

	return EVENT_SUCCE;
}
