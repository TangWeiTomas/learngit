/**************************************************************************************************
 * Filename:       interface_vDeviceList.h
 * Author:             edward
 * E-Mail:          ouxiangping@feixuekj.cn
 * Description:    在内存中管理设备信息
 *
 *  Copyright (C) 2016 feixue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2016-03-16,10:03)    :   Create the file.
 *
 *
 *************************************************************************/
#include <stdint.h>
#include "zbSocCmd.h"
//#include "hal_types.h"
#include "Types.h"
#include <stdbool.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "queue.h"
#include "logUtils.h"

#include "zbSocCmd.h"


epInfo_t* vdevListGetDeviceByIeeeEp(uint8_t ieee[],uint8_t endpoint);
epInfo_t * vdevListGetNextDev(uint32_t *context);
epInfo_t * vdevListGetDeviceByNaEp(uint16_t nwkAddr, uint8_t endpoint);

void  vdevListInit(void);
void  vdevListShowAllDeviceList(void);
void  vdevListRemoveDeviceByIeeeEp(uint8_t ieee[],uint8_t endpoint);
void  vdevListModifyByIeeeEp(uint8_t ieee[8],uint8_t endpoint,epInfo_t *epinfo);
int   vdevListAddDevice(epInfo_t *epinfo);
bool  vdevListModifyByNaEp(uint16_t nwkAddr, uint8_t endpoint,epInfo_t * epInfo);
int   vdevListSetTimeOutCnt(void);
void vdevListUpdateDeviceRssi(epInfo_t *epinfo,int8_t rssi);



