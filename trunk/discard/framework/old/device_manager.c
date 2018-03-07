/**************************************************************************************************
 * Filename:       localCmdMng.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-11-30,13:28)    :   Create the file.
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

#include "device_manager.h"
#include "zbSocCmd.h"
#include "interface_devicelist.h"
#include "zbSocPrivate.h"
#include "interface_devicelist.h"
#include "interface_deviceStatelist.h"
#include "zbSocMasterControl.h"
#include "interface_vDeviceList.h"


/************************************************************************
* 函数名 :devMgr_getAllRegisterOnlineDevices(hostCmd *cmd)
* 描述   :   获取所有的已注册的在线节点设备的IEEE地址
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
uint8 devMgr_getAllRegisterOnlineDevices(uint8* devCount,uint8 *pBuf,uint16 *pBufLen)
{
	epInfo_t *epInfo = NULL;

    uint32_t context = 0;
    hostCmd cmd;
    cmd.idx = 0;

    *devCount=0;

   log_debug("devMgr_getAllRegisterOnlineDevices++\n");
   
    while ((epInfo = vdevListGetNextDev(&context)) != NULL)
    {       
        if((epInfo->nwkAddr == 0)||(epInfo->endpoint == 0))
			continue;
	
#if USE_MASTER_CONTROL
			//屏蔽主控设备
		if(epInfo->deviceID == ZB_DEV_MASTER_CONTROL)
			continue;
#endif

        if((epInfo->registerflag == true)&&(epInfo->onlineflag==true))
        {
            //设备数量加1
            (*devCount)++;
            cmdSetStringVal(&cmd, epInfo->IEEEAddr, 8);
            cmdSet8bitVal(&cmd, epInfo->endpoint);
            cmdSet16bitVal(&cmd, epInfo->deviceID);

//			virEpInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
//			if(virEpInfo == NULL)
//			{
				cmdSet8bitVal(&cmd, epInfo->onlineDevRssi);
//			}
//			else
//			{
//				cmdSet8bitVal(&cmd, virEpInfo->onlineDevRssi);
//			}
            
			log_debug("IEEE:");
			log_debug_array(epInfo->IEEEAddr,8,":");
			log_debug("Endpoint:%02x\n",epInfo->endpoint);

        }
    
//        usleep(500);
//        msleep(10);
    }

    *pBufLen = cmd.idx;
    memcpy(pBuf,cmd.data,cmd.idx);

    log_debug("devMgr_getAllRegisterOnlineDevices--\n");
    return 0;
}

/************************************************************************
* 函数名 :devMgr_getAllUnregisterOnlineDevices(hostCmd *cmd)
* 描述   :   获取所有的未注册的在线节点设备的IEEE地址
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
uint8 devMgr_getAllUnregisterOnlineDevices(uint8* devCount,uint8 *pBuf,uint16 *pBufLen)
{
  
    epInfo_t *epInfo = NULL;
    uint32_t context = 0;
    hostCmd cmd;
    cmd.idx = 0;

    *devCount=0;

    log_debug("devMgr_getAllUnregisterOnlineDevices++\n");

    while ((epInfo = vdevListGetNextDev(&context)) != NULL)
    {
		if((epInfo->nwkAddr == 0)||(epInfo->endpoint == 0))
			continue;
    	
//屏蔽主控设备
#if USE_MASTER_CONTROL
		if(epInfo->deviceID == ZB_DEV_MASTER_CONTROL)
			continue;
#endif	

        if((epInfo->registerflag == false)&&(epInfo->onlineflag==true))
        {
            //设备数量加1
            (*devCount)++;
            cmdSetStringVal(&cmd, epInfo->IEEEAddr, 8);
            cmdSet8bitVal(&cmd, epInfo->endpoint);
            cmdSet16bitVal(&cmd, epInfo->deviceID);

			log_debug("IEEE:");
            log_debug_array(epInfo->IEEEAddr,8,":");
            log_debug("Endpoint:%02x\n",epInfo->endpoint);

        }
    }
    usleep(1000);
 
    *pBufLen = cmd.idx;
    memcpy(pBuf,cmd.data,cmd.idx);

   	log_debug("devMgr_getAllUnregisterOnlineDevices--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_SetDevOnlineByIeeeEp(uint16 nwkAddr)
* 描述   :   更新设备在线的信息
* 输入   ：
* 输出   ：无
* 返回   ：0:没有设备状态更新1:设备在线状态更新
************************************************************************/
uint8 devMgr_UpdateDevOnlineStateByIeee(uint8* ieeeAddr, bool Status,uint8 rssi)
{
    uint8 ret=0;
    log_debug("devMgr_UpdateDevOnlineStateByIeee++\n");
    if(Status != true)
    {
        //超出范围
        return ret;
    }
	
#if 0
	uint32_t context = 0;

	epInfo_t *epInfo;

	uint8 IEEEAddr[8]= {0};

    memcpy(IEEEAddr, ieeeAddr, 8);

	log_debug_array(IEEEAddr,8,":");

    while ((epInfo = devListGetNextDev(&context)) != NULL)
    {
        //比较下IEEE 地址
        if(memcmp(epInfo->IEEEAddr,IEEEAddr,8) == 0)
        {
            if(epInfo->onlineflag != Status)
            {
                //标记是否有设备在线状态更新
                ret = 1;
                epInfo->onlineflag = Status;
            }
            epInfo->onlineTimeoutCounter = 0;

            if(rssi != 0) epInfo->onlineDevRssi = rssi;
            //更新数据库信息
            devListModifyRecordByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint,epInfo);
        }
    }
#else
	
//	ret = vdevListUpdateDevOnlineStateByIeee(ieeeAddr,  Status, rssi);

#endif
    log_debug("devMgr_UpdateDevOnlineStateByIeee--\n");
    return ret;
}


/************************************************************************
* 函数名 :devMgr_UpdateDevOnlineTimeoutCounter(uint16 nwkAddr)
* 描述   :   一分钟更新一次设备的信息
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
uint8 devMgr_UpdateDevOnlineTimeoutCounter(void)
{

    log_debug("devMgr_UpdateDevOnlineTimeoutCounter++ \n");
	
#if 0
    while ((epInfo = devListGetNextDev(&context)) != NULL)
    {
        log_debug("nwkAddr=%x,endpoint=%x \n",epInfo->nwkAddr,epInfo->endpoint);

		if (epInfo->deviceID == ZB_DEV_DOOR_SENSOR) //门磁设备特殊处理
		{
			continue;
		}
			
        if((epInfo->nwkAddr != 0)&&(epInfo->endpoint != 0))
        {
            log_debug("IND nwkAddr=%x,endpoint=%x \n",epInfo->nwkAddr,epInfo->endpoint);

            if(epInfo->onlineflag == true)
            {
                epInfo->onlineTimeoutCounter++;
            }

            //5 分钟没有设备上报信息就认为设备离线
            if(epInfo->onlineTimeoutCounter >= MAX_ONLINE_TIMEOUT_COUNTER)
            {
                devMgr_SetDevOnlineStateByNaEp(epInfo,false);
            }
            else
            {
                //更新数据库信息
                devListModifyRecordByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint,epInfo);
            }
        }
    }
#else
	vdevListSetTimeOutCnt();
#endif
	
    log_debug("devMgr_UpdateDevOnlineTimeoutCounter-- \n");
    return 0;
}

