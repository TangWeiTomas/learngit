/**************************************************************************************************
 * Filename:       localTimerHandle.c
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
#include <stdio.h>
#include <stdlib.h>
#include "timer_manager.h"
#include "localTimerHandle.h"
#include "cmdMsgHandle.h"
#include "devTaskMng.h"


/************************************************************************
* 函数名 :Timer_CycleSendHeartPacket(CTimer * timer,void *arg)
* 描述   :  定时发送心跳包任务
* 输入   ：
* 输出   ：无
* 返回   ：
************************************************************************/
void Timer_CycleSendHeartPacket(tm_timer_t * timer,void *arg)
{
    time_t now;
    struct tm *timenow;

    LOG_PRINT("Timer_StartSendHeartPacket++ \n");
    
    time(&now);
    timenow = localtime(&now);
    LOG_PRINT("year=%d,weekday=%d, mon=%d,monday=%d, hour=%d,min=%d,sec=%d\n",(timenow->tm_year+1900),timenow->tm_wday,(timenow->tm_mon+1),timenow->tm_mday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);

#if 1
    //每周三的凌晨3点整自动重启运行
    if((timenow->tm_wday == 3)&&(timenow->tm_hour == 3)&&(timenow->tm_min == 0)&&(timenow->tm_sec == 0))
    {
        system("reboot -f");
    }
#else
    if((timenow->tm_wday == 5)&&(timenow->tm_hour == 16)&&(timenow->tm_min == 20)&&(timenow->tm_sec == 0))
    {
        system("reboot -f");
    }
#endif
    LOG_PRINT("Timer_CycleSendHeartPacket\n");

    pthread_mutex_lock(&m_db_mutex);
    zigbeeDev_UpdateDevOnlineTimeoutCounter();
    HeartPacketSend();
    pthread_mutex_unlock(&m_db_mutex);

    LOG_PRINT("Timer_StartSendHeartPacket-- \n");

    //return 0;
}

/************************************************************************
* 函数名 :Timer_TaskInit(void)
* 描述   :  初始化定时器任务
* 输入   ：
* 输出   ：无
* 返回   ：
************************************************************************/
void Timer_TaskInit(void)
{
#if 0
    SocketHeartTimerID = malloc(sizeof(CTimer));
    Init_Timer(SocketHeartTimerID,HEART_PACKET_INTERVAL_TIME,Timer_CycleSendHeartPacket,(void *)2,TIMER_CIRCLE);
    start_Timer(SocketHeartTimerID);
#endif    
}

/************************************************************************
* 函数名 :Timer_updateHeartPacket(void)
* 描述   :  更新心跳包定时器任务
* 输入   ：
* 输出   ：无
* 返回   ：
************************************************************************/
void Timer_updateHeartPacket(void)
{
#if 0
    if(SocketHeartTimerID != NULL)
    {
        reset_Timer(SocketHeartTimerID,HEART_PACKET_INTERVAL_TIME);
    }

#endif
}

/************************************************************************
* 函数名 :Timer_TaskInit(void)
* 描述   :  添加定时器任务
* 输入   ：
* 输出   ：无
* 返回   ：
************************************************************************/
void Timer_AddDevTask(tm_timer_t *timerID,uint16 intervalSec,uint8 taskid)
{
#if 0
    uint8 *arg = malloc(sizeof(uint8));
    timerID = malloc(sizeof(CTimer));
    arg[0] = taskid;
    
    LOG_PRINT("Timer_AddDevTask taskid=%d !\n",taskid);
    Init_Timer(timerID,SecTime(intervalSec),Timer_devTaskHandle,(void *)arg,TIMER_ONCE);
    start_Timer(timerID);
#endif	
}



