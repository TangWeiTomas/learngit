/***********************************************************************************
 * 文 件 名   : SuccessiveEvents.h
 * 负 责 人   : Edward
 * 创建日期   : 2016年5月12日
 * 文件描述   : 处理场景，任务及定时任务时连续触发的设备事件
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#include "zbSocCmd.h"
#include <stdio.h>
#include <stdlib.h>
#include "Timer_utils.h"
#include <string.h>

typedef struct {
	epInfo_t epInfo;
	tu_evtimer_t timer;
	uint8_t length;						//最大8字节
	uint8_t dataSegment[DATASEGMENT];
}suc_event_t;

extern uint8_t successive_event_interval;

int successive_set_event(epInfo_t *epInfo,char *data,uint8_t len);

