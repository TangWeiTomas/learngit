/***********************************************************************************
 * �� �� ��   : SuccessiveEvents.h
 * �� �� ��   : Edward
 * ��������   : 2016��5��12��
 * �ļ�����   : �����������񼰶�ʱ����ʱ�����������豸�¼�
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/
#include "SuccessiveEvents.h"
#include "logUtils.h"
#include "globalVal.h"

//�¼����
uint8_t successive_event_interval = 0;

void successive_event_handler(void *args)
{
	log_debug("successive_event_handler++\n");
	if(args == NULL)
		return ;

	suc_event_t *event  = (suc_event_t*)args;
	
	Device_ProcessEventByDeviceId(&event->epInfo,event->dataSegment,event->length);

	if(event->timer.in_use == false)
	{
		log_debug("free event\n");
		free(event);
	}
	
	log_debug("successive_event_handler--\n");
	
	return;
}

int successive_set_event(epInfo_t *epInfo,char *data,uint8_t len)
{
	int ret = -1;
	
	log_debug("successive_set_event++\n");
	
	if(epInfo == NULL||data==NULL||len<=0)
	{	
		log_debug("args error\n");
		return ret;
	}
	
#if 0	
	if((epState = devState_getSwitchValue(epInfo))!=-1)
	{
		if(memcmp(&epState,data,len)==0)
			return 0;
	}
#endif

	log_debug("devState Change++\n");
	
	suc_event_t *event = (suc_event_t*)malloc(sizeof(suc_event_t));
	if(event == NULL)
	{
		log_debug("malloc failed\n");
		return ret;
	}
	
	memset(event,0,sizeof(suc_event_t));
	memcpy(&(event->epInfo),epInfo,sizeof(epInfo_t));
	memcpy(event->dataSegment,data,len);
	
	event->length = len;
	event->timer.in_use = false;
	event->timer.base = main_base_event_loop;
		
	successive_event_interval++;
	
	ret = tu_set_evtimer(&event->timer,successive_event_interval*200,false,(timer_handler_cb_t)successive_event_handler,(void *)event);

	zblist_add(epInfo,data,len);
	
	log_debug("successive_set_event--\n");

	return ret;
}

void successive_kill_event(suc_event_t *event)
{
	
}
