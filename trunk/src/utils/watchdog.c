/**************************************************************************
 * Filename:       watchdog.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    程序看门狗，如果程序出现异常，重启程序
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/


/*********************************************************************
* INCLUDES
*/
#include<unistd.h>
#include<stdio.h>
#include<stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "logUtils.h"
/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/

#define WATCHDOG_TIMEOUT	5 //5min

/*********************************************************************
* TYPEDEFS
*/
typedef struct _watchdog
{
	uint8_t timeCnt;
	uint8_t timeOut ;
	pthread_mutex_t lock;
	pthread_t pthread;
}watchdog_t;


/*********************************************************************
* GLOBAL VARIABLES
*/

       
/*********************************************************************
* LOCAL VARIABLES
*/

static watchdog_t watchdogs;
static pid_t mainPid = 0;
/*********************************************************************
* LOCAL FUNCTIONS
*/

static void watchdog_put(void)
{
	pthread_mutex_lock(&watchdogs.lock);
	watchdogs.timeCnt = 0;
	pthread_mutex_unlock(&watchdogs.lock);
}

static uint8_t watchdog_get(void)
{
	uint8_t data = 0;
	pthread_mutex_lock(&watchdogs.lock);
	watchdogs.timeCnt++;
	data = watchdogs.timeCnt;
	pthread_mutex_unlock(&watchdogs.lock);

	return data;
}

void *watchdogProcessHandler(void *args)
{

	uint8_t data = 0;

	
	while(1)
	{
		data = watchdog_get();

		log_debug("watchdogProcessHandler %d\n",data);

		if(data > watchdogs.timeOut)
		{
			//超时，重启程序
			if(kill(mainPid,SIGKILL)==0)
				break;
		}
	
		sleep(60);
	}

	pthread_detach(pthread_self());
}


/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* @fn          watchdogInit
*
* @brief       Add device to descovery list.
*
* @param       pSimpleDescRsp - SimpleDescRsp containing epInfo of new EP.
*
* @return      index of device or 0xFF if no room in list
*/
int watchdogInit(void)
{
	int err = 0;
	
	mainPid = getpid();

	log_debug("PID=%d\n",mainPid);

	
	pthread_mutex_init(&(watchdogs.lock),NULL);
	watchdogs.timeCnt = 0;
	watchdogs.timeOut = WATCHDOG_TIMEOUT;

	err = pthread_create(&(watchdogs.pthread),NULL,watchdogProcessHandler,NULL);
	if(err != 0)
	{
		log_debug("watchdog pthread create failed\n");
	}
	
	return err;
}

void watchdogfeed(void)
{
	log_debug("watchdogfeed++\n");
	watchdog_put();
	log_debug("watchdogfeed--\n");
}

/*********************************************************************/

