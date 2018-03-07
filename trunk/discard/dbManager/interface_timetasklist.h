#ifndef __TIME_TASK_LIST_H__
#define __TIME_TASK_LIST_H__



#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include "zbSocCmd.h"
#include "logUtils.h"

#define NEW_TIMER_TASK		0X00

typedef struct nodeMembersRecord_s
{
	struct nodeMembersRecord_s *next;
	uint8_t IEEEAddr[8];
	uint8_t endpoint;			   		// Endpoint identifier
	uint8_t length;						//最大8字节
	uint8_t dataSegment[DATASEGMENT];
} nodeMembersRecord_t;

typedef struct timeTaskRecord_s
{
    void *next;				
    uint8_t timeTaskId;		//定时任务ID
	uint8_t timeTaskType;   //0:场景任务1:节点任务
	uint8_t timeTaskState;	//定时器是否生效，0:关闭 1:开启

	//定时任务时间参数
	uint8_t timerType;		//0:单次  1:循环
	uint8_t hours;			//小时
	uint8_t minute;			//分钟
    uint8_t second;			//秒
	
	//定时任务被执行的成员
	uint8_t memberscount;	//触发事件个数
	//场景任务执行SCENDID中的场景
	uint8_t sceneid;		//场景任务ID
	//设备任务执行members中的成员
	nodeMembersRecord_t *members;//成员
} timeTaskRecord_t;


/*
 * timeTaskListAddTask - create a timeTask and add a rec fto the list.
 */
//uint8_t timeTaskListAddTask(char *timeTaskNameStr, uint16_t groupId);

uint8_t timeTaskListAddTask( timeTaskRecord_t *Task);
timeTaskRecord_t * timeTaskListRemoveALLTask();
timeTaskRecord_t * timeTaskListRemoveTaskByID(uint8_t timeTaskId);


/*
 * timeTaskListAddTask - gets the scen id of a a timeTask
 */

uint8_t timeTaskListGetUnusedTaskId(void);
uint8_t timeTaskListGetTaskNum();

/*
 * timeTaskListGetNextTask - Return the next timeTask in the list.
 */
timeTaskRecord_t* timeTaskListGetNextTask(uint32_t *context);

/*
 * groupListInitDatabase - Restore Task List from file.
 */
timeTaskRecord_t * timeTaskListGetTaskByID(uint8_t key);
void timeTaskListInitDatabase(char * dbFilename);
uint16_t timeTaskListGetAllTask(void);

#ifdef __cplusplus
}
#endif

#endif
