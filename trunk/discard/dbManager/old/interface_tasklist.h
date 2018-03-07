/**************************************************************************************************
 * Filename:       interface_devicelist.h
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef INTERFACE_TASKLIST_H
#define INTERFACE_TASKLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include "zbSocCmd.h"
#include "hal_types.h"


typedef struct
{
    uint16_t nwkAddr;
    uint8_t endpoint;
    uint8_t taskid;
    uint8_t alarmMode;
    uint8_t alarmHours;
    uint8_t alarmMinute;
    uint8_t alarmSecond;
    uint8_t taskType;
    uint8_t taskdata1;
    uint8_t taskdata2;
} taskInfo_t;


typedef struct
{
    void *next;
    taskInfo_t taskInfo;
} taskRecord_t;

/*
 * devListAddDevice - create a device and add a rec to the list.
*/

taskInfo_t * taskListAddTask(taskInfo_t *taskInfo,uint16_t dstAddr,uint8_t endpoint,uint8_t alarmMode,
                             uint8_t alarmHours,uint8_t alarmMinute,uint8_t alarmSecond,uint8_t taskType,uint8_t taskdata1,uint8_t taskdata2);



/*
 * devListNumDevices - get the number of devices in the list.
 */
uint32_t taskListNumTasks( void );

uint8_t taskListGetUnusedTaskId(void);


/*
 * devListInitDatabase - restore device list from file.
 */
void taskListInitDatabase( char * dbFilename );

taskInfo_t * taskListGetNextTask(uint32_t *context);

taskInfo_t * taskListGetTaskByID(uint8_t taskid);

taskInfo_t * taskListRemovetaskByID(uint8_t taskid);

taskInfo_t * taskListRemoveAlltask();


#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_TASKLIST_H */
