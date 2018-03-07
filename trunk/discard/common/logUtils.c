/***********************************************************************************
 * 文 件 名   : LogUtils.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : 日志输出功能
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include "logUtils.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>


//bool use_log_debug  = false;	//调试日志

#define LOG_CACHE_BUF_SIZE	512

void logUtils_array(const uint8_t *array,uint16_t length,uint8_t *IntervalSymbol)
{
	uint8_t record[LOG_CACHE_BUF_SIZE] = {0};
	uint16_t count=0;
	uint16_t reclen = 0; 

	uint16_t cnt = 0;

	for(cnt = 0;cnt < length;cnt++)
	{
		reclen = strlen(record);
		
		if((reclen+4) >= LOG_CACHE_BUF_SIZE)
			break;
			
		if(IntervalSymbol == NULL)
		{
			sprintf(record+reclen,"%x ",array[cnt]);
		}
		else
		{
			if(cnt < length -1)
			{
				sprintf(record+reclen,"%x%s",array[cnt],IntervalSymbol);
			}
			else
			{
				sprintf(record+reclen,"%x",array[cnt]);
			}
		}
	}

	if(use_log_debug)
	{
		fprintf(stderr,"%s\n",record);
	}
	else
	{
		syslog(LOG_INFO | LOG_USER, "%s\n",record);
	}
}

void logUtils_msg(const char *fmt ,...)
{
	va_list ap;
	va_start(ap,fmt);

	if(use_log_debug)
	{
		vfprintf(stderr, fmt, ap);
	}
	else
	{
		vsyslog(LOG_INFO | LOG_USER, fmt, ap);
	}
	
	va_end(ap);
}

char*  getlocaltime(void)
{
	static char times[24]={0};
	time_t timep;
	struct tm *tp;
	time(&timep);
	tp = localtime(&timep);
	sprintf(times,"%d-%d-%d %d:%d:%d",(1900+tp->tm_year),( 1+tp->tm_mon), tp->tm_mday,tp->tm_hour, tp->tm_min, tp->tm_sec);
	return times;
}



