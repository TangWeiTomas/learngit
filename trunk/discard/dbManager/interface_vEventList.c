/***********************************************************************************
 * 文 件 名   : interface_vEventLsit.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年11月25日
 * 文件描述   : 管理event事件
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "interface_vEventList.h"
#include "logUtils.h"
LIST_HEAD(eventList, eventList_t) ;

static struct eventList vEventListHead ;


static int vEventList_UpdataEvent(eventList_t *event,ActionEvent_t *eventinfo)
{
	event->evnetId = eventinfo->ActionEventID;
	event->eventState = eventinfo->EventState;
	event->type = eventinfo->type;
	event->Condition.endpoint = eventinfo->Condition.endpoint;
	memcpy(event->Condition.IEEEAddr,eventinfo->Condition.IEEEAddr,8);
	event->Condition.active = eventinfo->Condition.dataSegment[0];
	if(eventinfo->type == 0x00)
		event->sceneid = eventinfo->sceneid;
	else
		event->sceneid = 0;
}

static eventList_t* vEventList_GetEventByEventID(uint8_t eventid)
{
	eventList_t  *event=NULL;

	log_debug("vEventList_GetEventByEventID++\n");

    LIST_FOREACH(event,&vEventListHead,entry_)
    {
		
        if(event->evnetId == eventid)
        {
        	log_debug("Found EventID %d\n",event->evnetId);
			return event;
        }
    }

    log_debug("Not Found EventID %d\n",eventid);
	log_debug("vEventList_GetEventByEventID--\n");
	
    return NULL;
}

static eventList_t* vEventList_GetEventBySceneID(uint8_t sceneid)
{
	eventList_t  *event=NULL;

	log_debug("vEventList_GetEventBySceneID++\n");

    LIST_FOREACH(event,&vEventListHead,entry_)
    {	
        if((event->type == 0x00) && (event->sceneid == sceneid))
        {
        	log_debug("Found SceneID %d\n",event->sceneid);
			return event;
        }
    }

    log_debug("Not Found SceneID %d\n",sceneid);
	log_debug("vEventList_GetEventBySceneID--\n");
	
    return NULL;
}

eventList_t* vEventList_GetEventByIeeEpID(uint8_t ieee[8],uint8_t endpoint,uint8_t eventid)
{
	eventList_t  *event=NULL;

	log_debug("vEventList_GetEventByIeeEpID++\n");

    LIST_FOREACH(event,&vEventListHead,entry_)
    {	
       if((memcmp(ieee,event->Condition.IEEEAddr,8) == 0)&&(event->Condition.endpoint == endpoint) && (event->evnetId== eventid))
		{
			log_debug("Found Event %d\n",event->evnetId);
			return event;
		}
    }

    log_debug("Not Found Event %d\n",eventid);
	log_debug("vEventList_GetEventByIeeEpID--\n");
	
    return NULL;
}

eventList_t *vEventList_GetEventByIeeeEpSt(uint8_t ieee[8],uint8_t endpoint,uint8_t state)
{
	eventList_t  *event=NULL;

	log_debug("vEventList_GetEventByIeeeEpSt++\n");

    LIST_FOREACH(event,&vEventListHead,entry_)
    {	
		if((memcmp(ieee,event->Condition.IEEEAddr,8) == 0)&&(event->Condition.endpoint == endpoint) && (event->Condition.active == state))
		{
			log_debug("Found Event %d\n",event->evnetId);
			return event;
		}
    }

    log_debug("Not Found Event \n");
	log_debug("vEventList_GetEventByIeeeEpSt--\n");
	
    return NULL;
}


int vEventList_ModifyEvent(ActionEvent_t *eventinfo)
{
	eventList_t  *event=NULL;
	do
	{
		event = vEventList_GetEventByEventID(eventinfo->ActionEventID);
		if(event == NULL)
		{
			log_err("Not Fount Event %d\n",eventinfo->ActionEventID);
			break;
		}

		vEventList_UpdataEvent(event,eventinfo);
		return EVENT_SUCCE;
		
	}while(0);
	
    return EVENT_FAILE;
}

int vEventList_RemoveEventByEventID(uint8_t eventid)
{
	eventList_t *event = NULL;
		
	log_debug("vEventList_RemoveEventByEventID++\n");

	do{
		event = vEventList_GetEventByEventID(eventid);
		if(event ==NULL)
			break;
		LIST_REMOVE(event,entry_);
		free(event);
		return EVENT_SUCCE;
		
	}while(0);

	log_debug("vEventList_RemoveEventByEventID--\n");

	return EVENT_FAILE;
}

int vEventList_RemoveEventBySceneID(uint8_t sceneid)
{
	eventList_t *event = NULL;
		
	log_debug("vEventList_RemoveEventByEventID++\n");

	do{
		event = vEventList_GetEventBySceneID(sceneid);
		if(event ==NULL)
			break;
		LIST_REMOVE(event,entry_);
		free(event);
		return EVENT_SUCCE;
		
	}while(0);

	log_debug("vEventList_RemoveEventByEventID--\n");

	return EVENT_FAILE;
}


int vEventList_addEvent(ActionEvent_t *eventinfo)
{
	int ret = EVENT_SUCCE;
	eventList_t *event = NULL;
	
	ASSERT(eventinfo != NULL);

	if(vEventList_GetEventByEventID(eventinfo->ActionEventID)== NULL)
	{
		event = malloc(sizeof(eventList_t));
		if(event == NULL)
			return EVENT_FAILE;

		vEventList_UpdataEvent(event,eventinfo);
		LIST_INSERT_HEAD(&vEventListHead,event,entry_);
	}
	else
	{
		ret = vEventList_ModifyEvent(eventinfo);
	}
	
	return ret;
}


int vEventList_Init(void)
{
	ActionEvent_t *eventinfo;

	uint32_t context = 0;

	log_debug("vEventList_Init++\n");

	LIST_INIT(&vEventListHead);
	
	//将事件放入列表中，不带被触发对象
	while((eventinfo = eventListGetNextDev(&context))!=NULL)
	{
		vEventList_addEvent(eventinfo);
	}

	log_debug("vEventList_Init--\n");

	return EVENT_SUCCE;
}


int vEventList_Distory(void)
{
	eventList_t  *event=NULL;

	log_debug("vEventList_GetEventBySceneID++\n");

    LIST_FOREACH(event,&vEventListHead,entry_)
    {	
		LIST_REMOVE(event,entry_);
		free(event);
    }
    
    return EVENT_SUCCE;
}
