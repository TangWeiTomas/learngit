/***********************************************************************************
 * 文 件 名   : indLight.c
 * 负 责 人   : Edward
 * 创建日期   : 2017年2月21日
 * 文件描述   : led 状态指示灯
 * 版权说明   : Copyright (c) 2008-2017   无锡飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

/*********************************************************************
* INCLUDES
*/
#include<stdio.h>
#include "indLight.h"
#include "fileMng.h"
#include "logUtils.h"
#include "globalVal.h"
#include "Timer_utils.h"

/*********************************************************************
* MACROS
*/
		/*服务器连接指示灯*/
#define SYSLED_SYS_LED						"wr8305rt:sys"
		
		/*新版本服务器连接指示灯*/
#define SYSLED_CONNECTED_LED			"zbserver"
	 
		/*zigbee电源控制*/
#define SYS_ZB_POWER_CTRL					"zpower"
		
#define SYSLED_ON									"on"
#define SYSLED_OFF								"off"
#define SYSLED_BLINK							"blink"

/*********************************************************************
* CONSTANTS
*/
#define MAX_LED_SIZE	5
/*********************************************************************
* TYPEDEFS
*/
typedef tu_evtimer_t ledBlink;
/*********************************************************************
* GLOBAL VARIABLES
*/

//服务器指示灯位置
#define LED_SERVICE_INDICATOR_PATH		"/sys/class/leds/zbserver/brightness"
#define LED_SERVICE_INDICATOR_OFF		"/bin/zbLedIndicator off"
#define LED_SERVICE_INDICATOR_ON 		"/bin/zbLedIndicator on"

#define LED_CMD				"/bin/zbLedIndicator"
/*********************************************************************
* LOCAL VARIABLES
*/

static ledBlink *ledIndBlink = NULL ;
static int LedStatus = LIGHT_OFF;
static uint8_t ledCurtStatus[MAX_LED_SIZE] = {0xff};
static uint8_t ledSetCnt[MAX_LED_SIZE] = {0x00};

/*********************************************************************
* LOCAL FUNCTIONS
*/

/*********************************************************************
* GLOBAL FUNCTIONS
*/

uint8_t led_SetSysLedStatus(uint8_t led,uint8_t status )
{
	pid_t mthread = 0;
	char *leds = NULL;
	char *stu = NULL;
	char cmds[128] = {0};

	if(led == SYS_LED)
		leds = SYSLED_SYS_LED;
	else if (led == CONNECT_LED)
		leds = SYSLED_CONNECTED_LED;
	else
		return FAILED;

	if(status == LED_ON)
		stu = SYSLED_ON;
	else if(status == LED_OFF)
		stu = SYSLED_OFF;
	else if(status == LED_BLINK)
		stu = SYSLED_BLINK;
	else
		return FAILED;

/*
	log_debug("led_SetSysLedStatus = %d , %d\n",ledCurtStatus[led],status);
	
	if(led < MAX_LED_SIZE && ledCurtStatus[led]!=status)
	{
		//设置次数
		if(ledSetCnt[led]++ > 5)
		{
			ledSetCnt[led] = 0;
			ledCurtStatus[led] = status;
		}
	}
	else
	{
		return SUCCESS;
	}
*/
	sprintf(cmds,"%s %s %s",LED_CMD,leds,stu);
	
	log_debug("led_SetSysLedStatus = %s\n",cmds);

	mthread = system(cmds);
		
	if( 0 > mthread)
		return -1;
		
	if(WIFEXITED(mthread))
	{
		status = WEXITSTATUS(mthread);
		//关、开
		if(status == 0 || status == 1)
		{
			return status;
		}
	}

	return SUCCESS;
}


/*****************************************************************************
 * 函 数 名  : led_Serviceindicator
 * 负 责 人  : Edward
 * 创建日期  : 2017年3月15日
 * 函数功能  : 连接到服务器指示灯
 * 输入参数  : light  0:关闭 1:开启
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/

int8_t led_Serviceindicator(IndLight_Type_t light)
{
	pid_t mthread = 0;
	int status = 0;
	char *cmd = NULL;
	
	if(!isFileExist(LED_SERVICE_INDICATOR_PATH))
		return -1;

	if(light == LIGHT_ON)
		cmd = LED_SERVICE_INDICATOR_ON;
	else
		cmd = LED_SERVICE_INDICATOR_OFF;

	mthread = system(cmd);
	
	if( 0 > mthread)
		return -1;
		
	if(WIFEXITED(mthread))
	{
		status = WEXITSTATUS(mthread);
		//关、开
		if(status == 0 || status == 1)
		{
			return status;
		}
	}

    return -1 ;
}

void led_Serviceindicator_Blink_Event(void *args)
{
	log_debug("led_Serviceindicator_Blink_Event++\n");
	

	if(LedStatus)
	{
		LedStatus = LIGHT_OFF;
		led_Serviceindicator(LIGHT_OFF);
	}
	else
	{
		LedStatus = LIGHT_ON;
		led_Serviceindicator(LIGHT_ON);	
	}

	log_debug("led_Serviceindicator_Blink_Event--\n");

}

int8_t  led_Serviceindicator_Blink(uint64_t second)
{
	int ret = -1;
	log_debug("led_Serviceindicator_Blink++\n");

	if(ledIndBlink == NULL)
		ledIndBlink = tu_evtimer_new(main_base_event_loop);

	if(ledIndBlink == NULL)
		return ret;

	ret = tu_set_evtimer(ledIndBlink,second*1000,true,led_Serviceindicator_Blink_Event,ledIndBlink);

	if(ret == -1)
		return ret;

	LedStatus = LIGHT_OFF;
	
	log_debug("led_Serviceindicator_Blink--\n");

	return ret;	
}


int8_t  led_Serviceindicator_Blink_Off(void)
{
	if(ledIndBlink)
	{
	 	tu_evtimer_free(ledIndBlink);
	}

	ledIndBlink = NULL;
}

/*********************************************************************/

