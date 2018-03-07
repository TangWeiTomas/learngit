/***********************************************************************************
 * 文 件 名   : Out_Of_Power.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年4月5日
 * 文件描述   : 离人断电功能
 * 版权说明   : Copyright (c) 2008-2016  无锡飞雪科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/msg.h>
#include <stdint.h>
	
#include "Types.h"
#include "cJSON.h"
//#include "hal_types.h"
#include "interface_scenelist.h"
	
#include "interface_srpcserver.h"
#include "interface_srpcserver_defs.h"
#include "zbSocCmd.h"
#include "errorCode.h"
#include "globalVal.h"
//#include "globalDef.h"
#include "comParse.h"
#include "interface_devicelist.h"
	
#include "logUtils.h"
//#include "Socket_Interface.h"
#include "fileMng.h"
#include "zbSocCmd.h"
#include "Zigbee_device_Heartbeat_Manager.h"
#include "interface_deviceStatelist.h"
#include "interface_eventlist.h"
#include "SimpleDBTxt.h"
#include "scene_manager.h"
#include "interface_timetasklist.h"
#include "One_key_match.h"
#include "zbSocMasterControl.h"
#include "interface_vDeviceList.h"
#include "zbSocPrivate.h"

typedef tu_evtimer_t OutOfPower_t;

static OutOfPower_t *mOutOfPower = NULL;

/*****************************************************************************
 * 函 数 名  : Infrared_body_sensor_Set
 * 负 责 人  : Edward
 * 创建日期  : 2016年4月5日
 * 函数功能  : 设置红外人体检测功能启动或关闭
 * 输入参数  : bool state  0:关闭红外人体检测，1:启动红外人体检测
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void Infrared_body_sensor_Set(bool state)
{
	epInfo_t *epInfo;
	uint32_t context = 0;
	log_debug("Infrared_body_sensor_Set++\n");
	while((epInfo = devListGetNextDev(&context))!=NULL)
	{
		if((epInfo->deviceID == ZB_DEV_INFRARED_BODY_SENSOR)&&(epInfo->onlineflag == true))
		{	
			//设置红外设备
			zbSoc_SetDevValidReq(state,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
	}
	log_debug("Infrared_body_sensor_Set++\n");
}

/*****************************************************************************
 * 函 数 名  : Out_Of_Power_Handler
 * 负 责 人  : Edward
 * 创建日期  : 2016年4月5日
 * 函数功能  : 触发离人断电处理函数
 * 输入参数  : void *args  无
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void Out_Of_Power_Handler(void *args)
{
	log_debug("Out_Of_Power_Handler++\n");

	if(args == NULL)
		return;
	
	//关闭取电开关设备
	epInfo_t *epInfo;
	uint32_t context = 0;
	OutOfPower_t *mOutOfPower = (OutOfPower_t*)args;

	//关闭红外检测设备
	Infrared_body_sensor_Set(Off);

	//查找智能取电开关设备，关闭取电开关
	while((epInfo = devListGetNextDev(&context))!=NULL)
	{
		if(epInfo->deviceID == ZB_DEV_POWER_SWITCH)
		{
			zbSoc_SetGenOnOffState(Off,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
	}

	if(mOutOfPower->in_use == false)
	{
		tu_evtimer_free(mOutOfPower);
	}
	 	
	log_debug("Out_Of_Power_Handler--\n");
}

/*****************************************************************************
 * 函 数 名  : Out_Of_Power_Start
 * 负 责 人  : Edward
 * 创建日期  : 2016年4月5日
 * 函数功能  : 间隔millisecends秒后触发离人断电
 * 输入参数  : uint64_t milliseconds  定时间隔时间
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
int Out_Of_Power_Start(uint64_t milliseconds)
{
#if OUT_POWER_ENBALE
	log_debug("Out_Of_Power_Start++\n");
	if(mOutOfPower == NULL)
	{
		mOutOfPower = tu_evtimer_new(main_base_event_loop);
		if(mOutOfPower == NULL)
		{
			log_debug("ERROR: tu_evtimer_new failed\n");
			return -1;
		}
	}

	//if(tm_timer_add(mOutOfPower,milliseconds,ONCE,Out_Of_Power_Handler,NULL)==false)
	if(tu_set_timer_realtime(mOutOfPower,milliseconds,ONCE,Out_Of_Power_Handler,(void*)mOutOfPower)==-1)
	{
		log_err("ERROR:tm_timer_add Failed!\n");
		tu_evtimer_free(mOutOfPower);
		mOutOfPower = NULL;
		return -1;
	}

	//启动相应的红外检测设备
	Infrared_body_sensor_Set(On);
	log_debug("Out_Of_Power_Start--\n");
#endif
	return 0;
}

/*****************************************************************************
 * 函 数 名  : Out_Of_Power_Stop
 * 负 责 人  : Edward
 * 创建日期  : 2016年4月5日
 * 函数功能  : 关闭离人断电
 * 输入参数  : void  无
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/

void Out_Of_Power_Stop(void)
{
#if OUT_POWER_ENBALE
	log_debug("Out_Of_Power_Stop++\n");
	static uint8_t count = 0;
	//检测5次方才有效
	count++;
	if(count > 5)
	{
		if(mOutOfPower != NULL)
		{
			tu_kill_evtimer(mOutOfPower);
			tu_evtimer_free(mOutOfPower);
			 mOutOfPower = NULL;
			//关闭红外检测设备
			Infrared_body_sensor_Set(Off);
		}
		count = 0;
	}
	log_debug("Out_Of_Power_Stop--\n");
#endif
}

