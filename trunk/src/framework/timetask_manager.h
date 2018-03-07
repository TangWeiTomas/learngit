#ifndef __TIMER_TASK_H__
#define __TIMER_TASK_H__
#include "interface_timetasklist.h"
#include <event2/util.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "Types.h"
#include "queue.h"
#include "Timer_utils.h"
#include "logUtils.h"


typedef struct TimeTaskListHead_t
{
	timeTaskRecord_t TimeTaskinfo;
	tu_evtimer_t *timertask;
    LIST_ENTRY(TimeTaskListHead_t) entry_;
} TimeTaskListHead_t;

LIST_HEAD(TimeTaskList, TimeTaskListHead_t) ;

uint8_t timerTask_ModifyRecordByID(timeTaskRecord_t *timerTaskInfo);
uint8_t timerTask_AddTask(timeTaskRecord_t *timerTaskInfo);
uint8_t timerTask_DeleteTask(uint8_t timeTaskId);
int timeTaskList_DelTimeTaskWithSceneID(uint8_t sceneid);
bool timeTaskList_evInit(struct event_base *base);


#endif
