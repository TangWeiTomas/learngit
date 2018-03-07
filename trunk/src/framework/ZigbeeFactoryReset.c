/***********************************************************************************
 * 文 件 名   : ZigbeeFactoryReset.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : 信号注册及处理
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include "ZigbeeFactoryReset.h"
#include "interface_srpcserver.h"
#include "zbSocCmd.h"
#include "errorCode.h"
#include "logUtils.h"
#include "zbSocUart.h"
#include "Polling.h"

#include <event2/event.h>
#include <event2/event-config.h>
#include <event2/util.h>
//#include <assert.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

static struct event *sigusr1_event = NULL;
static struct event *sigusr2_event = NULL;
static tu_evtimer_t *sysreboot_event = NULL;
static tu_evtimer_t *wifi_restart_event = NULL;
//static tu_evtimer_t *sysFactory_event = NULL;

static void zbSoc_FactoryReset_signal_cb(evutil_socket_t fd, short event, void *arg)
{
	log_debug("zbSoc_FactoryReset_signal_cb\n");
	int cnt = 0;
	
#if 0
	zbSoc_RevertFactorySettingCmd();
#endif

	while(cnt++ < 5)
	{
    	SRPC_GatewayFactoryInd();
    }
}

static bool zbSoc_FactoryReset_Singal_evInit(struct event_base *base)
{
	sigusr1_event = evsignal_new(base, SIGUSR1, zbSoc_FactoryReset_signal_cb, base);
	if(sigusr1_event == NULL)
		return false;
	if(evsignal_add(sigusr1_event, NULL)==0)
		return true;
	event_free(sigusr1_event);
	return false;
}

static void zbSoc_reboot_Singal_cb(evutil_socket_t fd, short event, void *arg)
{
	log_debug("zbSoc_reboot_Singal_cb\n");
	mt_Sys_Reset_sreq();
}

static bool  zbSoc_reboot_Singal_evInit(struct event_base *base)
{
	sigusr2_event = evsignal_new(base, SIGUSR2, zbSoc_reboot_Singal_cb, base);
	if(sigusr2_event == NULL)
		return false;
	if(evsignal_add(sigusr2_event, NULL)==0)
		return true;
	event_free(sigusr2_event);
	return false;
}

void  zbSoc_Signal_evInit(struct event_base *base)
{
	if(zbSoc_reboot_Singal_evInit(base)==false)
	{
		log_debug("zbSoc_reboot_Singal_evInit failed\n");
	}
	
	if(zbSoc_FactoryReset_Singal_evInit(base)==false)
	{
		log_debug("zbSoc_FactoryReset_Singal_evInit failed\n");
	}

	return ;
}

void zbSoc_Signal_relase(void)
{
	if(sigusr2_event)
		event_free(sigusr2_event);
	if(sigusr1_event)
		event_free(sigusr1_event);
}

void zbSoc_SystemReboot_handler(void *args)
{
	log_debug("zbSoc_SystemReboot_handler++\n");

#ifdef OPENWRT_TEST
	    system("reboot");
	    usleep(1000);
	    exit(0);
#endif

	log_debug("zbSoc_SystemReboot_handler--\n");

	tu_evtimer_free((tu_evtimer_t * )args);
}

int zbSoc_SystemReboot(uint64_t milliseconds)
{
	int ret = -1;
	log_debug("zbSoc_SystemReboot++\n");

	sysreboot_event = tu_evtimer_new(main_base_event_loop);
	if(sysreboot_event == NULL)
		return ret;

	ret = tu_set_evtimer(sysreboot_event,milliseconds,false,zbSoc_SystemReboot_handler,sysreboot_event);

	if(ret == -1)
		return ret;
		
	log_debug("zbSoc_SystemReboot--\n");

	return ret;
}

void zbSoc_WifiRestart_handler(void *args)
{

	log_debug("zbSoc_WifiRestart_handler++\n");

#ifdef OPENWRT_TEST
	    system("nr && wifi");
	    usleep(3000);
	    exit(0);
#endif

	log_debug("zbSoc_WifiRestart_handler--\n");
	tu_evtimer_free((tu_evtimer_t * )args);
}

int zbSoc_WifiRestart(uint64_t milliseconds)
{
	int ret = -1;

	log_debug("zbSoc_WifiRestart++\n");
	
	wifi_restart_event = tu_evtimer_new(main_base_event_loop);
	if(wifi_restart_event == NULL)
		return ret;

	ret = tu_set_evtimer(wifi_restart_event,milliseconds,false,zbSoc_WifiRestart_handler,wifi_restart_event);

	if(ret == -1)
		return ret;
		
	log_debug("zbSoc_WifiRestart--\n");
	return ret;
}

#if 0
static void zbSoc_SystemReFactory_handler(void *args)
{

#ifdef OPENWRT_TEST
		//恢复出厂设置，并重启
	    system("/sbin/jffs2reset -y && reboot");
	    exit(0);
#endif
	tu_evtimer_free((tu_evtimer_t * )args);
}

static int zbSoc_SystemFactory(uint64_t milliseconds)
{
	int ret = -1;
	
	sysFactory_event = tu_evtimer_new(main_base_event_loop);
	if(sysFactory_event == NULL)
		return ret;

	ret = tu_set_evtimer(sysFactory_event,milliseconds,false,zbSoc_SystemReFactory_handler,sysFactory_event);

	if(ret == -1)
		return ret;

	return ret;
}
#endif

