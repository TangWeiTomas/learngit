/**************************************************************************************************
 * Filename:       localTimerHandle.h
 * Author:             zxb      
 * E-Mail:          zxb@yystart.com 
 * Description:    处理本地的Timer 任务，如心跳包等
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/ 
 * 
 * Version:         1.00  (2014-12-02,16:11)    :   Create the file.
 *                  
 *
 *************************************************************************/
#ifndef _LOCAL_TIMER_HANDLE_H_
#define _LOCAL_TIMER_HANDLE_H_
#include <sys/time.h>
#include "globalVal.h"
#include "devTaskMng.h"
#include "globalDef.h"
#include "timer_manager.h"

#define ONE_SEC_TIME     1000
#define SecTime(n)     ((n)*ONE_SEC_TIME)

#define ONE_MIN_TIME     1000*60
#define MinTime(n)     ((n)*ONE_MIN_TIME)


#define HEART_PACKET_INTERVAL_TIME     SecTime(60)      //心跳包60s 一次


void Timer_TaskInit(void);

void Timer_updateHeartPacket(void);

void Timer_AddDevTask(tm_timer_t *timerID,uint16 intervalSec,uint8 taskid);


#endif//_LOCAL_TIMER_HANDLE_H_

