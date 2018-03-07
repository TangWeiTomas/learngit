/***********************************************************************************
 * �� �� ��   : SuccessiveEvents.h
 * �� �� ��   : Edward
 * ��������   : 2016��5��12��
 * �ļ�����   : �����������񼰶�ʱ����ʱ�����������豸�¼�
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/
#include "zbSocCmd.h"
#include <stdio.h>
#include <stdlib.h>
#include "Timer_utils.h"
#include <string.h>

typedef struct {
	epInfo_t epInfo;
	tu_evtimer_t timer;
	uint8_t length;						//���8�ֽ�
	uint8_t dataSegment[DATASEGMENT];
}suc_event_t;

extern uint8_t successive_event_interval;

int successive_set_event(epInfo_t *epInfo,char *data,uint8_t len);

