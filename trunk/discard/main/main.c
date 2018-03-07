/***********************************************************************************
 * 文 件 名   : main.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : linuxgateway网关程序的入口函数
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include <getopt.h>
#include "Types.h"
#include "globalVal.h"
#include "Polling.h"
#include "logUtils.h"
#include "zbSocCmd.h"
#include "SimpleDBMgr.h"
#include "ZigbeeFactoryReset.h"
#include "Zigbee_device_Heartbeat_Manager.h"
#include "interface_vDeviceList.h"
#include "Tcp_client.h"
#include "event_manager.h"
#include "timetask_manager.h"

#include <event2/event.h>

struct event_base *main_base_event_loop = NULL;

static struct option const cmd_optons[]=
{
	{"help",0,NULL,'h'},
	{"version",0,NULL,'v'},
	{"Debug",0,NULL,'d'},
	{NULL,0,NULL,0}
};

void main_usage(char* exeName)
{
    log_info("Usage: ./%s <port>\n", exeName);
    log_info("Eample: ./%s /dev/ttyS0\n", exeName);
}

/*****************************************************************************
 * 函 数 名  : main_parse_cmd
 * 负 责 人  : Edward
 * 创建日期  : 2016年5月9日
 * 函数功能  : 解析传递的命令参数
 * 输入参数  : int argc    命令个数
               char**argv  命令
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
uint8_t main_parse_cmd(int argc,char**argv)
{
	int cmd = 0;
	while((cmd = getopt_long(argc,argv,"vVhHdD",cmd_optons,NULL))!=-1)
	{
		switch(cmd)
		{
			case 'v':
			case 'V':
			{
			
				printf("Version: %s  SVN:%d  (%s , %s)\n",VERSION,SVERSION,__DATE__,__TIME__);
				return 1;
			}
			break;
			case 'h':
			case 'H':
			{
				printf("Usage: %s [options]\n", argv[0]);
				printf("Options:\n");
				printf("  -v\tDisplay %s version information\n", argv[0]);
				printf("  -h\tDisplay help information\n");
				printf("  -m\tenable printf log to logfile\n");
				printf("  -s\tenable syslog\n");
				printf("  -d\tenable show log with console\n");
				printf("Usage: %s <port> <options>\n", argv[0]);
				printf("Eample: %s /dev/ttyS0 -ds\n", argv[0]);
				return 1;
			}
			break;
			case 'd':
			case 'D':
			{
				use_log_debug = true;
				printf("fszigbeegw enable log debug\n");
			}
			break;
			default:
				return 1;
			break;
		}
	}
	
	return 0;
}

int main(int argc,char**argv)
{
	char *device = "/dev/ttyS0";
	
	if (argc < 2)
    {
        main_usage(argv[0]);
        log_err("attempting to use /dev/ttyS0\n");
    }
    else
    {
        device = argv[1];
    }
    
	//显示当前版本和帮助命令
	if(main_parse_cmd(argc,argv))
	{
		return 0;
	}
	
	//使用libevent框架
	main_base_event_loop = event_base_new();

	//设置优先级等级数量
	//event_base_priority_init(main_base_event_loop, 4);

	//串口初始化
	if(zbSocUart_evInit(main_base_event_loop,device)==false)
	{
		log_debug("zbSocUart_evInit");
		goto done;
	}

	//数据库初始化
	SimpleDBMgr_Init();

	//在内存中存储设备信息
	vdevListInit();
	
	//注册出厂设置复位接受处理信号
#if SIGNEL_FACTORY
	zbSoc_Signal_evInit(main_base_event_loop);
#endif 
	//命令重发
	zblist_evInit(main_base_event_loop);
	
	//事件初始化
	vEventList_Init();

	//定时任务初始化
	if(timeTaskList_evInit(main_base_event_loop) != true)
	{
		log_debug("timeTaskList_evInit failed\n");
		goto done;
	}

	//初始化连接心跳、数据心跳、定时电量上报
	if(zbSoc_Heartbeat_Task_evInit(main_base_event_loop) != true)
	{
		log_debug("zbSoc_Heartbeat_Task_evInit failed\n");
		goto done;
	}

	//串口心跳初始化
	if(zbSoc_Heartbeat_Uart_Report_start() != true)
	{
		log_debug("zbSoc_Heartbeat_Uart_Report_start failed\n");
		goto done;
	}

	//设备心跳上报初始化
	if(zbSoc_Heartbeat_DeviceList_Report_start()!=true)
	{
		log_debug("zbSoc_Heartbeat_DeviceList_Report_start failed\n");
		goto done;
	}
	
#if POWER_REPORT_ENBALE
	if(zbSoc_Heartbeat_DevicePower_Report_start()!=true)
	{
		log_debug("zbSoc_Heartbeat_DevicePower_Report_start failed\n");
		goto done;
	}
#endif

	//显示当前所有设备
	
#ifndef NDEBUG
	vdevListShowAllDeviceList();
#endif

#if PERMIMNG
	PermiMng_Init(main_base_event_loop);
#endif
	//创建本地Server
	if(tcp_server_evInit(main_base_event_loop)!=true)
	{
		log_debug("tcp_server_evInit failed\n");
		goto done;
	}
	
	//连接远程服务器
	if(tcp_client_evInit(main_base_event_loop)!=true)
	{
		log_debug("tcp_client_evInit failed\n");
		goto done;
	}
	
	if (udpBroadcast_Init(main_base_event_loop))
	{
		log_err("udplisten_Init error\n");
		goto done;
	}

//	mt_Zdo_Ext_Nwk_Info_sreq();
	
	//任务调度(libevent框架)
	event_base_dispatch(main_base_event_loop);
	
done:
	timeTaskList_ManagerRelase();
	zbSocUart_evRelase();
	SimpleDBMgr_release();
#if SIGNEL_FACTORY
	zbSoc_Signal_relase();
#endif
	zbSoc_Heartbeat_relase();
	tcp_relase();
	vEventList_Distory();
	timeTaskList_ManagerRelase();
	udpBroadcast_relase();
	
	if(main_base_event_loop)
		event_base_free(main_base_event_loop);

	return 0;
}

