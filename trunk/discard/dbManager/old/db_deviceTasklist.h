/**************************************************************************************************
 * Filename:       db_deviceTasklist.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    存储待处理的任务，如定时器任务
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-10,13:57)    :   Create the file.
 *
 *************************************************************************/

#ifndef DB_DEVICE_TASKLIST_H
#define DB_DEVICE_TASKLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "Types.h"
#include <stdint.h>
#include "zbSocCmd.h"



typedef struct
{
    uint8_t taskid;
    uint8_t taskType;
    uint8_t alarmType;
    uint8_t alarmDay;
    uint8_t alarmWeek;
    uint8_t alarmHours;
    uint8_t alarmMinute;
    uint8_t alarmSecond;
} devTaskInfo_t;


typedef struct
{
    void *next;
    devTaskInfo_t taskInfo;
} devTaskRecord_t;

typedef struct
{
    uint8 ieeeAddr[8];
    uint8 endpoint;
} devTask_key_IEEE_EP_t;

/*
 * devListAddDevice - create a device and add a rec to the list.
*/

devTaskInfo_t * devTaskListAddTask(devTaskInfo_t *taskInfo,uint8 byteNum,uint8* bytes);


/*
 * devListNumDevices - get the number of devices in the list.
 */
uint32_t devTaskListNumTasks( void );

uint8_t devTaskListGetUnusedTaskId(void);


/*
 * devListInitDatabase - restore device list from file.
 */
void devTaskListInitDatabase( char * dbFilename );

devTaskInfo_t * devTaskListGetNextTask(uint32_t *context,uint8* byteNum,uint8 *bytes);

devTaskInfo_t * devTaskListGetTaskByID(uint8_t taskid,uint8* byteNum,uint8 *bytes);

devTaskInfo_t * devTaskListRemovetaskByID(uint8_t taskid);

devTaskInfo_t * devTaskListRemoveAlltask();


#ifdef __cplusplus
}
#endif

#endif /* DB_DEVICE_TASKLIST_H */
