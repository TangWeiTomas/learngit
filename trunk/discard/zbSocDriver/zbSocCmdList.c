/***********************************************************************************
 * 文 件 名   : zbSocCmdList.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : 串口数据发送队列
 * 版权说明   : Copyright (c) 2008-2016   飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
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
 * 函 数 名  : zblist_getzbSocCmdList
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月21日
 * 函数功能  : 从重发列表中找到指定的设备
 * 输入参数  : uint16_t nwkAddr  设备短地址
               uint8_t endpoint  设备端口号
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

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
 * 函 数 名  : zblist_delzbSocCmdList
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月21日
 * 函数功能  : 删除重发节点信息
 * 输入参数  : zbSocCmdList_t *zblist  重发节点信息
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

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
 * 函 数 名  : zblist_Process_Handler_cb
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月21日
 * 函数功能  : 定时任务回调函数，处理设备节点的数据重发
 * 输入参数  : void *args  节点信息
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

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
	
	//发送命令
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
			
			uint8_t  deviceType = zblist->cmd[0];//产品类型
			uint16_t TableIndex = zblist->cmd[1]<<8|zblist->cmd[2];//型号索引ID
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

		    //组包
		    if(Create_Irc_Ctrl_Package(deviceType,TableIndex,DeviceKeyLength,DeviceKey,&senddata)==true)
		    {
		    	log_debug_array(senddata.data,senddata.idx,NULL);
		   	 	//发送
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
 * 函 数 名  : zblist_add
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月21日
 * 函数功能  : 将需要重发数据的节点信息添加到管理列表
 * 输入参数  : epInfo_t *epinfo  节点描述信息
               uint8_t *cmd        命令
               uint8_t cmdlen      命令长度
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
bool zblist_add(epInfo_t *epinfo,uint8_t *cmd,uint8_t cmdlen)
{
	int ret = -1;
	zbSocCmdList_t *zblist = NULL;
	uint64_t milliseconds = 0;
	log_debug("zblist_add++\n");
	
	if(epinfo == NULL)
		return false;

	//如果该命令已经在列表中拥有了，则不再添加新的，而是替换掉现有的指令
	zblist = zblist_getzbSocCmdList(epinfo->nwkAddr,epinfo->endpoint);
	
	if(zblist != NULL)
	{
		//重新设置状态
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
		//重新计数
		zblist->timeout = 0;
		
		return true;
	}

	zblist = malloc(sizeof(zbSocCmdList_t));
	if(zblist == NULL)
	{
		log_debug("malloc failed\n");
		return false;
	}

	//初始化定时器结构
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
	
	//注意场景，命令间的间隔
	ret = tu_set_evtimer_realtime(zblist->evtimer,milliseconds+successive_event_interval*200,true,(timer_handler_cb_t)zblist_Process_Handler_cb,(void *)zblist);
	if(ret)
	{
		log_debug("tu_set_evtimer_realtime failed\n");
		zblist_delzbSocCmdList(zblist);
		return false;
	}

	//将数据添加到列表中
	LIST_INSERT_HEAD(zbList,zblist,entry_);

	log_debug("zblist_add--\n");

	return true;
}

/*****************************************************************************
 * 函 数 名  : zblist_remove
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月21日
 * 函数功能  : 从设备管理列表中删除节点信息
 * 输入参数  : uint16_t nwkAddr  设备短地址
               uint8_t endpoint  设备端口号
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

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
 * 函 数 名  : zblist_evInit
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月21日
 * 函数功能  : 初始化重发管理列表
 * 输入参数  : struct event_base *base  调度队列
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

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

