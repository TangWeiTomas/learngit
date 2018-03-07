/***********************************************************************************
 * �� �� ��   : Out_Of_Power.c
 * �� �� ��   : Edward
 * ��������   : 2016��4��5��
 * �ļ�����   : ���˶ϵ繦��
 * ��Ȩ˵��   : Copyright (c) 2008-2016  ������ѩ�Ƽ����޹�˾
 * ��    ��   : 
 * �޸���־   : 
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
 * �� �� ��  : Infrared_body_sensor_Set
 * �� �� ��  : Edward
 * ��������  : 2016��4��5��
 * ��������  : ���ú��������⹦��������ر�
 * �������  : bool state  0:�رպ��������⣬1:��������������
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

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
			//���ú����豸
			zbSoc_SetDevValidReq(state,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
	}
	log_debug("Infrared_body_sensor_Set++\n");
}

/*****************************************************************************
 * �� �� ��  : Out_Of_Power_Handler
 * �� �� ��  : Edward
 * ��������  : 2016��4��5��
 * ��������  : �������˶ϵ紦����
 * �������  : void *args  ��
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
void Out_Of_Power_Handler(void *args)
{
	log_debug("Out_Of_Power_Handler++\n");

	if(args == NULL)
		return;
	
	//�ر�ȡ�翪���豸
	epInfo_t *epInfo;
	uint32_t context = 0;
	OutOfPower_t *mOutOfPower = (OutOfPower_t*)args;

	//�رպ������豸
	Infrared_body_sensor_Set(Off);

	//��������ȡ�翪���豸���ر�ȡ�翪��
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
 * �� �� ��  : Out_Of_Power_Start
 * �� �� ��  : Edward
 * ��������  : 2016��4��5��
 * ��������  : ���millisecends��󴥷����˶ϵ�
 * �������  : uint64_t milliseconds  ��ʱ���ʱ��
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

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

	//������Ӧ�ĺ������豸
	Infrared_body_sensor_Set(On);
	log_debug("Out_Of_Power_Start--\n");
#endif
	return 0;
}

/*****************************************************************************
 * �� �� ��  : Out_Of_Power_Stop
 * �� �� ��  : Edward
 * ��������  : 2016��4��5��
 * ��������  : �ر����˶ϵ�
 * �������  : void  ��
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/

void Out_Of_Power_Stop(void)
{
#if OUT_POWER_ENBALE
	log_debug("Out_Of_Power_Stop++\n");
	static uint8_t count = 0;
	//���5�η�����Ч
	count++;
	if(count > 5)
	{
		if(mOutOfPower != NULL)
		{
			tu_kill_evtimer(mOutOfPower);
			tu_evtimer_free(mOutOfPower);
			 mOutOfPower = NULL;
			//�رպ������豸
			Infrared_body_sensor_Set(Off);
		}
		count = 0;
	}
	log_debug("Out_Of_Power_Stop--\n");
#endif
}

