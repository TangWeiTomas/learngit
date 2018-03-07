/**************************************************************************************************
 * Filename:       Zigbee_device_Heartbeat_Manager.c
 * Author:             edward
 * E-Mail:          ouxiangping@feixuekj.com
 * Description:    注册和管理zigbee设备的心跳功能
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

#include "watchdog.h"

#include <event2/event.h>
#include "indLight.h"

extern uart_event_t g_uart_event;

//设备心跳定时上报任务
static tu_evtimer_t *zbSoc_DeviceList_Report_Handle = NULL;
//服务器与主机连接的心跳定时器
static tu_evtimer_t *zbSoc_Heartbeat_Client_Handle = NULL;
//电量上报定时任务
static tu_evtimer_t *zbSoc_DevicePower_Report_Handle = NULL;
//当连接上服务器后，定时5后发送心跳和注册消息
static tu_evtimer_t *zbSoc_Connected_Report_Handle = NULL;
//串口心跳包
static tu_evtimer_t *zbSoc_Uart_Report_Handle = NULL;

#if SUPPORT_YINJIA_HEATBEAT
//盈家专用心跳包
static tu_evtimer_t *zbSoc_Heartbeat_YinJiaHandle = NULL;
#endif

#if PERMIMNG
//当请求权限是启动定时去请求
static tu_evtimer_t *zbSoc_Permimng_Report_Handle = NULL;
#endif

/************************************************************************
* 函数名 :ZbSocHeartbeat_HeartPacketSend(void)
* 描述   :   发送心跳包
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void ZbSocHeartbeat_HeartPacketSend(void)
{
    uint8_t allDevNum;
    uint8_t buf[MaxPacketLength];
    uint16_t bufLen;

    log_debug("ZbSocHeartbeat_HeartPacketSend++ \n");

#if 1
    //devMgr_getAllRegisterOnlineDevices(&allDevNum,buf,&bufLen);
    allDevNum = vdevListGetAllRegOnlineDev(buf,&bufLen);
    log_debug("allDevNum=%d bufLen = %d \n",allDevNum,bufLen);
    
    SRPC_heartPacketInd(allDevNum,buf,bufLen);
#endif	

	log_debug("ZbSocHeartbeat_HeartPacketSend-- \n");

	
    return;
}

void ZbSocHeartbeat_OnLineDeviceResp(void)
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

    //每周三的凌晨3点整自动重启运行
    if((timenow->tm_wday == 3)&&(timenow->tm_hour == 3)&&(timenow->tm_min == 0)&&(timenow->tm_sec == 0))
    {
			//重启协调器
			mt_Sys_Reset_sreq();
			//reboot system
			system("reboot -f");
    }
    
	SRPC_RoomfairyRegistered();
	vdevListSetTimeOutCnt();
	ZbSocHeartbeat_HeartPacketSend();
	//喂狗
	watchdogfeed();
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

	//喂狗
	watchdogfeed();

	return true;

}

#if SUPPORT_YINJIA_HEATBEAT


void zbSoc_Heartbeat_YinJia_Report_cb(void *args)
{
#if 0
	unsigned char buf[15] = {0};
	uint8_t idx = 0;
	log_debug("zbSoc_Heartbeat_YinJia_Report_cb++\n");

	memcpy(buf,roomfairy_WifiMac,6);
	idx = 6;
	buf[idx++] = 0x48;
	buf[idx++] = 0x41;
	buf[idx++] = 0x50;
	buf[idx++] = 0x49;
	buf[idx++] = 0x00;
	buf[idx++] = 0x00;
	buf[idx++] = 0x81;
	buf[idx] = 0xFF;

  	cmdMsgSend(buf,15);
  	log_debug("zbSoc_Heartbeat_YinJia_Report_cb--\n");
	log_debug("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14]);
#endif
	ZbSocHeartbeat_HeartPacketSend();

}

bool zbSoc_Heartbeat_YinJia_Report_start(void)
{
	int ret = -1;
	
	if(zbSoc_Heartbeat_YinJiaHandle == NULL)
		return false;
	
	ret =tu_set_evtimer_realtime(zbSoc_Heartbeat_YinJiaHandle, HEART_PACKET_YINJIA_TIME,CIRCLE,zbSoc_Heartbeat_YinJia_Report_cb,zbSoc_Heartbeat_YinJiaHandle);
//	ret =tu_set_evtimer_realtime(zbSoc_Heartbeat_YinJiaHandle, HEART_PACKET_YINJIA_TIME,CIRCLE,zbSoc_Heartbeat_DeviceList_Report_cb,zbSoc_Heartbeat_YinJiaHandle);

	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		return false;
	}

	return true;
}

bool zbSoc_Heartbeat_YinJia_Report_stop(void)
{
	if(zbSoc_Heartbeat_YinJiaHandle == NULL)
		return false;
	
	tu_kill_evtimer(zbSoc_Heartbeat_YinJiaHandle);

	return true;
}


#endif
bool zbSoc_Heartbeat_Client_Report_start(timer_handler_cb_t ZbSocHeartbeat_ZbClientReported_Handler,void * timer_handler_arg)
{
	int ret = -1;
	
	if(zbSoc_Heartbeat_Client_Handle == NULL)
		return false;
	log_debug("zbSoc_Heartbeat_Client_Report_start++\n");
	ret =tu_set_evtimer_realtime(zbSoc_Heartbeat_Client_Handle, CLIENT_HEART_PACKET_INTERVAL_TIME,ONCE,ZbSocHeartbeat_ZbClientReported_Handler,timer_handler_arg);

	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		return false;
	}
	log_debug("zbSoc_Heartbeat_Client_Report_start--\n");
	return true;
}

bool zbSoc_Heartbeat_Client_Report_stop(void)
{

	if(zbSoc_Heartbeat_Client_Handle == NULL)
		return false;

	log_debug("zbSoc_Heartbeat_Client_Report_stop++\n");
	tu_kill_evtimer(zbSoc_Heartbeat_Client_Handle);
	log_debug("zbSoc_Heartbeat_Client_Report_stop--\n");
	return true;
}

bool zbSoc_Heartbeat_Client_Report_refresh(void)
{
	log_debug("zbSoc_Heartbeat_Client_Report_refresh++\n");
	if(zbSoc_Heartbeat_Client_Handle == NULL)
		return false;
	tu_reset_evtimer(zbSoc_Heartbeat_Client_Handle,CLIENT_HEART_PACKET_INTERVAL_TIME,ONCE);
	log_debug("zbSoc_Heartbeat_Client_Report_refresh++\n");

	return true;
}

void zbSoc_Heartbeat_DevicePower_Report_cb(void *args)
{

	//Context必须设置为0 不然会造成数据库文件错乱
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
			case ZCL_HA_DEVICEID_IAS_ZONE:
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
		//上报注册消息
		SRPC_RoomfairyRegistered();
		sleep(1);
		//上报心跳消息
		//ZbSocHeartbeat_HeartPacketSend();
		//sleep(1);
#if POWER_REPORT_ENBALE
		//上报电量信息
		zbSoc_Heartbeat_DevicePower_Report_cb(NULL);
#endif 
		//真正建立连接后，设置LED状态
	 	led_SetSysLedStatus(SYS_LED,LED_ON);
	 	led_SetSysLedStatus(CONNECT_LED,LED_OFF);
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
	log_debug("zbSoc_Heartbeat_Connected_Report_stop++\n");
	tu_kill_evtimer(zbSoc_Connected_Report_Handle);
	log_debug("zbSoc_Heartbeat_Connected_Report_stop--\n");

	return true;
}


static void
zbSoc_Heartbeat_Uart_Report_cb(void *args)
{

	log_debug("zbSoc_Heartbeat_Uart_Report_cb++\n");
	uart_event_t *uart_event = args;
	int timeout = zbSocUart_timeout_get();
	
	//超时处理
	if(timeout >= MAX_UART_TIMEOUT_COUNTER)
	{
		log_debug("Serial Timeout\n");
		zbSocUart_event_relase();
		sleep(1);
		zbSocUart_event(uart_event);
		zbSocUart_timeout_Set(0);
	}
	//发送心跳命令
	else
	{
		log_debug("Serial Send HeartBeat\n");
		zbSocUart_timeout_Set(++timeout);
		zbSoc_GetCoordVersionCmd();

		//获取协调器信息
		if(mt_zdo_getCoorState() != DEV_ZB_COORD)
		{
			//获取协调器信息
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
	//定时发送请求命令
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

#if 0
#if SUPPORT_YINJIA_HEATBEAT

	if(zbSoc_Heartbeat_YinJiaHandle == NULL)
	{
		zbSoc_Heartbeat_YinJiaHandle = tu_evtimer_new(base);
		if(zbSoc_Heartbeat_YinJiaHandle == NULL)
		{
			log_debug("zbSoc_Heartbeat_YinJiaHandle failed\n");
			return false;
		}
	}

#endif
#endif

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

#if SUPPORT_YINJIA_HEATBEAT
	if(zbSoc_Uart_Report_Handle)
	{
		tu_evtimer_free(zbSoc_Uart_Report_Handle);
	}
#endif

	
#if PERMIMNG
	if(zbSoc_Permimng_Report_Handle)
	{
		tu_evtimer_free(zbSoc_Permimng_Report_Handle);
	}
#endif
}

