/***********************************************************************************
 * �� �� ��   : zbSocCmdList.c
 * �� �� ��   : Edward
 * ��������   : 2016��7��19��
 * �ļ�����   : �������ݷ��Ͷ���
 * ��Ȩ˵��   : Copyright (c) 2008-2016   ��ѩ����Ƽ����޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Types.h"
#include "queue.h"
#include "logUtils.h"
#include "DeviceCode.h"
#include "Timer_utils.h"
#include "interface_devicelist.h"
#include "zbSocCmdList.h"
#include "SuccessiveEvents.h"
#include "zbSocPrivate.h"


typedef struct zbSocCmdList_t
{
	uint8_t  endpoint;
	uint8_t  timeout;
	uint16_t nwkAddr;
	uint16_t deviceid;
	uint8_t *cmd;
	uint8_t  cmdlen;
	tu_evtimer_t *evtimer;
    LIST_ENTRY(zbSocCmdList_t) entry_;
} zbSocCmdList_t;

LIST_HEAD(zbSocCmdList, zbSocCmdList_t) ;

static struct event_base *evbase = NULL;
static struct zbSocCmdList *zbList = NULL;

bool zblist_remove(uint16_t nwkAddr,uint8_t endpoint);

/*****************************************************************************
 * �� �� ��  : zblist_getzbSocCmdList
 * �� �� ��  : Edward
 * ��������  : 2016��7��21��
 * ��������  : ���ط��б����ҵ�ָ�����豸
 * �������  : uint16_t nwkAddr  �豸�̵�ַ
               uint8_t endpoint  �豸�˿ں�
 * �������  : ��
 * �� �� ֵ  : static
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static zbSocCmdList_t * zblist_getzbSocCmdList(uint16_t nwkAddr,uint8_t endpoint)
{
	zbSocCmdList_t *zblist = NULL;

	log_debug("zblist_getzbSocCmdList++\n");

	log_debug("nwk:0x%x endpoint:%x\n",nwkAddr,endpoint);
		
	LIST_FOREACH(zblist,zbList,entry_)
	{
		log_debug("nwk:0x%x endpoint:0x%x deviceid:0x%x\n",zblist->nwkAddr,zblist->endpoint,zblist->deviceid);
		if((zblist->nwkAddr == nwkAddr) && (zblist->endpoint == endpoint))
		{
			log_debug("Find Device\n");
			return zblist;
		}
	}

	log_debug("zblist_getzbSocCmdList--\n");

	return NULL;
}

/*****************************************************************************
 * �� �� ��  : zblist_delzbSocCmdList
 * �� �� ��  : Edward
 * ��������  : 2016��7��21��
 * ��������  : ɾ���ط��ڵ���Ϣ
 * �������  : zbSocCmdList_t *zblist  �ط��ڵ���Ϣ
 * �������  : ��
 * �� �� ֵ  : static
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static void zblist_delzbSocCmdList(zbSocCmdList_t *zblist)
{
	if(zblist == NULL)
		return ;

	tu_kill_evtimer(zblist->evtimer);
	
	if(zblist->evtimer!=NULL)
		free(zblist->evtimer);
		
	if(zblist->cmd!=NULL)
		free(zblist->cmd);
		
	if(zblist!=NULL)
		free(zblist);
	return;
}

/*****************************************************************************
 * �� �� ��  : zblist_Process_Handler_cb
 * �� �� ��  : Edward
 * ��������  : 2016��7��21��
 * ��������  : ��ʱ����ص������������豸�ڵ�������ط�
 * �������  : void *args  �ڵ���Ϣ
 * �������  : ��
 * �� �� ֵ  : static
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static void zblist_Process_Handler_cb(void *args)
{
	epInfo_t *epInfo = NULL;
	zbSocCmdList_t *tmpzblist = NULL;
	zbSocCmdList_t *zblist = args;

	log_debug("zblist_Process_Handler_cb++\n");

	if(zblist->timeout >= MAX_ONLINE_TIMEOUT_COUNTER)
	{
		log_debug("Device TimeOut\n");
		zblist_remove(zblist->nwkAddr,zblist->endpoint);
		return;
	}
	else
	{
		zblist->timeout++;
	}

	epInfo = devListGetDeviceByNaEp(zblist->nwkAddr,zblist->endpoint);
	if(epInfo == NULL)
	{
		log_debug("NO FOUNT DEVICE\n");
		zblist_remove(zblist->nwkAddr,zblist->endpoint);
		return;
	}
	
	//��������
	switch(zblist->deviceid)
	{
		case ZB_DEV_ONOFF_DOORLOCK:
			zbSoc_SetGenOnOffState(zblist->cmd[zblist->cmdlen - 1],epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		break;
		
#if DEVICE_LIWEI_DOOR_SUPPERT
		case ZB_DEV_LEVEL_DOORLOCK:
			doorLevel_setOnOffReq(epInfo,zblist->cmd[zblist->cmdlen - 1]);
		break;
#endif

		case ZB_DEV_ONOFF_PLUG:
//				zbSoc_QuerySwitchSocketValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,zblist->cmd[zblist->cmdlen - 1]);
//			break;
		case ZB_DEV_POWER_SWITCH:
		case ZB_DEV_ONOFF_SWITCH:
		{
			#if USE_MASTER_CONTROL
				zbSoc_MasterControlSetOnOffState(epInfo,zblist->cmd[zblist->cmdlen - 1]);
			#else
				zbSoc_SetGenOnOffState(zblist->cmd[zblist->cmdlen - 1],epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
			#endif
		}		
		break;
		case ZB_DEV_INFRARED_BODY_SENSOR:
				zbSoc_SetDevValidReq(zblist->cmd[zblist->cmdlen - 1],epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
			break;
		case ZB_DEV_CENTRAL_AIR:
			
			break;
		case ZB_DEV_WIN_CURTAIN:
			zbSoc_CurtainDevCtrlCmdReq(zblist->cmd[zblist->cmdlen - 1],epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		break;
		case ZB_DEV_IRC_LEARN_CTRL:
		{
			log_debug("ZB_DEV_IRC_LEARN_CTRL\n");
			
			uint8_t  deviceType = zblist->cmd[0];//��Ʒ����
			uint16_t TableIndex = zblist->cmd[1]<<8|zblist->cmd[2];//�ͺ�����ID
			uint8_t  DeviceKeyLength = zblist->cmd[3];
			uint8_t *DeviceKey = NULL;
			uint8_t key = 0;;
			
			if((deviceType == IRC_DEVICE_TYPE_STB)||(deviceType == IRC_DEVICE_TYPE_IPTV)||(deviceType == IRC_DEVICE_TYPE_TV))
			{
				key = GetDevicePowerKey(deviceType);
				DeviceKey = &key;
			}
			else
			{
				DeviceKey = &zblist->cmd[4];
			}
			
		    hostCmd senddata;

			log_debug_array(zblist->cmd,zblist->cmdlen,NULL);

		    //���
		    if(Create_Irc_Ctrl_Package(deviceType,TableIndex,DeviceKeyLength,DeviceKey,&senddata)==true)
		    {
		    	log_debug_array(senddata.data,senddata.idx,NULL);
		   	 	//����
		    	zbSoc_IrcRemoteDevCtrlCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,senddata.idx,senddata.data);
			}
		}
		default:
			log_debug("unsuppert device %d\n",zblist->deviceid);
			zblist_remove(zblist->nwkAddr,zblist->endpoint);
		break;
	}
	log_debug("zblist_Process_Handler_cb--\n");

}

/*****************************************************************************
 * �� �� ��  : zblist_add
 * �� �� ��  : Edward
 * ��������  : 2016��7��21��
 * ��������  : ����Ҫ�ط����ݵĽڵ���Ϣ��ӵ������б�
 * �������  : epInfo_t *epinfo  �ڵ�������Ϣ
               uint8_t *cmd        ����
               uint8_t cmdlen      �����
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
bool zblist_add(epInfo_t *epinfo,uint8_t *cmd,uint8_t cmdlen)
{
	int ret = -1;
	zbSocCmdList_t *zblist = NULL;
	uint64_t milliseconds = 0;
	log_debug("zblist_add++\n");
	
	if(epinfo == NULL)
		return false;

	//����������Ѿ����б���ӵ���ˣ���������µģ������滻�����е�ָ��
	zblist = zblist_getzbSocCmdList(epinfo->nwkAddr,epinfo->endpoint);
	
	if(zblist != NULL)
	{
		//��������״̬
		if(zblist->cmdlen > cmdlen)
		{
			free(zblist->cmd);
			zblist->cmd = malloc(sizeof(uint8_t)*cmdlen);
			if(zblist->cmd == NULL)
			{
				log_debug("malloc failed\n");
				return false;
			}
			memset(zblist->cmd,0,sizeof(uint8_t)*cmdlen);
		}
		
		memcpy(zblist->cmd,cmd,cmdlen);
		zblist->cmdlen = cmdlen;
		//���¼���
		zblist->timeout = 0;
		
		return true;
	}

	zblist = malloc(sizeof(zbSocCmdList_t));
	if(zblist == NULL)
	{
		log_debug("malloc failed\n");
		return false;
	}

	//��ʼ����ʱ���ṹ
	zblist->evtimer  = tu_evtimer_new(evbase);
	if(zblist->evtimer == NULL)
	{
		log_debug("tu_evtimer_new failed\n");
		zblist_delzbSocCmdList(zblist);
		return false;
	}

	zblist->deviceid = epinfo->deviceID;
	zblist->endpoint = epinfo->endpoint;
	zblist->nwkAddr  = epinfo->nwkAddr;
	zblist->cmdlen   = cmdlen;
	zblist->timeout  = 0;

	log_debug("nwk:0x%x endpoint:0x%x deviceid:0x%x\n",zblist->nwkAddr,zblist->endpoint,zblist->deviceid);
	
	zblist->cmd		 = malloc(sizeof(uint8_t)*cmdlen);
	if(zblist->cmd == NULL)
	{
		log_debug("tu_evtimer_new failed\n");
		zblist_delzbSocCmdList(zblist);
		return false;
	}

	memcpy(zblist->cmd,cmd,cmdlen);
	
#if DEVICE_LIWEI_DOOR_SUPPERT

	if(epinfo->deviceID == ZB_DEV_LEVEL_DOORLOCK)
	{
		milliseconds = DEVICE_LIWEI_DOOR_QUERY_TIME;
	}
	else
	{
		milliseconds = DEVICE_CMD_RESEND_INTERVAL_TIME;
	}
	
#else
	milliseconds = DEVICE_CMD_RESEND_INTERVAL_TIME;
#endif
	
	//ע�ⳡ���������ļ��
	ret = tu_set_evtimer_realtime(zblist->evtimer,milliseconds+successive_event_interval*200,true,(timer_handler_cb_t)zblist_Process_Handler_cb,(void *)zblist);
	if(ret)
	{
		log_debug("tu_set_evtimer_realtime failed\n");
		zblist_delzbSocCmdList(zblist);
		return false;
	}

	//��������ӵ��б���
	LIST_INSERT_HEAD(zbList,zblist,entry_);

	log_debug("zblist_add--\n");

	return true;
}

/*****************************************************************************
 * �� �� ��  : zblist_remove
 * �� �� ��  : Edward
 * ��������  : 2016��7��21��
 * ��������  : ���豸�����б���ɾ���ڵ���Ϣ
 * �������  : uint16_t nwkAddr  �豸�̵�ַ
               uint8_t endpoint  �豸�˿ں�
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
bool zblist_remove(uint16_t nwkAddr,uint8_t endpoint)
{
	zbSocCmdList_t *zblist = NULL;

	log_debug("zblist_remove++\n");
	
	zblist = zblist_getzbSocCmdList(nwkAddr,endpoint);
	if(zblist==NULL)
	{
		log_debug("NO FOUND DEVICE\n");
		return true;
	}

	LIST_REMOVE(zblist,entry_);

	zblist_delzbSocCmdList(zblist);

	log_debug("zblist_remove--\n");

	return true;
}

/*****************************************************************************
 * �� �� ��  : zblist_evInit
 * �� �� ��  : Edward
 * ��������  : 2016��7��21��
 * ��������  : ��ʼ���ط������б�
 * �������  : struct event_base *base  ���ȶ���
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
bool zblist_evInit(struct event_base *base)
{
	if(zbList == NULL)
	{
		zbList = malloc(sizeof(zbSocCmdList_t));
		if(zbList == NULL)
		{
			log_debug("malloc failed\n");
			return false;
		}

		memset(zbList,0,sizeof(zbSocCmdList_t));
	}
	
	LIST_INIT(zbList);
	evbase = base;
	return true;
}

