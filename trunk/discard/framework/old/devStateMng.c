/**************************************************************************************************
 * Filename:       devStateMng.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-02,21:05)    :   Create the file.
 *
 *
 *************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>

#include "devStateMng.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "interface_deviceStatelist.h"
#include "interface_vDeviceList.h"


/*********************************************************************
 * MACROS
 */



/*********************************************************************
 * CONSTANTS
 */




/************************************************************
 * TYPEDEFS
 */



/*********************************************************************
 * GLOBAL VARIABLES
 */




/*********************************************************************
 * LOCAL VARIABLES
 */



/*********************************************************************
 * LOCAL FUNCTIONS
 */

/************************************************************************
* 函数名 :devState_updateSwitchVal(epInfo_t *epInfo,uint8 onoff)
* 描述   :    更新开关设备的状态值
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void devState_updateSwitchVal(epInfo_t *epInfo,uint8 onoff)
{
	if(epInfo == NULL)
		return;
#if 0
	devState_t epState={{0},0,0,{0},0};
	memcpy(epState.IEEEAddr, epInfo->IEEEAddr, 8);
	epState.endpoint = epInfo->endpoint;
	epState.length = 1;
	epState.deviceID = epInfo->deviceID;
	memcpy(epState.dataSegment, &onoff, epState.length);
	
	log_debug("devState_updateSwitchVal++\n");
	
    devStateListModifyRecordByIeeeEp(&epState);
#else
	epInfo_t *item = NULL;
	item = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
	if ( item != NULL )
	{
//		vdevListSetState(item,onoff);
	}

	//更新rssi
	if((item->onlineDevRssi!=epInfo->onlineDevRssi)&&(epInfo->onlineDevRssi!=0))
		item->onlineDevRssi = epInfo->onlineDevRssi;
		
#endif
}


int devState_getSwitchValue(epInfo_t *epInfo)
{
	epInfo_t *item = NULL;
	item = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
	if ( item != NULL )
	{
		//item->length = 0x01;
		//memcpy(item->dataSegment, &onoff,item->length);
//		return vdevListGetState(item);
	}

	return -1;
}

/************************************************************************
* 函数名 :devState_updateDoorLockVal(epInfo_t *epInfo,uint32 date)
* 描述   :   更新门锁的当前电量值
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
	
int devState_updateevicePowerValue(epInfo_t *epInfo,uint32 value)
{
	epInfo_t *epinfo = NULL;
	uint8 mloop = 0;
	epinfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
	if(epinfo==NULL)
		return -1;

//	vdevListSetBattery(epinfo,value);
	
	//更新rssi
	if((epinfo->onlineDevRssi!=epInfo->onlineDevRssi)&&(epInfo->onlineDevRssi!=0))
		epinfo->onlineDevRssi = epInfo->onlineDevRssi;
}

void devState_updateDoorLockVal(epInfo_t *epInfo,uint8 date)
{
	if(epInfo == NULL)
		return;
#if 0
	devState_t epState={{0},0,0,{0},0};
	memcpy(epState.IEEEAddr, epInfo->IEEEAddr, 8);
	epState.endpoint = epInfo->endpoint;
	epState.length = 1;
	epState.deviceID = epInfo->deviceID;
	memcpy(epState.dataSegment, &date, epState.length);

	log_debug("devState_updateDoorLockVal++\n");

    devStateListModifyRecordByIeeeEp(&epState);
#else
	epInfo_t *item = NULL;
	item = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
	if ( item != NULL )
	{
//		vdevListSetState(item,date);
	}
	if((item->onlineDevRssi!=epInfo->onlineDevRssi)&&(epInfo->onlineDevRssi!=0))
		item->onlineDevRssi = epInfo->onlineDevRssi;
#endif
}


/************************************************************************
* 函数名 :devState_updateHumitureVal(epInfo_t *epInfo,uint16 temp,uint16 hum)
* 描述   :    更新温度和湿度值
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void devState_updateHumitureVal(epInfo_t *epInfo,uint16 temp,uint16 hum)
{
	if(epInfo == NULL)
		return;
#if 0
    devState_t epState={{0},0,0,{0},0};
	memcpy(epState.IEEEAddr, epInfo->IEEEAddr, 8);
	epState.endpoint = epInfo->endpoint;
	epState.length = 4;
	epState.deviceID = epInfo->deviceID;
	memcpy(&epState.dataSegment[0], &temp, 2);
	memcpy(&epState.dataSegment[2], &hum, 2);

#ifdef DEBUG
    printf("FuncName:%s \n",__FUNCTION__);
#endif

    devStateListModifyRecordByIeeeEp(&epState);
#else
	epInfo_t *item = NULL;
	item = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
	if ( item != NULL )
	{
//		vdevListSetSenserHum(item,hum);
//		vdevListSetSenserTmp(item,temp);
	}
#endif
}

/************************************************************************
* 函数名 :devState_getHumitureVal(epInfo_t *epInfo,uint16 *temp,uint16 *hum)
* 描述   :    获取温度、湿度
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
bool devState_getHumitureVal(uint8* ieeeAddr,uint8 endpoint,uint16 *temp,uint16 *hum)
{
	
#if 0
	devState_t *epState;

	log_debug("devState_getHumitureVal++\n");
	
    if((epState = devStateListGetRecordByIeeeEp(ieeeAddr,endpoint))!=NULL)
    {
		*temp = (epState->dataSegment[0]<<8)|(epState->dataSegment[1]);
		*hum  = (epState->dataSegment[2]<<8)|(epState->dataSegment[3]);
        return true;
    }
  	log_debug("devState_getHumitureVal--\n");
    return false;
#else
	log_debug("devState_getHumitureVal++\n");

	epInfo_t *item = NULL;
	item = vdevListGetDeviceByIeeeEp(ieeeAddr,endpoint);
	if ( item != NULL )
	{
		*temp = vdevListGetSensertemp(item);
		*hum  = vdevListGetSenserHumidity(item);
        return true;
	}
	log_debug("devState_getHumitureVal--\n");
	return false;
#endif
}


/************************************************************************
* 函数名 :devState_updateHumitureLightVal(epInfo_t *epInfo,uint16 temp,uint16 hum,uint16 light)
* 描述   :    更新温度、湿度、光照值
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void devState_updateHumitureLightVal(epInfo_t *epInfo,uint16 temp,uint16 hum,uint16 light)
{
	if(epInfo == NULL)
		return;
#if 0
	devState_t epState={{0},0,0,{0},0};
	memcpy(epState.IEEEAddr, epInfo->IEEEAddr, 8);
	epState.endpoint = epInfo->endpoint;
	epState.length = 6;
	epState.deviceID = epInfo->deviceID;
	memcpy(&epState.dataSegment[0], &temp, 2);
	memcpy(&epState.dataSegment[2], &hum, 2);
	memcpy(&epState.dataSegment[4], &light, 2);
	log_debug("devState_updateHumitureLightVal++\n");
    devStateListModifyRecordByIeeeEp(&epState);
#else
	epInfo_t *item = NULL;
	item = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
//	if ( item != NULL )
//	{
//		item->length = 0x06;
//		memcpy(&item->dataSegment[0], &temp, 2);
//		memcpy(&item->dataSegment[2], &hum, 2);
//		memcpy(&item->dataSegment[4], &light, 2);
//	}
#endif
}

/************************************************************************
* 函数名 : devState_getHumitureLightVal(uint8* ieeeAddr,uint8 endpoint,uint16 *temp,uint16 *hum,uint16 *light)
* 描述   :    获取温度、湿度、光照值
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
bool devState_getHumitureLightVal(uint8* ieeeAddr,uint8 endpoint,uint16 *temp,uint16 *hum,uint16 *light)
{
  
	log_debug("devState_getHumitureLightVal++\n");
#if 0
	devState_t *epState;

    if((epState = devStateListGetRecordByIeeeEp(ieeeAddr,endpoint))!=NULL)
    {
		*temp = (epState->dataSegment[0]<<8)|(epState->dataSegment[1]);
		*hum  = (epState->dataSegment[2]<<8)|(epState->dataSegment[3]);
		*light= (epState->dataSegment[4]<<8)|(epState->dataSegment[5]);
        return true;
    }
    return false;
#else

	epInfo_t *item = NULL;
	item = vdevListGetDeviceByIeeeEp(ieeeAddr,endpoint);
//	if ( item != NULL )
//	{
//		*temp = (item->dataSegment[0]<<8)|(item->dataSegment[1]);
//		*hum  = (item->dataSegment[2]<<8)|(item->dataSegment[3]);
//		*light= (item->dataSegment[4]<<8)|(item->dataSegment[5]);
//        return true;
//	}
	
	log_debug("devState_getHumitureLightVal--\n");
	return false;
#endif
	
}


