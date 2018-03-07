/**************************************************************************************************
 * Filename:       devTaskMng.h
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

#ifndef DEV_TASK_MNG_H
#define DEV_TASK_MNG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hal_types.h"
#include "zbSocCmd.h"
#include "localTimerHandle.h"
#include "queue.h"
#include "globalDef.h"
#include "timer_manager.h"
enum
{
    DEV_TASK_ZIGBEE_ONOFF_TIMER = 0,
    DEV_TASK_UNUSED
};

enum
{
    PERTIME_TYPE_DAY = 0,
    PERTIME_TYPE_WEEK,
    PERTIME_TYPE_MONTH,
    PERTIME_TYPE_INVALID
};

typedef struct timeTaskList
{
    uint8 taskid;
    tm_timer_t *devTaskListTimerID;
    LIST_ENTRY(timeTaskList) entry_;
}timeTaskList;


LIST_HEAD(TListHead, timeTaskList) ;

extern struct TListHead timeTask_head;

extern bool newTimeTaskSettingFlag;

/********************************************************************/

void devTask_InitTask(void);

uint8 devTask_addPeriodTimerSwitchTask(uint8 *ieeeAddr,uint8 endPoint,SPeriodTimers* perTimer,uint8 switchCmd);

bool devTask_deleteTask(uint8 taskId);

void devTask_updateNeedHandleTask(struct tm *time);

void Timer_devTaskHandle(tm_timer_t * timer,void *arg);

void Timer_devTaskDeviceSwitchOnOff(hostCmd *cmd);

#ifdef __cplusplus
}
#endif

#endif /* DEV_TASK_MNG_H */
