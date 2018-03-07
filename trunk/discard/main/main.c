/***********************************************************************************
 * �� �� ��   : main.c
 * �� �� ��   : Edward
 * ��������   : 2016��7��19��
 * �ļ�����   : linuxgateway���س������ں���
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
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
 * �� �� ��  : main_parse_cmd
 * �� �� ��  : Edward
 * ��������  : 2016��5��9��
 * ��������  : �������ݵ��������
 * �������  : int argc    �������
               char**argv  ����
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

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
    
	//��ʾ��ǰ�汾�Ͱ�������
	if(main_parse_cmd(argc,argv))
	{
		return 0;
	}
	
	//ʹ��libevent���
	main_base_event_loop = event_base_new();

	//�������ȼ��ȼ�����
	//event_base_priority_init(main_base_event_loop, 4);

	//���ڳ�ʼ��
	if(zbSocUart_evInit(main_base_event_loop,device)==false)
	{
		log_debug("zbSocUart_evInit");
		goto done;
	}

	//���ݿ��ʼ��
	SimpleDBMgr_Init();

	//���ڴ��д洢�豸��Ϣ
	vdevListInit();
	
	//ע��������ø�λ���ܴ����ź�
#if SIGNEL_FACTORY
	zbSoc_Signal_evInit(main_base_event_loop);
#endif 
	//�����ط�
	zblist_evInit(main_base_event_loop);
	
	//�¼���ʼ��
	vEventList_Init();

	//��ʱ�����ʼ��
	if(timeTaskList_evInit(main_base_event_loop) != true)
	{
		log_debug("timeTaskList_evInit failed\n");
		goto done;
	}

	//��ʼ������������������������ʱ�����ϱ�
	if(zbSoc_Heartbeat_Task_evInit(main_base_event_loop) != true)
	{
		log_debug("zbSoc_Heartbeat_Task_evInit failed\n");
		goto done;
	}

	//����������ʼ��
	if(zbSoc_Heartbeat_Uart_Report_start() != true)
	{
		log_debug("zbSoc_Heartbeat_Uart_Report_start failed\n");
		goto done;
	}

	//�豸�����ϱ���ʼ��
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

	//��ʾ��ǰ�����豸
	
#ifndef NDEBUG
	vdevListShowAllDeviceList();
#endif

#if PERMIMNG
	PermiMng_Init(main_base_event_loop);
#endif
	//��������Server
	if(tcp_server_evInit(main_base_event_loop)!=true)
	{
		log_debug("tcp_server_evInit failed\n");
		goto done;
	}
	
	//����Զ�̷�����
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
	
	//�������(libevent���)
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

