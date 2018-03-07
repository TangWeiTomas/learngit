/**************************************************************************************************
 * Filename:       Zigbee_device_Heartbeat_Manager.c
 * Author:             edward
 * E-Mail:          ouxiangping@feixuekj.com
 * Description:    ע��͹���zigbee�豸����������
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-02,16:11)    :   Create the file.
 *
 *
 *************************************************************************/
#include "Zigbee_device_Heartbeat_Manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include "Types.h"
//#include "hal_types.h"
#include "zbSocCmd.h"
#include "queue.h"
#include "globalVal.h"
#include "zbSocPrivate.h"
//#include "timer_manager.h"
#include "interface_srpcserver.h"
#include "interface_devicelist.h"
#include "interface_vDeviceList.h"
#include "mt_zbSocCmd.h"

#include <event2/event.h>

extern uart_event_t g_uart_event;

//�豸������ʱ�ϱ�����
static tu_evtimer_t *zbSoc_DeviceList_Report_Handle = NULL;
//���������������ӵ�������ʱ��
static tu_evtimer_t *zbSoc_Heartbeat_Client_Handle = NULL;
//�����ϱ���ʱ����
static tu_evtimer_t *zbSoc_DevicePower_Report_Handle = NULL;
//�������Ϸ������󣬶�ʱ5����������ע����Ϣ
static tu_evtimer_t *zbSoc_Connected_Report_Handle = NULL;
//����������
static tu_evtimer_t *zbSoc_Uart_Report_Handle = NULL;
#if PERMIMNG
//������Ȩ����������ʱȥ����
static tu_evtimer_t *zbSoc_Permimng_Report_Handle = NULL;
#endif

/************************************************************************
* ������ :ZbSocHeartbeat_HeartPacketSend(void)
* ����   :   ����������
* ����   ��
* ���   ����
* ����   ��0:����ɹ�
************************************************************************/
void ZbSocHeartbeat_HeartPacketSend(void)
{
    uint8_t allDevNum;
    uint8_t buf[MaxPacketLength];
    uint16_t bufLen;

    log_debug("ZbSocHeartbeat_HeartPacketSend++ \n");

    //devMgr_getAllRegisterOnlineDevices(&allDevNum,buf,&bufLen);
    allDevNum = vdevListGetAllRegOnlineDev(buf,&bufLen);
    log_debug("allDevNum=%d bufLen = %d \n",allDevNum,bufLen);
    
    SRPC_heartPacketInd(allDevNum,buf,bufLen);
    log_debug("ZbSocHeartbeat_HeartPacketSend-- \n");
    return;
}

void zbSoc_Heartbeat_DeviceList_Report_cb(void *args)
{
	time_t now;
    struct tm *timenow;
    
	log_debug("zbSoc_Heartbeat_DeviceList_Report_cb++\n");

    time(&now);
    timenow = localtime(&now);
    log_debug("%d-%d-%d (%d) %d:%d:%d\n",(timenow->tm_year+1900),(timenow->tm_mon+1),\
    timenow->tm_mday,timenow->tm_wday,timenow->tm_hour,timenow->tm_min,timenow->tm_sec);

    //ÿ�������賿3�����Զ���������
    if((timenow->tm_wday == 3)&&(timenow->tm_hour == 3)&&(timenow->tm_min == 0)&&(timenow->tm_sec == 0))
    {
		//����Э����
		mt_Sys_Reset_sreq();
		//reboot system
        system("reboot -f");
    }
    
	SRPC_RoomfairyRegistered();
    vdevListSetTimeOutCnt();
	ZbSocHeartbeat_HeartPacketSend();
	
  	log_debug("zbSoc_Heartbeat_DeviceList_Report_cb--\n");
}

bool zbSoc_Heartbeat_DeviceList_Report_start(void)
{
	int ret = -1;
	
	if(zbSoc_DeviceList_Report_Handle == NULL)
		return false;
	
	ret =tu_set_evtimer_realtime(zbSoc_DeviceList_Report_Handle, HEART_PACKET_INTERVAL_TIME,CIRCLE,zbSoc_Heartbeat_DeviceList_Report_cb,zbSoc_DeviceList_Report_Handle);

	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		return false;
	}

	return true;
}

bool zbSoc_Heartbeat_DeviceList_Report_stop(void)
{
	if(zbSoc_DeviceList_Report_Handle == NULL)
		return false;
	
	tu_kill_evtimer(zbSoc_DeviceList_Report_Handle);

	return true;
}

bool zbSoc_Heartbeat_DeviceList_Report_refresh(void)
{
	log_debug("zbSoc_Heartbeat_DeviceList_Report_refresh++\n");
	if(zbSoc_DeviceList_Report_Handle == NULL)
		return false;
	
	tu_reset_evtimer(zbSoc_DeviceList_Report_Handle,HEART_PACKET_INTERVAL_TIME,CIRCLE);
	log_debug("zbSoc_Heartbeat_DeviceList_Report_refresh--\n");

	return true;

}

bool zbSoc_Heartbeat_Client_Report_start(timer_handler_cb_t ZbSocHeartbeat_ZbClientReported_Handler,void * timer_handler_arg)
{
	int ret = -1;
	
	if(zbSoc_Heartbeat_Client_Handle == NULL)
		return false;
	
	ret =tu_set_evtimer_realtime(zbSoc_Heartbeat_Client_Handle, CLIENT_HEART_PACKET_INTERVAL_TIME,ONCE,ZbSocHeartbeat_ZbClientReported_Handler,timer_handler_arg);

	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		return false;
	}
	
	return true;
}

bool zbSoc_Heartbeat_Client_Report_stop(void)
{

	if(zbSoc_Heartbeat_Client_Handle == NULL)
		return false;
	
	tu_kill_evtimer(zbSoc_Heartbeat_Client_Handle);
	return true;
}

bool zbSoc_Heartbeat_Client_Report_refresh(void)
{
	
	if(zbSoc_Heartbeat_Client_Handle == NULL)
		return false;
	tu_reset_evtimer(zbSoc_Heartbeat_Client_Handle,CLIENT_HEART_PACKET_INTERVAL_TIME,ONCE);

	return true;
}

void zbSoc_Heartbeat_DevicePower_Report_cb(void *args)
{

	//Context��������Ϊ0 ��Ȼ��������ݿ��ļ�����
	uint32_t context = 0;

	epInfo_t *epInfo = NULL;
//	vepInfo_t *virEpInfo = NULL;
	epInfo_t *vepInfo = NULL;
	log_debug("===========ZbSocHeartbeat_ZbDevicePowerReported_Handler==========\n");
	
//	if(mzbDeviceReported == NULL)
//		return ;

	while((epInfo = devListGetNextDev(&context))  !=NULL)
	{
		switch(epInfo->deviceID)
		{
			case ZB_DEV_ONOFF_DOORLOCK:
			{
				vepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
				if((vepInfo!=NULL) && (vepInfo->onlineflag == true))
				{
					uint8_t value = vdevListGetDoorBattery(vepInfo);
					if(value!=0)
						SRPC_DoorLockPowerValueInd(vepInfo->IEEEAddr,vepInfo->endpoint,value,vepInfo->onlineDevRssi);
				}
			}
			break;
			case ZB_DEV_POWER_SWITCH:
			{
				vepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
				if((vepInfo!=NULL) && (vepInfo->onlineflag == true))
				{
					uint32_t mPowerValue = vdevListGetPowerSwitchBattery(vepInfo);
					if(mPowerValue!=0)
						SRPC_QueryPowerSwitchValueInd(vepInfo->IEEEAddr,vepInfo->endpoint,mPowerValue,vepInfo->onlineDevRssi);
				}
			}
			break;
			case ZB_DEV_DOOR_SENSOR:
			{
				vepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
				if((vepInfo!=NULL) && (vepInfo->onlineflag == true))
				{
					uint8_t mPowerValue = vdevListGetAlarmBattery(vepInfo);
					if(mPowerValue!=0)
						SRPC_ComAlarmPowerValueInd(vepInfo->IEEEAddr,vepInfo->endpoint,vepInfo->deviceID,mPowerValue);
				}
			}
			break;
			case ZB_DEV_INFRARED_BODY_SENSOR:
			{
				vepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
				if((vepInfo!=NULL) && (vepInfo->onlineflag == true))
				{
					uint8_t mPowerValue = vdevListGetAlarmBattery(vepInfo);
					if(mPowerValue!=0)
						SRPC_ComAlarmPowerValueInd(vepInfo->IEEEAddr,vepInfo->endpoint,vepInfo->deviceID,mPowerValue);
				}
			}
			break;
			default:break;
		}
	}
	log_debug("ZbSocHeartbeat_ZbDevicePowerReported_Handler++\n");
}

bool zbSoc_Heartbeat_DevicePower_Report_start(void)
{
	int ret = -1;
	
	if(zbSoc_DevicePower_Report_Handle == NULL)
		return false;
	
	ret =tu_set_evtimer(zbSoc_DevicePower_Report_Handle, DEVICE_POWERVALUE_TIME,CIRCLE,zbSoc_Heartbeat_DevicePower_Report_cb,zbSoc_DevicePower_Report_Handle);

	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		return false;
	}

	return true;
}

bool zbSoc_Heartbeat_DevicePower_Report_stop(void)
{
	
	if(zbSoc_DevicePower_Report_Handle == NULL)
		return false;
	
	tu_kill_evtimer(zbSoc_DevicePower_Report_Handle);

	return true;
}

static void
zbSoc_Heartbeat_Connected_Report_cb(void *args)
{
	log_debug("zbSoc_Heartbeat_Connected_Report_cb\n");		
		//�ϱ�ע����Ϣ
		SRPC_RoomfairyRegistered();
		sleep(1);
		//�ϱ�������Ϣ
		//ZbSocHeartbeat_HeartPacketSend();
		//sleep(1);
#if POWER_REPORT_ENBALE
		//�ϱ�������Ϣ
		zbSoc_Heartbeat_DevicePower_Report_cb(NULL);
#endif 

}

bool zbSoc_Heartbeat_Connected_Report_start(void)
{
	int ret = -1;
	
	if(zbSoc_Connected_Report_Handle == NULL)
		return false;
	
	ret =tu_set_evtimer(zbSoc_Connected_Report_Handle, CONNECT_REPORT_TIME,ONCE,zbSoc_Heartbeat_Connected_Report_cb,zbSoc_Connected_Report_Handle);

	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		return false;
	}

	return true;
}

bool zbSoc_Heartbeat_Connected_Report_stop(void)
{
	
	if(zbSoc_Connected_Report_Handle == NULL)
		return false;
	
	tu_kill_evtimer(zbSoc_Connected_Report_Handle);

	return true;
}


static void
zbSoc_Heartbeat_Uart_Report_cb(void *args)
{

	log_debug("zbSoc_Heartbeat_Uart_Report_cb++\n");
	uart_event_t *uart_event = args;
	int timeout = zbSocUart_timeout_get();
	
	//��ʱ����
	if(timeout >= MAX_UART_TIMEOUT_COUNTER)
	{
		log_debug("Serial Timeout\n");
		zbSocUart_event_relase();
		sleep(1);
		zbSocUart_event(uart_event);
		zbSocUart_timeout_Set(0);
	}
	//������������
	else
	{
		log_debug("Serial Send HeartBeat\n");
		zbSocUart_timeout_Set(++timeout);
		zbSoc_GetCoordVersionCmd();

		//��ȡЭ������Ϣ
		if(mt_zdo_getCoorState() != DEV_ZB_COORD)
		{
			//��ȡЭ������Ϣ
			mt_Zdo_Ext_Nwk_Info_sreq();
		}
	}

	log_debug("zbSoc_Heartbeat_Uart_Report_cb--\n");
}

bool zbSoc_Heartbeat_Uart_Report_start(void)
{
	int ret = -1;

	log_debug("zbSoc_Heartbeat_Uart_Report_start++\n");

	
	if(zbSoc_Uart_Report_Handle == NULL)
		return false;
	
	ret =tu_set_evtimer_realtime(zbSoc_Uart_Report_Handle, HEART_UART_REPORT_TIME,CIRCLE,zbSoc_Heartbeat_Uart_Report_cb,&g_uart_event);

	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		return false;
	}

	log_debug("zbSoc_Heartbeat_Uart_Report_start--\n");
	return true;
}

bool zbSoc_Heartbeat_Uart_Report_stop(void)
{
	
	if(zbSoc_Uart_Report_Handle == NULL)
		return false;
	
	tu_kill_evtimer(zbSoc_Uart_Report_Handle);
	zbSocUart_timeout_Set(0);
	return true;
}

bool zbSoc_Heartbeat_Uart_Report_refresh(void)
{
	log_debug("zbSoc_Heartbeat_Uart_Report_refresh++\n");
	
	if(zbSoc_Uart_Report_Handle == NULL)
		return false;
		
	tu_reset_evtimer(zbSoc_Uart_Report_Handle,HEART_UART_REPORT_TIME,CIRCLE);

	zbSocUart_timeout_Set(0);
	
	log_debug("zbSoc_Heartbeat_Uart_Report_refresh--\n");
	return true;
}


#if PERMIMNG

static void
zbSoc_Heartbeat_Permimng_Report_cb(void *args)
{
	log_debug("zbSoc_Heartbeat_Permimng_Report_cb\n");
	//��ʱ������������
	SRPC_PermissionRequest_Ind();
}

bool zbSoc_Heartbeat_Permimng_Report_start(void)
{
	int ret = -1;
	
	if(zbSoc_Permimng_Report_Handle == NULL)
		return false;
	
	ret =tu_set_evtimer(zbSoc_Permimng_Report_Handle, PermiMngRequestSendTime,CIRCLE,zbSoc_Heartbeat_Permimng_Report_cb,zbSoc_Permimng_Report_Handle);

	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		return false;
	}

	return true;
}

bool zbSoc_Heartbeat_Permimng_Report_stop(void)
{
	
	if(zbSoc_Connected_Report_Handle == NULL)
		return false;
	
	tu_kill_evtimer(zbSoc_Permimng_Report_Handle);

	return true;
}

#endif

bool zbSoc_Heartbeat_Task_evInit(struct event_base *base)
{
	ASSERT(base != NULL);

	if(zbSoc_DeviceList_Report_Handle == NULL)
	{
		zbSoc_DeviceList_Report_Handle = tu_evtimer_new(base);
		if(zbSoc_DeviceList_Report_Handle == NULL)
		{
			log_debug("zbSoc_DeviceList_Report_Handle failed\n");
			return false;
		}
	}

	if(zbSoc_Heartbeat_Client_Handle == NULL)
	{
		zbSoc_Heartbeat_Client_Handle = tu_evtimer_new(base);
		if(zbSoc_Heartbeat_Client_Handle == NULL)
		{
			log_debug("zbSoc_Heartbeat_Client_Handle failed\n");
			return false;
		}
	}

	if(zbSoc_DevicePower_Report_Handle == NULL)
	{
		zbSoc_DevicePower_Report_Handle = tu_evtimer_new(base);
		if(zbSoc_DevicePower_Report_Handle == NULL)
		{
			log_debug("zbSoc_DevicePower_Report_Handle failed\n");
			return false;
		}
	}

	if(zbSoc_Connected_Report_Handle == NULL)
	{
		zbSoc_Connected_Report_Handle = tu_evtimer_new(base);
		if(zbSoc_Connected_Report_Handle == NULL)
		{
			log_debug("zbSoc_Connected_Report_Handle failed\n");
			return false;
		}
	}
	
	if(zbSoc_Uart_Report_Handle == NULL)
	{
		zbSoc_Uart_Report_Handle = tu_evtimer_new(base);
		if(zbSoc_Uart_Report_Handle == NULL)
		{
			log_debug("zbSoc_Uart_Report_Handle failed\n");
			return false;
		}
	}

#if PERMIMNG
	if(zbSoc_Permimng_Report_Handle == NULL)
	{
		zbSoc_Permimng_Report_Handle = tu_evtimer_new(base);
		if(zbSoc_Permimng_Report_Handle == NULL)
		{
			log_debug("zbSoc_Permimng_Report_Handle failed\n");
			return false;
		}
	}
#endif
	
	return true;
}

void zbSoc_Heartbeat_relase(void)
{
	if(zbSoc_DeviceList_Report_Handle )
	{
		tu_evtimer_free(zbSoc_DeviceList_Report_Handle);
	}

	if(zbSoc_Heartbeat_Client_Handle)
	{
		tu_evtimer_free(zbSoc_Heartbeat_Client_Handle);
	}

	if(zbSoc_DevicePower_Report_Handle)
	{
		tu_evtimer_free(zbSoc_DevicePower_Report_Handle);
	}

	if(zbSoc_Connected_Report_Handle)
	{
		tu_evtimer_free(zbSoc_Connected_Report_Handle);
	}

	if(zbSoc_Uart_Report_Handle)
	{
		tu_evtimer_free(zbSoc_Uart_Report_Handle);
	}
	
#if PERMIMNG
	if(zbSoc_Permimng_Report_Handle)
	{
		tu_evtimer_free(zbSoc_Permimng_Report_Handle);
	}
#endif
}



