/**************************************************************************************************
 * Filename:	   interface_vDeviceList.c
 * Author:			   edward
 * E-Mail:			ouxiangping@feixuekj.cn
 * Description:    在内存中管理设备信息
 *
 *	Copyright (C) 2016 feixue Company - http://www.feixuekj.cn/
 *
 * Version: 		1.00  (2016-03-16,10:03)	:	Create the file.
 *
 *
 *************************************************************************/
#include <stdbool.h>
#include "interface_vDeviceList.h"
#include "interface_devicelist.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>

#include "zbSocCmd.h"
#include "interface_devicelist.h"
#include "zbSocPrivate.h"
#include "interface_deviceStatelist.h"
#include "zbSocMasterControl.h"
#include "hwSyslog.h"
#include "GwComDef.h"

#if 0
typedef struct
{
	uint8_t  state;					//开关状态
	uint8_t  isEnable;				//是否被禁用
	uint8_t  cpercent;				//只用于窗帘百分比
	uint32_t BatteryValue ; 		//电量值

	/*智能插座属性*/
	uint32_t socketEleValue;		//电量
	uint32_t socketPower;			//功率
	uint32_t socketCurrent;			//电流
	uint32_t socketVoltage;			//电压

	/*温湿度检测*/
	uint16_t SenserTmp;				//温度
	uint16_t SenserHum;				//湿度

	/*空调控制器*/
	uint8_t  CentralAir[16];		//空调
	
}General_Device_Attribute_t;
#endif

typedef struct
{
	uint8_t state;				//开关状态
}General_Switch_Attribute_t;

typedef struct
{
	uint8_t state;					//开关状态 1:有人 0:无人
	uint8_t battery	;				//电量
	uint8_t batteryAlarm;		//电量报警
	uint8_t isEnable;				//是否使能
}General_Alarm_Attribute_t;

typedef struct
{
	uint16_t temperature;				//温度
	uint16_t humidity;					//湿度
	uint8_t battery	;						//电量
	uint8_t isEnable;						//是否使能
}General_Senser_Attribute_t;

typedef struct
{
	uint8_t state;				//窗帘状态
	uint8_t percent;			//百分比 位移
}General_Curtain_Attribute_t;

typedef struct
{
	uint8_t state;						//门锁状态
	uint8_t  doubleLockStu; 			//反锁状态
	uint8_t  batteryType	;			//电量类型
	uint8_t  alarm;						//报警
	uint8_t  batteryStu;				//门锁电压状态 00:电压正常 01:电压异常
	uint16_t battery	;				//电池容量
}General_Door_Attribute_t;

typedef struct
{
	uint8_t state;				//取电状态
	uint32_t battery	;		//电量
}General_PowerSwitch_Attribute_t;

typedef struct
{
	uint8_t  state;			//取电状态
	uint32_t battery;		//电量
	uint32_t power;			//功率
	uint32_t current;		//电流
	uint32_t voltage;		//电压
}General_SocketSwitch_Attribute_t;

#define CENTRAL_AIR_DATA_SIZE 	16

typedef struct
{
	uint8_t  state[CENTRAL_AIR_DATA_SIZE];		//温控器
	uint8_t	 idx;			//数据长度
}General_CentralAir_Attribute_t;

//智能电表
typedef struct
{
	
}General_EMeter_Attribute_t;

typedef struct vepInfo_t
{
	epInfo_t epInfo;
	void *data;
	TAILQ_ENTRY(vepInfo_t) entry_;
}vepInfo_t;

TAILQ_HEAD(vepInfoList, vepInfo_t) ;

static struct vepInfoList vDeviceListHead ;

#define VDEVLIST_FOREACH(OBJ) TAILQ_FOREACH(OBJ,&vDeviceListHead,entry_)

static vepInfo_t *vdevListMalloc(uint16_t deviceid)
{
	char *obj = NULL;
	
	vepInfo_t *epinfo = malloc(sizeof(vepInfo_t));
	if(epinfo == NULL)
	{
		log_err("malloc failed\n");
		return NULL;
	}
	
	memset(epinfo,0,sizeof(vepInfo_t));

	switch(deviceid){
	case ZB_DEV_WIN_CURTAIN: //窗帘
	case ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE:
	case ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE_FXKJ:
		obj = malloc(sizeof(General_Curtain_Attribute_t));
		if(obj != NULL)
			memset(obj,0,sizeof(General_Curtain_Attribute_t));
	break;
//	case ZB_DEV_MINI_SWITCH://随意贴
//		obj = malloc(sizeof(General_Curtain_Attribute_t));
//	break;
	case ZB_DEV_ONOFF_PLUG://插座
		obj = malloc(sizeof(General_SocketSwitch_Attribute_t));
		if(obj != NULL)
			memset(obj,0,sizeof(General_SocketSwitch_Attribute_t));
	break;
	case ZB_DEV_ONOFF_SWITCH://开关
		obj = malloc(sizeof(General_Switch_Attribute_t));
		if(obj != NULL)
			memset(obj,0,sizeof(General_Switch_Attribute_t));
	break;
	case ZB_DEV_SMOKE_SENSOR://烟雾报警器
	case ZB_DEV_INFRARED_BODY_SENSOR: //红外感应
	case ZB_DEV_DOOR_SENSOR://门磁感应
	case ZCL_HA_DEVICEID_IAS_ZONE:
		obj = malloc(sizeof(General_Alarm_Attribute_t));
		if(obj != NULL)
			memset(obj,0,sizeof(General_Alarm_Attribute_t));
	break;
	case ZB_DEV_LEVEL_DOORLOCK://门锁
	case ZB_DEV_ONOFF_DOORLOCK:
	case ZCL_HA_DEVICEID_FXZB_DOORLOCK_YAOTAI:
	case ZCL_HA_DEVICEID_FXZB_DOORLOCK_YGS:
	case ZCL_HA_DEVICEID_FXZB_DOORLOCK_JIULIAN:
	{
		General_Door_Attribute_t *dst  = malloc(sizeof(General_Door_Attribute_t));
		if(dst != NULL)
		{
			memset(dst,0,sizeof(General_Door_Attribute_t));
			dst->doubleLockStu = 0xFF;
			dst->battery = 0xFFFF;
			dst->batteryType = 0xff;
			dst->state = 0xFF;
			dst->alarm = 0xFF;
			dst->batteryStu = 0xFF;
		}
		obj = (char*)dst;
	}
	break;
	case ZB_DEV_POWER_SWITCH://取电开关
		obj = malloc(sizeof(General_PowerSwitch_Attribute_t));
		if(obj != NULL)
			memset(obj,0,sizeof(General_PowerSwitch_Attribute_t));
	break;
	case ZB_DEV_CENTRAL_AIR://温控器
		obj = malloc(sizeof(General_CentralAir_Attribute_t));
		if(obj != NULL)
			memset(obj,0,sizeof(General_CentralAir_Attribute_t));
	break;
	case ZB_DEV_TEMP_HUM://温湿度
		obj = malloc(sizeof(General_Senser_Attribute_t));
		if(obj != NULL)
			memset(obj,0,sizeof(General_Senser_Attribute_t));
	break;
	case ZB_DEV_METER_ENERGY_DEVICE:
		
	break;
	default:break;
	}

	epinfo->data = obj;
	
	return epinfo;
}


static void vdevListInsert(vepInfo_t *vepinfo)
{
	ASSERT(vepinfo != NULL);

	TAILQ_INSERT_TAIL(&vDeviceListHead,vepinfo,entry_);
}

static void vdevListRemove(vepInfo_t *vepinfo)
{
	ASSERT(vepinfo != NULL);

	TAILQ_REMOVE(&vDeviceListHead,vepinfo,entry_);
}

static vepInfo_t * vdevListScrechByNaEp(uint16_t nwkAddr, uint8_t endpoint)
{
	vepInfo_t *item = NULL;
	epInfo_t *epInfo = NULL;
	
	TAILQ_FOREACH(item, &vDeviceListHead, entry_)
	{
	   epInfo = &(item->epInfo);
	   if((nwkAddr == epInfo->nwkAddr)&&(endpoint==epInfo->endpoint))
	   {
			return item;
	   }
	}
	
	return NULL;
}


static void vdevListDeviceUpdateInfo(epInfo_t *dst,epInfo_t *src)
{
	ASSERT(dst != NULL && src != NULL);

	//memcpy(dst->IEEEAddr,src->IEEEAddr,8);
	
	dst->deviceID = src->deviceID;
	dst->endpoint = src->endpoint;
	dst->nwkAddr = src->nwkAddr;
	
	if(src->onlineDevRssi != 0)
		dst->onlineDevRssi = src->onlineDevRssi;

	dst->onlineflag = src->onlineflag;
	dst->onlineTimeoutCounter = src->onlineTimeoutCounter;
	dst->registerflag = src->registerflag;
}

static vepInfo_t * vdevListScrechByIeeeEp(uint8_t ieee[8],uint8_t endpoint)
{
	vepInfo_t *item = NULL;
	epInfo_t *epInfo = NULL;

	ASSERT(ieee!=NULL);
	
	TAILQ_FOREACH(item, &vDeviceListHead, entry_)
	{
		epInfo = &item->epInfo;
		if((memcmp(ieee,epInfo->IEEEAddr,8)==0)&&(endpoint==epInfo->endpoint))
		{
			return item;
		}
	}
	
	return NULL;
}

/*判断是否需要更新数据库文件*/
/*****************************************************************************
 * 函 数 名  : vdevListIsupgradeDatabase
 * 负 责 人  : Edward
 * 创建日期  : 2016年12月10日
 * 函数功能  : 判断是否需要更新数据库内容
 * 输入参数  : epInfo_t *dst  目标
               epInfo_t *src  源数据
 * 输出参数  : 无
 * 返 回 值  : static 0:不需要修改 1:需要修改 -1:不同对象能修改
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static int vdevListIsupgradeDatabase(epInfo_t *dst,epInfo_t *src)
{
	int ret = 0;

	ASSERT((dst = NULL) && (src != NULL));

	if(dst == NULL || src == NULL)
		return -1;

	if((memcmp(dst->IEEEAddr,src->IEEEAddr,8)!=0)&&(dst->endpoint != src->endpoint))
		return -1;

	/*当下列属性被修改后，才会重新更新数据库文件*/
	if(dst->nwkAddr != src->nwkAddr) ret = 1;
	if(dst->deviceID != src->deviceID) ret = 1;
	if(dst->onlineflag != src->onlineflag) ret = 1;
	if(dst->registerflag != src->registerflag) ret = 1;

	return ret;
}

static int vdevListUpgradeDatabase(epInfo_t *devInfo)
{
	ASSERT(devInfo != NULL);

	epInfo_t *epInfo = NULL;
	int ret = -1;

	if(devInfo == NULL)
		return -1;

	epInfo = devListGetDeviceByIeeeEp(devInfo->IEEEAddr,devInfo->endpoint);
	if(epInfo == NULL)
		return -1;
	
	ret = vdevListIsupgradeDatabase(devInfo,epInfo);

	/*更新*/
	if(ret > 0)
	{
		ret = devListModifyRecordByIeeeEp(devInfo->IEEEAddr,devInfo->endpoint,devInfo);
	}
	
	return ret;
}

epInfo_t * vdevListGetNextDev(uint32_t *context)
{

	uint32_t index = 1;
	
	vepInfo_t *vepInfo = NULL;
	(*context)++;
	
	TAILQ_FOREACH(vepInfo, &vDeviceListHead, entry_)  
	{
		if((*context)==index)
		{
			return &vepInfo->epInfo;
		}
		else
		{
			index++;
		}
	}

	return NULL;
}

epInfo_t * vdevListGetDeviceByNaEp(uint16_t nwkAddr, uint8_t endpoint)
{
	vepInfo_t *vepinfo = vdevListScrechByNaEp(nwkAddr,endpoint);

	if(vepinfo == NULL)
		return ((epInfo_t*)NULL);
		
	return &(vepinfo->epInfo);
}

epInfo_t *vdevListGetDeviceByIeeeEp(uint8_t ieee[8],uint8_t endpoint)
{
	vepInfo_t *vepinfo = vdevListScrechByIeeeEp(ieee,endpoint);
	if(vepinfo == NULL)
		return NULL;
		
	return &(vepinfo->epInfo);
}

void vdevListRemoveDeviceByIeeeEp(uint8_t ieee[8],uint8_t endpoint)
{
	vepInfo_t *item = NULL;
	
	log_debug("vdevListRemoveDeviceByIeeeEp++\n");

	do{
		item = vdevListScrechByIeeeEp(ieee,endpoint);
		if(item ==NULL)
			break;
		vdevListRemove(item);
		if(item->data != NULL)
			free(item->data);
		free(item);
	}while(0);
	
	log_debug("vdevListRemoveDeviceByIeeeEp--\n");
}

void vdevListRemoveDeviceByNaEp(uint16_t nwkAddr,uint8_t endpoint)
{
	vepInfo_t *item = NULL;
	
	log_debug("vdevListRemoveDeviceByIeeeEp++\n");

	do{
		item = vdevListScrechByNaEp(nwkAddr,endpoint);
		if(item ==NULL)
			break;
		vdevListRemove(item);
		if(item->data != NULL)
			free(item->data);
		free(item);
	}while(0);
	
	log_debug("vdevListRemoveDeviceByIeeeEp--\n");
}

void vdevListModifyByIeeeEp(uint8_t ieee[8],uint8_t endpoint,epInfo_t *epInfo)
{
	vepInfo_t *item = NULL;
	epInfo_t *vepInfo = NULL;

	do{
		item = vdevListScrechByIeeeEp(ieee,endpoint);
		if(item ==NULL)
			break;
		
		if(item->epInfo.deviceID != epInfo->deviceID)
		{
			log_err("epInfo deviceId is change\n");
			vdevListRemove(item);
			if(item->data != NULL)
				free(item->data);
			free(item);
			item = vdevListMalloc(epInfo->deviceID);
			if(item == NULL)
				break;
				
			memcpy(&(item->epInfo),epInfo,sizeof(epInfo_t));
			vdevListInsert(item);
			
		}else{
			vdevListDeviceUpdateInfo(&item->epInfo,epInfo);
		}
		
		//vdevListUpgradeDatabase()
	}while(0);
}

bool vdevListModifyByNaEp(uint16_t nwkAddr, uint8_t endpoint,epInfo_t * epInfo)
{
	vepInfo_t *item = NULL;
	do{
		item = vdevListScrechByNaEp(nwkAddr,endpoint);
		if(item ==NULL)
			break;

		if(item->epInfo.deviceID != epInfo->deviceID)
		{
			log_err("epInfo deviceId is change\n");
			vdevListRemove(item);
			if(item->data != NULL)
				free(item->data);
			free(item);
			item = vdevListMalloc(epInfo->deviceID);
			if(item == NULL)
				break;
				
			memcpy(&(item->epInfo),epInfo,sizeof(epInfo_t));
			vdevListInsert(item);
		}else{
			vdevListDeviceUpdateInfo(&item->epInfo,epInfo);
		}
	}while(0);
}
	
int vdevListAddDevice(epInfo_t *epinfo)
{
	vepInfo_t *vepInfo = NULL;
	
	ASSERT(epinfo != NULL);
	
	if((vdevListGetDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint))==NULL)
	{
		vepInfo= vdevListMalloc(epinfo->deviceID);
		if(vepInfo != NULL)
		{
			memcpy(&(vepInfo->epInfo),epinfo,sizeof(epInfo_t));
			
			vdevListInsert(vepInfo);
		}
	}
	else
	{
		vdevListModifyByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint,epinfo);
	}
	
	return 0;
}

void vdevListInit(void)
{
	uint32_t context = 0;
	epInfo_t *epinfo;
	
	log_debug("vdevListInit++\n");
	
	TAILQ_INIT(&vDeviceListHead);
	//将数据库中的所有的设备添加队列中
	
	while((epinfo = devListGetNextDev(&context))!= NULL)
	{
		if(epinfo->deviceID != 0x0 && epinfo->endpoint != 0x00)
			vdevListAddDevice(epinfo);
	}

	log_debug("vdevListInit--\n");
}

void vdevListShowAllDeviceList(void)
{
	uint32_t context = 0;
	int count = 0;
	epInfo_t *epInfo = NULL;
	//TAILQ_FOREACH(item, &vDeviceListHead, entry_)
	while((epInfo = vdevListGetNextDev(&context))!=NULL)
	{
		log_debug("context = %d\n",context);
		log_debug(
            "%d , %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X , 0x%04X , 0x%02X , 0x%04X , 0x%02X , 0x%02X , 0x%02X , 0x%04X\n",
            context,epInfo->IEEEAddr[0], epInfo->IEEEAddr[1], epInfo->IEEEAddr[2],
            epInfo->IEEEAddr[3], epInfo->IEEEAddr[4], epInfo->IEEEAddr[5],
            epInfo->IEEEAddr[6], epInfo->IEEEAddr[7], epInfo->nwkAddr,
            epInfo->endpoint,  epInfo->deviceID,epInfo->onlineDevRssi,
            epInfo->registerflag, epInfo->onlineflag,epInfo->onlineTimeoutCounter);
	}
}

//设置超时统计
int vdevListSetTimeOutCnt(void)
{
	uint8_t isTimeout = FALSE;
	vepInfo_t *item = NULL;
	epInfo_t *epInfo = NULL;
	
	log_debug("vdevListSetTimeOutCnt++\n");

	VDEVLIST_FOREACH(item)
	{
		epInfo = &(item->epInfo);
		
#if 0
		//门磁设备特殊处理
		if(vepInfo->deviceID == ZB_DEV_DOOR_SENSOR)
		{
			continue;
		}		
#endif	

		if(epInfo->deviceID == 0x0 || epInfo->endpoint == 0x0 || epInfo->nwkAddr == 0x0)
			continue;

		if(epInfo->onlineflag == true)
		{
			epInfo->onlineTimeoutCounter++;
		}

		//屏蔽红外人体感应设备
		if(epInfo->deviceID == ZCL_HA_DEVICEID_IAS_ZONE)
		{
			continue;
			//isTimeout = (epInfo->onlineTimeoutCounter >= MAX_ONLIEN_TIMEOUT_IAS_ZONE_CNT)?TRUE:FALSE;
		}
		else
		{
			isTimeout = (epInfo->onlineTimeoutCounter >= MAX_ONLINE_TIMEOUT_COUNTER)?TRUE:FALSE;
		}
		
		if(!isTimeout)
		{
			epInfo->onlineTimeoutCounter = 0;
			epInfo->onlineflag = false;
			devListModifyRecordByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint,epInfo);
			hwSyslog(epInfo->IEEEAddr,OffLine);
		}
	}
	
	log_debug("vdevListSetTimeOutCnt--\n");
}

uint8_t vdevListSetDevOnlineState(epInfo_t *devInfo,bool online)
{
	
	vepInfo_t *item = NULL;
	epInfo_t *vepInfo = NULL;
	uint8_t ret= 0;

	ASSERT(devInfo != NULL);
	
	log_debug("vdevListSetDevOnlineState++\n");

	VDEVLIST_FOREACH(item)
	{
		vepInfo = &(item->epInfo);
		if(memcmp(vepInfo->IEEEAddr,devInfo->IEEEAddr,8) == 0)
		{	
			vepInfo->onlineTimeoutCounter = 0;
			vepInfo->onlineDevRssi = devInfo->onlineDevRssi;

			if(vepInfo->onlineflag != online)
			{
				ret = 1;
				vepInfo->onlineflag = online;

				hwSyslog(vepInfo->IEEEAddr,online);

				//更新文件
				devListModifyRecordByIeeeEp(vepInfo->IEEEAddr,vepInfo->endpoint,vepInfo);
			}
		}
	}
	
	log_debug("vdevListSetDevOnlineState--\n");
	return ret;
}

//获取在线的设备
uint8_t vdevListGetAllRegOnlineDev(uint8_t *buf,uint16_t *bufsize)
{
	vepInfo_t *item = NULL;
	
    hostCmd cmd;
    cmd.idx = 0;

    uint8_t devCount=0;

   	log_debug("vdevListGetAllRegOnlineDev++\n");

	VDEVLIST_FOREACH(item)
	{
		if(item->epInfo.deviceID == 0x0 || item->epInfo.endpoint == 0x0 || item->epInfo.nwkAddr == 0x0)
			continue;
			
#if USE_MASTER_CONTROL
		//屏蔽主控设备
		if(item->epInfo.deviceID == ZB_DEV_MASTER_CONTROL)
			continue;
#endif
		//屏蔽过滤的和不在线的
		if((item->epInfo.registerflag == false) || (item->epInfo.onlineflag == false))
			continue;

		devCount++;
		cmdSetStringVal(&cmd, item->epInfo.IEEEAddr, 8);
		cmdSet8bitVal(&cmd,item->epInfo.endpoint);
		cmdSet16bitVal(&cmd, item->epInfo.deviceID);
		cmdSet8bitVal(&cmd, item->epInfo.onlineDevRssi);

		log_debug("IEEE:");
		log_debug_array(item->epInfo.IEEEAddr,8,":");
		log_debug("Endpoint:%02x RSSI:%d\n",item->epInfo.endpoint,item->epInfo.onlineDevRssi);
	}
	
	*bufsize = cmd.idx;
    memcpy(buf,cmd.data,cmd.idx);
    
    log_debug("vdevListGetAllRegOnlineDev--\n");
    return devCount;
}

//获取屏蔽的断开设备
uint8_t vdevListGetAllUnRegOnlineDev(uint8_t *buf,uint16_t *bufsize)
{
	vepInfo_t *item = NULL;
	
    hostCmd cmd;
    cmd.idx = 0;

    uint8_t devCount=0;

   	log_debug("vdevListGetAllUnRegOnlineDev++\n");

	VDEVLIST_FOREACH(item)
	{
		if(item->epInfo.deviceID == 0x0 || item->epInfo.endpoint == 0x0 || item->epInfo.nwkAddr == 0x0)
			continue;
			
#if USE_MASTER_CONTROL
		//屏蔽主控设备
		if(item->epInfo.deviceID == ZB_DEV_MASTER_CONTROL)
			continue;
#endif
		//屏蔽过滤的和不在线的
		if((item->epInfo.registerflag == true) || (item->epInfo.onlineflag == false))
			continue;

		devCount++;
		cmdSetStringVal(&cmd, item->epInfo.IEEEAddr, 8);
		cmdSet8bitVal(&cmd,item->epInfo.endpoint);
		cmdSet16bitVal(&cmd, item->epInfo.deviceID);
		
		log_debug("IEEE:");
		log_debug_array(item->epInfo.IEEEAddr,8,":");
		log_debug("Endpoint:%02x\n",item->epInfo.endpoint);
	}

	*bufsize = cmd.idx;

    memcpy(buf,cmd.data,cmd.idx);
    
    log_debug("vdevListGetAllUnRegOnlineDev--\n");
    return devCount;
}

void vdevListUpdateDeviceRssi(epInfo_t *epinfo,int8_t rssi)
{
	vepInfo_t *item = NULL;

	if(epinfo == NULL)
		return;

	VDEVLIST_FOREACH(item)
	{
		if(memcmp(epinfo->IEEEAddr,item->epInfo.IEEEAddr,8)==0)
		{
			item->epInfo.onlineDevRssi = rssi;
		}
	}
}
/*设备操作封装*/
/*============================================================*/
void vdevListSetCurtainState(epInfo_t *epinfo,uint8_t state)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;
	
	((General_Curtain_Attribute_t*)vepinfo->data)->state = state;
}

uint8_t vdevListGetCurtainState(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Curtain_Attribute_t*)vepinfo->data)->state;
}

uint8_t vdevListGetCurtainPercent(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Curtain_Attribute_t*)vepinfo->data)->percent;
}

void vdevListSetCurtainPercent(epInfo_t *epinfo,uint8_t percent)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Curtain_Attribute_t*)vepinfo->data)->percent = percent;
}
/*============================================================*/
void vdevListSetSocketState(epInfo_t *epinfo,uint8_t state)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_SocketSwitch_Attribute_t*)vepinfo->data)->state = state;
}

uint8_t vdevListGetSocketState(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_SocketSwitch_Attribute_t*)vepinfo->data)->state;
}

void vdevListSetSocketBattery(epInfo_t *epinfo,uint32_t battery)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_SocketSwitch_Attribute_t*)vepinfo->data)->battery = battery;
}

uint32_t vdevListGetSocketBattery(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_SocketSwitch_Attribute_t*)vepinfo->data)->battery;
}

void vdevListSetSocketPower(epInfo_t *epinfo,uint32_t power)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_SocketSwitch_Attribute_t*)vepinfo->data)->power = power;
}

uint32_t vdevListGetSocketPower(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_SocketSwitch_Attribute_t*)vepinfo->data)->power;
}

void vdevListSetSocketCurrent(epInfo_t *epinfo,uint32_t Current)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_SocketSwitch_Attribute_t*)vepinfo->data)->current = Current;
}

uint32_t vdevListGetSocketCurrent(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_SocketSwitch_Attribute_t*)vepinfo->data)->current;
}

void vdevListSetSocketVoltage(epInfo_t *epinfo,uint32_t voltage)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_SocketSwitch_Attribute_t*)vepinfo->data)->voltage = voltage;
}

uint32_t vdevListGetSocketVoltage(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_SocketSwitch_Attribute_t*)vepinfo->data)->voltage;
}

/*============================================================*/
void vdevListSetSwitchState(epInfo_t *epinfo,uint8_t state)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Switch_Attribute_t*)vepinfo->data)->state = state;
}

uint8_t vdevListGetSwitchState(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Switch_Attribute_t*)vepinfo->data)->state;
}
/*============================================================*/
void vdevListSetAlarmState(epInfo_t *epinfo,uint8_t state)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Alarm_Attribute_t*)vepinfo->data)->state = state;
}

uint8_t vdevListGetAlarmState(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Alarm_Attribute_t*)vepinfo->data)->state;
}

void vdevListSetAlarmBattery(epInfo_t *epinfo,uint8_t battery)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Alarm_Attribute_t*)vepinfo->data)->battery = battery;
}

uint8_t vdevListGetAlarmBattery(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return 0;

	if(vepinfo->data == NULL)
		return 0;
	
	return ((General_Alarm_Attribute_t*)vepinfo->data)->battery;
}

void vdevListSetAlarmBatteryAlarm(epInfo_t *epinfo,uint8_t alarm)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Alarm_Attribute_t*)vepinfo->data)->batteryAlarm = alarm;
}

uint8_t vdevListGetAlarmBatteryAlarm(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return 0;

	if(vepinfo->data == NULL)
		return 0;
	
	return ((General_Alarm_Attribute_t*)vepinfo->data)->batteryAlarm;
}

void vdevListSetAlarmisEnable(epInfo_t *epinfo,uint8_t isEnable)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Alarm_Attribute_t*)vepinfo->data)->isEnable = isEnable;
}

uint8_t vdevListGetAlarmisEnable(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Alarm_Attribute_t*)vepinfo->data)->isEnable;
}

/*============================================================*/
void vdevListSetDoorState(epInfo_t *epinfo,uint8_t state)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Door_Attribute_t*)vepinfo->data)->state = state;
}

uint8_t vdevListGetDoorState(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Door_Attribute_t*)vepinfo->data)->state;
}

void vdevListSetDoorBattery(epInfo_t *epinfo,uint8_t batteryType,uint16_t battery)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	if(((General_Door_Attribute_t*)vepinfo->data)->batteryType == 0XFF)
	{
		((General_Door_Attribute_t*)vepinfo->data)->batteryType = 1<<batteryType;
	}
	else
	{
		((General_Door_Attribute_t*)vepinfo->data)->batteryType |= 1<<batteryType;
	}

	//((General_Door_Attribute_t*)vepinfo->data)->batteryType = batteryType;
	
	((General_Door_Attribute_t*)vepinfo->data)->battery = battery;
}

uint16_t vdevListGetDoorBattery(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return 0;

	if(vepinfo->data == NULL)
		return 0;
	
	return ((General_Door_Attribute_t*)vepinfo->data)->battery;
}

uint8_t vdevListGetDoorBatteryStu(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return 0;

	if(vepinfo->data == NULL)
		return 0;
	
	return ((General_Door_Attribute_t*)vepinfo->data)->batteryStu;
}

void vdevListSetDoorBatteryStu(epInfo_t *epinfo,uint8_t batteryType,uint8_t batteryStu)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	if(((General_Door_Attribute_t*)vepinfo->data)->batteryType == 0XFF)
	{
		((General_Door_Attribute_t*)vepinfo->data)->batteryType = 1<<batteryType;
	}
	else
	{
		((General_Door_Attribute_t*)vepinfo->data)->batteryType |= 1<<batteryType;
	}

	//((General_Door_Attribute_t*)vepinfo->data)->batteryType = batteryType;

	((General_Door_Attribute_t*)vepinfo->data)->batteryStu = batteryStu;
}

uint8_t vdevListGetDoorBatteryType(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return 0;

	if(vepinfo->data == NULL)
		return 0;
	
	return ((General_Door_Attribute_t*)vepinfo->data)->batteryType;
}

uint8_t vdevListGetDoorAlarm(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return 0;

	if(vepinfo->data == NULL)
		return 0;
	
	return ((General_Door_Attribute_t*)vepinfo->data)->alarm;
}

void vdevListSetDoorAlarm(epInfo_t *epinfo,uint8_t alarm)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Door_Attribute_t*)vepinfo->data)->alarm = alarm;
}

void vdevListSetDoorDLockStu(epInfo_t *epinfo,uint8_t DLockStu)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Door_Attribute_t*)vepinfo->data)->doubleLockStu = DLockStu;
}

uint8_t vdevListGetDoorDLockStu(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return 0;

	if(vepinfo->data == NULL)
		return 0;
	
	return ((General_Door_Attribute_t*)vepinfo->data)->doubleLockStu;
}


/*============================================================*/
void vdevListSetPowerSwitchState(epInfo_t *epinfo,uint8_t state)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_PowerSwitch_Attribute_t*)vepinfo->data)->state = state;
}

int8_t vdevListGetPowerSwitchState(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_PowerSwitch_Attribute_t*)vepinfo->data)->state;
}


void vdevListSetPowerSwitchBattery(epInfo_t *epinfo,uint32_t battery)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_PowerSwitch_Attribute_t*)vepinfo->data)->battery = battery;
}

uint32_t vdevListGetPowerSwitchBattery(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return 0;

	if(vepinfo->data == NULL)
		return 0;
	
	return ((General_PowerSwitch_Attribute_t*)vepinfo->data)->battery;
}


/*============================================================*/
void vdevListSetCentralAirState(epInfo_t *epinfo,uint8_t *buf,uint8_t len)
{
	ASSERT(epinfo != NULL);

	log_debug("vdevListSetCentralAirState++\n");
	
	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
	{
		log_debug("vepinfo object is NULL\n");
		return;
	}
	
	if(vepinfo->data == NULL)
	{
		log_debug("data object is NULL\n");
		return;
	}

	if(len > CENTRAL_AIR_DATA_SIZE)
		return ;

	memcpy(((General_CentralAir_Attribute_t*)vepinfo->data)->state,buf,len);

	((General_CentralAir_Attribute_t*)vepinfo->data)->idx = len;
	
	log_debug_array(buf,len,NULL);
	
	log_debug("vdevListSetCentralAirState--\n");
	
	//((General_CentralAir_Attribute_t*)vepinfo->data).state = battery;
}

int8_t vdevListGetCentralAirState(epInfo_t *epinfo,uint8_t *buf,uint8_t len)
{
	ASSERT(epinfo != NULL);

	uint8_t length = 0;

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;

	length = ((General_CentralAir_Attribute_t*)vepinfo->data)->idx;
	
	if((len > CENTRAL_AIR_DATA_SIZE)||(length < len))
		return -1;
		
	memcpy(buf,((General_CentralAir_Attribute_t*)vepinfo->data)->state,len);

	return len;
}

int8_t vdevListGetCentralAirIdx(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_CentralAir_Attribute_t*)vepinfo->data)->idx;
}

/*============================================================*/

void vdevListSetSensertemp(epInfo_t *epinfo,uint16_t temperature)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Senser_Attribute_t*)vepinfo->data)->temperature = temperature;
}

uint16_t vdevListGetSensertemp(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Senser_Attribute_t*)vepinfo->data)->temperature;
}


void vdevListSetSenserHumidity(epInfo_t *epinfo,uint16_t humidity)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Senser_Attribute_t*)vepinfo->data)->humidity = humidity;
}

uint16_t vdevListGetSenserHumidity(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Senser_Attribute_t*)vepinfo->data)->humidity;
}

void vdevListSetSenserBattery(epInfo_t *epinfo,uint16_t battery)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Senser_Attribute_t*)vepinfo->data)->battery = battery;
}

uint8_t vdevListGetSenserBattery(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Senser_Attribute_t*)vepinfo->data)->battery;
}

void vdevListSetSenserisEnable(epInfo_t *epinfo,uint16_t isEnable)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return;

	if(vepinfo->data == NULL)
		return;

	((General_Senser_Attribute_t*)vepinfo->data)->isEnable = isEnable;
}

uint8_t vdevListGetSenserisEnable(epInfo_t *epinfo)
{
	ASSERT(epinfo != NULL);

	vepInfo_t *vepinfo = container_of(epinfo,vepInfo_t,epInfo);
	if(vepinfo==NULL)
		return -1;

	if(vepinfo->data == NULL)
		return -1;
	
	return ((General_Senser_Attribute_t*)vepinfo->data)->isEnable;
}

/*============================================================*/
