/**************************************************************************************************
 * Filename:       localTimerHandle.c
 * Author:             zxb      
 * E-Mail:          zxb@yystart.com 
 * Description:    �����ص�Timer ��������������
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
* ������ :Timer_CycleSendHeartPacket(CTimer * timer,void *arg)
* ����   :  ��ʱ��������������
* ����   ��
* ���   ����
* ����   ��
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
    //ÿ�������賿3�����Զ���������
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
* ������ :Timer_TaskInit(void)
* ����   :  ��ʼ����ʱ������
* ����   ��
* ���   ����
* ����   ��
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
* ������ :Timer_updateHeartPacket(void)
* ����   :  ������������ʱ������
* ����   ��
* ���   ����
* ����   ��
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
* ������ :Timer_TaskInit(void)
* ����   :  ��Ӷ�ʱ������
* ����   ��
* ���   ����
* ����   ��
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



