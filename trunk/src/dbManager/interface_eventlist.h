/**************************************************************************************************
 * Filename:       interface_eventlist.h
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

#ifndef INTERFACE_EVENTLIST_H
#define INTERFACE_EVENTLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "Types.h"
#include "zbSocCmd.h"

#define NEW_TIMER_EVENT		0X00

typedef struct Event_s{
	struct Event_s *next;
	uint8_t IEEEAddr[8];
	uint8_t endpoint;			    // Endpoint identifier
	uint8_t length;					//最大8字节
	uint8_t dataSegment[DATASEGMENT];
}Event_t;

typedef struct {
	uint8_t ActionEventID;
	uint8_t EventState;				//事件使能状态
	uint8_t type;					//0: 场景1:节点任务
	uint8_t membersCount;			//设备数量
	uint8_t sceneid;				//场景ID
	Event_t Condition; 				//条件
	Event_t *members;				//目标的动作
}ActionEvent_t;


/*
 * devListAddDevice - create a device and add a rec to the list.
 */
uint8_t eventListAddDevice( ActionEvent_t *EventInfo);

/*
 * eventListRemoveDeviceByNaEp - remove a device rec from the list.
 */
ActionEvent_t * eventListRemoveDeviceByNaEp( uint16_t nwkAddr, uint8_t endpoint );

ActionEvent_t * eventListRemoveDeviceByIeeeEp(uint8_t ieeeAddr[],uint8_t endpoint);
ActionEvent_t * eventListRemoveDeviceByIeeeEpID(uint8_t ieeeAddr[],uint8_t endpoint,uint8_t eventid);

ActionEvent_t * eventListRemoveAllDevice(void);


bool eventListModifyRecordByNaEp(uint16_t nwkAddr, uint8_t endpoint,ActionEvent_t * epInfo);

bool eventListModifyRecordByIeeeEp(uint8_t* ieeeAddr, uint8_t endpoint,ActionEvent_t * epInfo);
bool eventListModifyRecordByIeeeEpID(uint8_t* ieeeAddr, uint8_t endpoint,uint8_t eventid,ActionEvent_t * eventInfo);

/*
 * eventListNumDevices - get the number of devices in the list.
 */
uint32_t eventListNumDevices( void );

/*
 * eventListInitDatabase - restore device list from file.
 */
void eventListInitDatabase( char * dbFilename );

ActionEvent_t * eventListGetNextDev(uint32_t *context);

ActionEvent_t * eventListGetDeviceByIeeeEp( uint8_t ieeeAddr[], uint8_t endpoint );

ActionEvent_t * eventListGetDeviceByNaEp( uint16_t nwkAddr, uint8_t endpoint );

ActionEvent_t * eventListRemoveDeviceByIeee( uint8_t ieeeAddr[] );

ActionEvent_t * eventListGetDeviceByIeeeEpID(uint8_t ieeeAddr[], uint8_t endpoint,uint8_t eventid);
uint16_t eventListGetAllDevice(uint8_t *devNum,uint8_t *pBuf,uint16_t bufsize);
ActionEvent_t * eventListGetDeviceByEventID(uint8_t eventid);
bool eventListModifyRecordByEventID(uint8_t eventid,ActionEvent_t * eventInfo);


#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_DEVICELIST_H */
