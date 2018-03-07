#ifndef __ZIGBEE_DEVICE_HEARTBEAT_MANAGER_H__
#define __ZIGBEE_DEVICE_HEARTBEAT_MANAGER_H__
#include "queue.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
//#include "hal_types.h"
//#include "timer_manager.h"
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


//typedef struct ZbSocHeartbeat_t
//{
//    uint8_t ZbSocHeartbeatTaskId;
//    char *name;
//    tm_timer_t *timertask;
//    LIST_ENTRY(ZbSocHeartbeat_t) entry_;
//} ZbSocHeartbeat_t;

//LIST_HEAD(ZbSocHeartbeatList, ZbSocHeartbeat_t) ;

bool zbSoc_Heartbeat_DevicePower_Report_start(void);
bool zbSoc_Heartbeat_DevicePower_Report_stop(void);
void ZbSocHeartbeat_HeartPacketSend(void);
bool zbSoc_Heartbeat_DeviceList_Report_start(void);
bool zbSoc_Heartbeat_DeviceList_Report_stop(void);
bool zbSoc_Heartbeat_DeviceList_Report_refresh(void);
bool zbSoc_Heartbeat_Client_Report_start(timer_handler_cb_t ZbSocHeartbeat_ZbClientReported_Handler,void * timer_handler_arg);
bool zbSoc_Heartbeat_Client_Report_stop(void);
bool zbSoc_Heartbeat_Client_Report_refresh(void);
bool zbSoc_Heartbeat_Connected_Report_start(void);
bool zbSoc_Heartbeat_Connected_Report_stop(void);
bool zbSoc_Heartbeat_Task_evInit(struct event_base *base);
void zbSoc_Heartbeat_relase(void);
bool zbSoc_Heartbeat_Uart_Report_start(void);
bool zbSoc_Heartbeat_Uart_Report_stop(void);
bool zbSoc_Heartbeat_Uart_Report_refresh(void);
int zbSoc_SystemReboot(uint64_t milliseconds);
int zbSoc_WifiRestart(uint64_t milliseconds);



#endif
