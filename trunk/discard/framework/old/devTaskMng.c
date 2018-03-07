/**************************************************************************************************
 * Filename:       devTaskMng.c
 * Author:             zxb      
 * E-Mail:          zxb@yystart.com 
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/ 
 * 
 * Version:         1.00  (2014-12-02,21:05)    :   Create the file.
 *                  
 *
 *************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "devTaskMng.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "db_deviceTasklist.h"
#include "interface_devicelist.h"

#define DEV_TASK_INTERVAL_TIME     SecTime(5)      


struct TListHead timeTask_head = LIST_HEAD_INITIALIZER();
static tm_timer_t DevTaskHead;

bool newTimeTaskSettingFlag = false;

/*********************************************************************
 * MACROS
 */



/*********************************************************************
 * CONSTANTS
 */




/************************************************************
 * TYPEDEFS
 */



/*********************************************************************
 * GLOBAL VARIABLES
 */




/*********************************************************************
 * LOCAL VARIABLES
 */


void devTask_TimerTaskHandle(tm_timer_t *DevTask);


/*********************************************************************
 * LOCAL FUNCTIONS
 */
/************************************************************************
* 函数名 :devTask_InitTask(void)
* 描述   :    初始化周期定时器任务
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/

void devTask_InitTask(void)
{
	
	LOG_DEBUG("devTask_InitTask++\n");

#if 0    
    pthread_t timerTask;

    int err=pthread_create(&timerTask,NULL,devTask_TimerTaskHandle,NULL);    //创建线程来帮忙accept

    if(err!=0) 
    {
        LOG_PRINT("[devTask_InitTask]can not create thread : %s\n",strerror(errno));
    }
#endif
	//初始化队列
   LIST_INIT(&timeTask_head);
   memset(&DevTaskHead,0,sizeof(DevTaskHead));
   if(tm_timer_add(&DevTaskHead,DEV_TASK_INTERVAL_TIME,CIRCLE,(timer_handler_cb_t)devTask_TimerTaskHandle,&DevTaskHead)==false)
   {
		LOG_PRINT("Create devTask_InitTask failed\n");
		return;
   } 
}

/************************************************************************
* 函数名 :devTask_InitTask(void)
* 描述   :    初始化周期定时器任务
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void devTask_TimerTaskHandle(tm_timer_t *DevTask)
{
    time_t now;
    struct tm *timenow;
    static int lasthour = -1;

#if 0
    //初始化队列
    LIST_INIT(&timeTask_head);
     
    while(1)
    {
        sleep(1);
        
        time(&now);
        timenow = localtime(&now);
        //LOG_PRINT("year=%d,weekday=%d, mon=%d,monday=%d, hour=%d,min=%d,sec=%d\n",(timenow->tm_year+1900),timenow->tm_wday,(timenow->tm_mon+1),timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);

        //每隔一个小时，更新一下下一个小时内需要处理的定时任务
        if(timenow->tm_hour != lasthour)
        {
            LOG_PRINT("update the next hour timer task!\n");
            lasthour = timenow->tm_hour;
            devTask_updateNeedHandleTask(timenow);
        }

        if(newTimeTaskSettingFlag == true)
        {
            devTask_updateNeedHandleTask(timenow);            
            newTimeTaskSettingFlag = false;
        }
        
    }
#endif
	//判断是否被停止，如果停止则重新启动
	if(DevTask->timing_task_timer.in_use == false)
	{
		tm_timer_remove(DevTask);
		tm_timer_add(&DevTaskHead,DEV_TASK_INTERVAL_TIME,CIRCLE,(timer_handler_cb_t)devTask_TimerTaskHandle,&DevTaskHead);
	}

	time(&now);
    timenow = localtime(&now);
	if(timenow->tm_hour != lasthour)
    {
        LOG_PRINT("update the next hour timer task!\n");
        lasthour = timenow->tm_hour;
        devTask_updateNeedHandleTask(timenow);
    }

    if(newTimeTaskSettingFlag == true)
    {
        devTask_updateNeedHandleTask(timenow);            
        newTimeTaskSettingFlag = false;
    }

}

/************************************************************************
* 函数名 :devTask_updateNeedHandleTask(uint8 taskId)
* 描述   :    删除周期定时器开关任务
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void devTask_updateNeedHandleTask(struct tm *time)
{
    uint32_t context = 0;
#if 0
    devTaskInfo_t * devTaskInfo;

#ifdef DEBUG
    LOG_PRINT("FuncName:%s \n",__FUNCTION__);
#endif

    while((devTaskInfo=devTaskListGetNextTask(&context,NULL,NULL)) != NULL)
    {
        if((devTaskInfo->alarmMinute*60+devTaskInfo->alarmSecond)<(time->tm_min*60+time->tm_sec))
        {
            LOG_PRINT("Taskid %0d Task don't need to handle.\n",devTaskInfo->taskid);
            continue;
        }
        
        switch(devTaskInfo->alarmType)
        {
            case PERTIME_TYPE_MONTH:
                if((devTaskInfo->alarmDay == time->tm_mday)
                    &&(devTaskInfo->alarmHours == time->tm_hour))
                {
                    timeTaskList* timeTask = malloc(sizeof(timeTaskList));
                    uint16 intervalSec;          
                    intervalSec = ((uint16)(devTaskInfo->alarmMinute- time->tm_min)*60+devTaskInfo->alarmSecond- time->tm_sec );
                    Timer_AddDevTask(timeTask->devTaskListTimerID,intervalSec,devTaskInfo->taskid);

                    timeTask->taskid = devTaskInfo->taskid;
                    LIST_INSERT_HEAD(&timeTask_head, timeTask, entry_);
                }
                break;
            case PERTIME_TYPE_WEEK:
                if((devTaskInfo->alarmWeek == time->tm_wday)
                    &&(devTaskInfo->alarmHours == time->tm_hour))
                {
                    timeTaskList* timeTask = malloc(sizeof(timeTaskList));
                    uint16 intervalSec;          
                    intervalSec = ((uint16)(devTaskInfo->alarmMinute- time->tm_min)*60+devTaskInfo->alarmSecond- time->tm_sec );
                    Timer_AddDevTask(timeTask->devTaskListTimerID,intervalSec,devTaskInfo->taskid);

                    timeTask->taskid = devTaskInfo->taskid;
                    LIST_INSERT_HEAD(&timeTask_head, timeTask, entry_);
                }
                break;
            case PERTIME_TYPE_DAY:
                if(devTaskInfo->alarmHours == time->tm_hour)
                {
                    timeTaskList* timeTask = malloc(sizeof(timeTaskList));
                    uint16 intervalSec;          
                    intervalSec = ((uint16)(devTaskInfo->alarmMinute- time->tm_min)*60+devTaskInfo->alarmSecond- time->tm_sec );
                    Timer_AddDevTask(timeTask->devTaskListTimerID,intervalSec,devTaskInfo->taskid);

                    timeTask->taskid = devTaskInfo->taskid;
                    LIST_INSERT_HEAD(&timeTask_head, timeTask, entry_);
                }
                break;
            default:
                break;
        }

        
    }
#endif
}


/************************************************************************
* 函数名 :devTask_deleteTask(uint8 taskId)
* 描述   :    删除周期定时器开关任务
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
bool devTask_deleteTask(uint8 taskId)
{
    devTaskInfo_t * taskInfo;

	LOG_DEBUG("devTask_deleteTask++\n");

    taskInfo = devTaskListRemovetaskByID(taskId);

    if(taskInfo != NULL)
    {
        return true;
    }

    return false;
}

/************************************************************************
* 函数名 :devTask_addPeriodTimerSwitchTask(uint8 *ieeeAddr,uint8 endPoint,SPeriodTimers* perTimer,uint8 switchCmd)
* 描述   :    添加周期定时器开关任务
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
uint8 devTask_addPeriodTimerSwitchTask(uint8 *ieeeAddr,uint8 endPoint,SPeriodTimers* perTimer,uint8 switchCmd)
{
    devTaskInfo_t taskInfo;
    hostCmd cmd;
    cmd.idx=0;

    cmdSetStringVal(&cmd,ieeeAddr,8);
    cmdSet8bitVal(&cmd,endPoint);
    cmdSet8bitVal(&cmd, switchCmd);

	LOG_DEBUG("devTask_addPeriodTimerSwitchTask++\n");

    taskInfo.taskid = devTaskListGetUnusedTaskId();
    taskInfo.taskType = DEV_TASK_ZIGBEE_ONOFF_TIMER;
    taskInfo.alarmType = perTimer->periodType;
    taskInfo.alarmDay = perTimer->day;
    taskInfo.alarmWeek = perTimer->week;
    taskInfo.alarmHours = perTimer->hour;
    taskInfo.alarmMinute = perTimer->min;
    taskInfo.alarmSecond = perTimer->sec;

    devTaskListAddTask(&taskInfo,cmd.idx,cmd.data);

    newTimeTaskSettingFlag = true;

    return taskInfo.taskid;
}


/************************************************************************
* 函数名 :Timer_devTaskHandle(CTimer * timer,void *arg)
* 描述   :  定时发送心跳包任务
* 输入   ：
* 输出   ：无
* 返回   ：
************************************************************************/
void Timer_devTaskHandle(tm_timer_t* timer,void *arg)
{
#if 0
    devTaskInfo_t *devTaskInfo;
    timeTaskList  *tmp=NULL;
    uint8 taskid = *((uint8*)arg);
    hostCmd cmd;
    cmd.idx=0;

    LOG_PRINT("Timer_devTaskHandle++ ,taskid=%d\n",taskid);
    
    devTaskInfo = devTaskListGetTaskByID(taskid,(uint8*)(&(cmd.size)),cmd.data);

    LIST_FOREACH(tmp,&timeTask_head,entry_)
    {
        if((tmp->taskid == taskid)&&(tmp->devTaskListTimerID == timer))
        {
            LIST_REMOVE(tmp,entry_);
            free(tmp);
            LOG_PRINT("Remove the Taskid=%d\n",taskid);
            break;
        }
    }

    switch(devTaskInfo->taskType)
    {
        case DEV_TASK_ZIGBEE_ONOFF_TIMER:
            Timer_devTaskDeviceSwitchOnOff(&cmd);
            break;
        default:
            break;
    }
    
    free(timer);
    free(arg);

    LOG_PRINT("Timer_devTaskHandle-- \n");
#endif
 //   return 0;
}


/************************************************************************
* 函数名 :Timer_devTaskDeviceSwitchOnOff(hostCmd *cmd)
* 描述   :  定时控制开关节点
* 输入   ：
* 输出   ：无
* 返回   ：
************************************************************************/
void Timer_devTaskDeviceSwitchOnOff(hostCmd *cmd)
{
    uint8 ieeeAddr[8];
    uint8 endPoint,switchCmd;
    epInfo_t *epInfo;
    uint8 i;
    LOG_DEBUG("Timer_devTaskDeviceSwitchOnOff++\n");
    
    cmdGetStringVal(cmd, ieeeAddr,8);//节点IEEE地址
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &switchCmd);

    //判断该IEEE地址设备是否存在
    LOG_PRINT("Timer_devTaskDeviceSwitchOnOff Success.endpoint=%02x,switchCmd=%02x\n",endPoint,switchCmd);

    LOG_PRINT("IEEE Addr: ");
    for(i=0;i<8;i++)
    {
        LOG_PRINT("%02x",ieeeAddr[i]);
        if(i<7)LOG_PRINT(":");
    }
    LOG_PRINT("\n");

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

    if (epInfo != NULL)
    {
        LOG_PRINT("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
        zbSoc_SetGenOnOffState(switchCmd,epInfo->nwkAddr,endPoint,afAddr16Bit);        
    }

}



