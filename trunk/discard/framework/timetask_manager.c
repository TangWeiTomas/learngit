/***********************************************************************************
 * �� �� ��   : timetask_manager.c
 * �� �� ��   : Edward
 * ��������   : 2016��7��19��
 * �ļ�����   : ��ʱ�������
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/

#include "timetask_manager.h"
#include <stdint.h>
#include "errorCode.h"
#include "interface_srpcserver.h"
#include "interface_timetasklist.h"
#include "Types.h"
#include <errno.h>
#include <string.h>
#include "interface_scenelist.h"

#include "zbSocCmd.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "errorCode.h"
#include "interface_devicelist.h"
#include "interface_eventlist.h"
#include "logUtils.h"
#include "PackageUtils.h"
#include "SuccessiveEvents.h"
#include "interface_vDeviceList.h"


//��ʱ����ѭ���¼�1h
#define TIMETASKTIMER_CIRCLE_TIME	(60*60*1000)

static struct event_base *taskBase = NULL; 

//�洢��ʱ�¼��б�
static struct TimeTaskList TimeTaskListHead ;

//��ʱ���¶�ʱ�¼�����
static tu_evtimer_t *TimeTaskTimer = NULL;

TimeTaskListHead_t* timeTaskList_isExist(uint8_t timeTaskId);
void timeTaskList_UpdateTimeTask_Handler(void *arg);
static int timeTaskList_AddTimeTask(TimeTaskListHead_t* Task,uint64_t milliseconds,bool continious,timer_handler_cb_t timer_handler_cb,void * timer_handler_arg);
static int timeTaskList_DelTimeTask(timeTaskRecord_t* Task);

uint8_t timerTask_AddTask(timeTaskRecord_t *timerTaskInfo)
{
	uint8_t timerTaskId = 0;
	timerTaskId = timeTaskListAddTask(timerTaskInfo);
	timeTaskList_UpdateTimeTask_Handler(NULL);
	SRPC_PeriodTimerSwitchCtrlCfm(YY_STATUS_SUCCESS,timerTaskId);
	return 0;
}

uint8_t timerTask_ModifyRecordByID(timeTaskRecord_t *timerTaskInfo)
{
	bool rtn = false;
	timeTaskRecord_t *exsistingScene;
	
	exsistingScene = timeTaskListGetTaskByID(timerTaskInfo->timeTaskId);

	if(exsistingScene != NULL)
	{
		rtn = timeTaskListModifyRecordByID(timerTaskInfo->timeTaskId,timerTaskInfo);
		if(rtn == true)
		{
			timeTaskList_DelTimeTask(exsistingScene);
			timeTaskList_UpdateTimeTask_Handler(NULL);
			SRPC_PeriodTimerSwitchCtrlCfm(YY_STATUS_SUCCESS,timerTaskInfo->timeTaskId);
			return 0; 
		}
		else
		{
			SRPC_PeriodTimerSwitchCtrlCfm(YY_STATUS_FAIL,timerTaskInfo->timeTaskId);
			return -1;
		}
	}
	else
	{
		SRPC_PeriodTimerSwitchCtrlCfm(YY_STATUS_NODE_NO_EXIST,timerTaskInfo->timeTaskId);
		return -1;
	}
	return 0;
}

uint8_t timerTask_DeleteTask(uint8_t timeTaskId)
{
	timeTaskRecord_t *exsistingScene;
	exsistingScene = timeTaskListGetTaskByID(timeTaskId);
	if(exsistingScene != NULL)
	{
		timeTaskListRemoveTaskByID(timeTaskId);
		//������ʱ������鿴�Ƿ��и�����
		timeTaskList_DelTimeTask(exsistingScene);
		SRPC_DeletePeriodTimerSwitchCtrlCfm(YY_STATUS_SUCCESS,timeTaskId);
	}
	else
	{
		SRPC_DeletePeriodTimerSwitchCtrlCfm(YY_STATUS_NODE_NO_EXIST,timeTaskId);
		return -1;
	}
	return 0;
}

uint8_t  timerTask_ControlTask(uint8_t timeTaskId,uint8_t timeTaskState)
{
	bool rtn = false;
	timeTaskRecord_t *exsistingScene;
	
	if(timeTaskId == 0)
	{
		SRPC_CtlPeriodTimerSwitchCtrlCfm(YY_STATUS_NODE_NO_EXIST,timeTaskId);
		return -1;
	}

	exsistingScene = timeTaskListGetTaskByID(timeTaskId);

	if(exsistingScene != NULL)
	{
		if(timeTaskState == ENABLE)
		{
			exsistingScene->timeTaskState = ENABLE;
		}
		else
		{
			exsistingScene->timeTaskState = DISABLE;
		}

		rtn = timeTaskListModifyRecordByID(timeTaskId,exsistingScene);
		if(rtn)
		{
			SRPC_CtlPeriodTimerSwitchCtrlCfm(YY_STATUS_SUCCESS,timeTaskId);
			//�����¼�����
			timeTaskList_UpdateTimeTask_Handler(NULL);
			return 0; 
		}
		else
		{
			SRPC_CtlPeriodTimerSwitchCtrlCfm(YY_STATUS_FAIL,timeTaskId);
			return -1;
		}
	}
	else
	{
		SRPC_CtlPeriodTimerSwitchCtrlCfm(YY_STATUS_NODE_NO_EXIST,timeTaskId);
		return -1;
	}
	
	return 0;
}

static int timeTaskList_AddTimeTask(TimeTaskListHead_t* Task,uint64_t milliseconds,bool continious,timer_handler_cb_t timer_handler_cb,void * timer_handler_arg)
{
	log_debug("timeTaskList_AddTimeTask++\n");

    ASSERT(Task != NULL);
        
    Task->timertask = tu_evtimer_new(taskBase);

	if(Task->timertask == NULL)
		return -1;
	
    if(tu_set_evtimer_realtime(Task->timertask,milliseconds,continious,timer_handler_cb,timer_handler_arg)==-1)
    {
    	log_err("tu_set_timer_realtime failed\n");
        return -1;
    }
	
    LIST_INSERT_HEAD(&TimeTaskListHead, Task, entry_);
    
    log_debug("timeTaskList_AddTimeTask--\n");
}

static int timeTaskList_DelTimeTask(timeTaskRecord_t* Task)
{
    TimeTaskListHead_t  *tmp=NULL;

    LIST_FOREACH(tmp,&TimeTaskListHead,entry_)
    {
        if(tmp->TimeTaskinfo.timeTaskId== Task->timeTaskId)
        {
			log_debug("Remove the TimeTaskid=%d\n",Task->timeTaskId);
			tu_kill_evtimer(tmp->timertask);
  			tu_evtimer_free(tmp->timertask);
            LIST_REMOVE(tmp,entry_);
            free(tmp);
            break;
        }
    }
    return 0;
}

int timeTaskList_DelTimeTaskWithSceneID(uint8_t sceneid)
{
    TimeTaskListHead_t  *tmp=NULL;
	uint32_t context = 0;
	timeTaskRecord_t *timeTaskInfo;
	while((timeTaskInfo = timeTaskListGetNextTask(&context))!=NULL)
	{
		if(timeTaskInfo->timeTaskType == EVENT_TYPE_SCENEID_ACTION)
		{
			if(timeTaskInfo->sceneid == sceneid)
			{
				//ɾ������
				timeTaskListRemoveTaskByID(timeTaskInfo->timeTaskId);
			}
		}
	}

	//ɾ����������
	LIST_FOREACH(tmp,&TimeTaskListHead,entry_)
    {
		if(tmp->TimeTaskinfo.timeTaskType == EVENT_TYPE_SCENEID_ACTION)
		{
			if(tmp->TimeTaskinfo.sceneid == sceneid)
			{
				tu_kill_evtimer(tmp->timertask);
				tu_evtimer_free(tmp->timertask);
	            LIST_REMOVE(tmp,entry_);
	            free(tmp);
			}
		}
    }
    return 0;
}

//�ж��������иĶ�ʱ�����Ƿ����
TimeTaskListHead_t* timeTaskList_isExist(uint8_t timeTaskId)
{
	TimeTaskListHead_t *tmp;
	LIST_FOREACH(tmp,&TimeTaskListHead,entry_)
	{
		if(tmp->TimeTaskinfo.timeTaskId == timeTaskId)
		{
			return tmp;
		}
	}
	return NULL;
}

int timeTaskList_ProcessNodeEvent(TimeTaskListHead_t *timeTask)
{

//	vepInfo_t *vepInfo;
	epInfo_t *epInfo = NULL;

	nodeMembersRecord_t *nodeMembers;
	timeTaskRecord_t *timeTasks;

	log_debug("timeTaskList_ProcessNodeEvent++\n");

	timeTasks= timeTaskListGetTaskByID(timeTask->TimeTaskinfo.timeTaskId);
	if(timeTasks != NULL)
	{
		successive_event_interval = 0;
		nodeMembers = timeTasks->members;
		while(nodeMembers != NULL)
		{
			epInfo = vdevListGetDeviceByIeeeEp(nodeMembers->IEEEAddr,nodeMembers->endpoint);
			if(epInfo != NULL)
			{
				if(true == epInfo->onlineflag)//����
				{
					successive_set_event(epInfo,nodeMembers->dataSegment,nodeMembers->length);
					//Device_ProcessEventByDeviceId(epInfo,sceneMembers->dataSegment,sceneMembers->length);
				}
			}
			nodeMembers = nodeMembers->next;
		}
		return 0;
	}
	return -1;
}

//��ʱ����������
void timeTaskList_Process_Handler(TimeTaskListHead_t *timeTask)
{
	log_debug("====timeTaskList_Process_Handler++=======\n");

	if(timeTask == NULL)
		return ;

	switch(timeTask->TimeTaskinfo.timeTaskType)
	{
		case EVENT_TYPE_SCENEID_ACTION://��������
			Scene_ProcessEvent(timeTask->TimeTaskinfo.sceneid);
		break;
		case EVENT_TYPE_NODE_ACTION://�ڵ�����
			timeTaskList_ProcessNodeEvent(timeTask);
		break;
	}
	
	//��������ִ����ɺ�����Ϊ�ر�״̬
	if(timeTask->TimeTaskinfo.timerType == ONCE)
	{
		//���ڽڵ����ݶ�ʧ��������Ҫ���»�ȡȻ���޸ĺ�д��
		timeTaskRecord_t *timeTasks = timeTaskListGetTaskByID(timeTask->TimeTaskinfo.timeTaskId);
		if(timeTasks != NULL)
		{
			timeTasks->timeTaskState = false;
			timeTaskListModifyRecordByID(timeTasks->timeTaskId,timeTasks);
		}
	}
	
	//�Ӷ�ʱ�������б���ɾ������
	timeTaskList_DelTimeTask(&timeTask->TimeTaskinfo);
	
	log_debug("=====timeTaskList_Process_Handler--======\n");
}

//����2Сʱ�ڵ�����
void timeTaskList_UpdateTimeTask_Handler(void *arg)
{
	time_t now;
    struct tm *timenow;

	uint32_t context = 0;

	timeTaskRecord_t *timeTaskInfo;
	TimeTaskListHead_t *timeTask;
	
	time(&now);
	timenow = localtime(&now);

	log_debug("timeTaskList_UpdateTimeTask_Handler++\n");
	log_debug("CurrentTime %d %d %d\n",timenow->tm_hour,timenow->tm_min,timenow->tm_sec);

	//������ʱ�����б�����Сʱ�ڵ�������ӵ�TimeTaskListHead������
	while((timeTaskInfo = timeTaskListGetNextTask(&context)) != NULL)
	{
		if(timeTaskInfo->timeTaskState == true)
		{
			//�ж��Ƿ����
			if(timeTaskList_isExist(timeTaskInfo->timeTaskId)!=NULL)
				continue;
			
			//������Сʱ�ڵ�
			if((timenow->tm_hour==timeTaskInfo->hours))
			{
				uint64_t timeTaskSeconds = timeTaskInfo->minute*60+ timeTaskInfo->second;
				uint64_t timenowSeconds = timenow->tm_min*60+timenow->tm_sec;

				if(timeTaskSeconds > timenowSeconds)
				{
					log_debug("=========================================\n");
					log_debug("RunTime %d %d %d\n",timeTaskInfo->hours,timeTaskInfo->minute,timeTaskInfo->second);
					log_debug("=========================================\n");

					//��timeTaskList_DelTimeTask�б��ͷ�
					timeTask = malloc(sizeof(TimeTaskListHead_t));
					if(timeTask != NULL)
					{
						uint64_t milliseconds = (timeTaskSeconds - timenowSeconds)*1000;//((timeTaskInfo->minute-timenow->tm_min)*60+ timeTaskInfo->second)*1000;
						//memcpy(&timeTask->TimeTaskinfo,timeTaskInfo,sizeof(timeTaskRecord_t));
						
						timeTask->TimeTaskinfo.next = timeTaskInfo->next;
						timeTask->TimeTaskinfo.timeTaskId = timeTaskInfo->timeTaskId;
						timeTask->TimeTaskinfo.timeTaskType = timeTaskInfo->timeTaskType;
						timeTask->TimeTaskinfo.timeTaskState = timeTaskInfo->timeTaskState;
						timeTask->TimeTaskinfo.timerType = timeTaskInfo->timerType;
						timeTask->TimeTaskinfo.hours = timeTaskInfo->hours;
						timeTask->TimeTaskinfo.minute = timeTaskInfo->minute;
						timeTask->TimeTaskinfo.second = timeTaskInfo->second;
						timeTask->TimeTaskinfo.memberscount = timeTaskInfo->memberscount;
						timeTask->TimeTaskinfo.sceneid = timeTaskInfo->sceneid;
						timeTask->TimeTaskinfo.members = NULL;
						
						timeTaskList_AddTimeTask(timeTask,milliseconds,ONCE,(timer_handler_cb_t)timeTaskList_Process_Handler,timeTask);
					}
				}
			}
			else if((timenow->tm_hour+1)==timeTaskInfo->hours)
			{
				uint64_t timeTaskSeconds = 60*60+timeTaskInfo->minute*60+ timeTaskInfo->second;
				uint64_t timenowSeconds = 60*60+timenow->tm_min*60+timenow->tm_sec;

				if(timeTaskSeconds < timenowSeconds)
				{
					timeTask = malloc(sizeof(TimeTaskListHead_t));
					if(timeTask != NULL)
					{
						timenowSeconds = timenow->tm_min*60+timenow->tm_sec;
						uint64_t milliseconds = (timeTaskSeconds - timenowSeconds)*1000;
						//memcpy(&timeTask->TimeTaskinfo,timeTaskInfo,sizeof(timeTaskRecord_t));

						timeTask->TimeTaskinfo.next = timeTaskInfo->next;
						timeTask->TimeTaskinfo.timeTaskId = timeTaskInfo->timeTaskId;
						timeTask->TimeTaskinfo.timeTaskType = timeTaskInfo->timeTaskType;
						timeTask->TimeTaskinfo.timeTaskState = timeTaskInfo->timeTaskState;
						timeTask->TimeTaskinfo.timerType = timeTaskInfo->timerType;
						timeTask->TimeTaskinfo.hours = timeTaskInfo->hours;
						timeTask->TimeTaskinfo.minute = timeTaskInfo->minute;
						timeTask->TimeTaskinfo.second = timeTaskInfo->second;
						timeTask->TimeTaskinfo.memberscount = timeTaskInfo->memberscount;
						timeTask->TimeTaskinfo.sceneid = timeTaskInfo->sceneid;
						timeTask->TimeTaskinfo.members = NULL;
						
						timeTaskList_AddTimeTask(timeTask,milliseconds,ONCE,(timer_handler_cb_t)timeTaskList_Process_Handler,timeTask);
					}
				}
			}
		}
		else //ɾ���б�رյ��¼�
		{
			//ɾ���б��йرյĶ�ʱ������
			if(timeTaskList_isExist(timeTaskInfo->timeTaskId)!=NULL)
			{
				timeTaskList_DelTimeTask(timeTaskInfo);
			}
		}
	}

	//���¶�ʱʱ��
	if(TimeTaskTimer != NULL)
		tu_reset_evtimer(TimeTaskTimer,TIMETASKTIMER_CIRCLE_TIME,CIRCLE);
	
	log_debug("timeTaskList_UpdateTimeTask_Handler--\n");
}

bool timeTaskList_evInit(struct event_base *base)
{  
	log_debug("timeTaskList_ManagerInit++ \n");

	ASSERT(base != NULL);

	//��ʼ����ʱ�����б�
	LIST_INIT(&TimeTaskListHead);

	taskBase = base;

	//��Ӷ�ʱ���񣬶�ʱ���������б�
	TimeTaskTimer = tu_evtimer_new(taskBase);//malloc(sizeof(tm_timer_t));

	if(TimeTaskTimer == NULL)
	{
		log_debug("tu_evtimer_new failed\n");
		return false;
	}
   
	if(tu_set_evtimer_realtime(TimeTaskTimer,TIMETASKTIMER_CIRCLE_TIME,CIRCLE,(timer_handler_cb_t)timeTaskList_UpdateTimeTask_Handler,NULL)==-1)
    {
    	log_err("tu_set_timer_realtime failed\n");
    	tu_evtimer_free(TimeTaskTimer);
    	TimeTaskTimer = NULL;
        return false;
    }
    
	timeTaskList_UpdateTimeTask_Handler(NULL);
	
	log_debug("timeTaskList_ManagerInit-- \n");
	
	return true;	
}

void timeTaskList_ManagerRelase(void)
{
	TimeTaskListHead_t  *tmp=NULL;
	if(TimeTaskTimer!=NULL)
	{
		tu_kill_evtimer(TimeTaskTimer);
		tu_evtimer_free(TimeTaskTimer);
		TimeTaskTimer = NULL;
	}
	
    LIST_FOREACH(tmp,&TimeTaskListHead,entry_)
    {
		log_debug("Remove the TimeTaskid\n");
        tu_kill_evtimer(tmp->timertask);
		tu_evtimer_free(tmp->timertask);
        LIST_REMOVE(tmp,entry_);
        free(tmp);
    }
}
