/***********************************************************************************
 * 文 件 名   : doorlock_Level.h
 * 负 责 人   : Edward
 * 创建日期   : 2016年10月12日
 * 文件描述   : 力维门锁
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#ifndef __WATER_METER_XX_H__
#define __WATER_METER_XX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zbSocCmd.h"

void waterMeter_ReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void waterMeter_HeartReport(hostCmd *cmd,epInfo_t *epInfo);
void waterMeter_StateReport(hostCmd *cmd,epInfo_t *epInfo);
/*
void waterMeter_QueryBalance(epInfo_t *epInfo);
void waterMeter_QueryResidueWater(epInfo_t *epInfo);
void waterMeter_Recharge(epInfo_t *epInfo, int money);
void waterMeter_LadderPrice(epInfo_t *epInfo, uint16_t price[5][2]);
void waterMeter_DeviceCtrl(epInfo_t *epInfo, uint8_t enable);
void waterMeter_PriceMode(epInfo_t *epInfo, uint8_t mode);
void waterMeter_WarnValue(epInfo_t *epInfo, uint16_t waterValue);
 * */

void waterMeter_serverProcess(uint16_t nCMD, epInfo_t *epInfo, hostCmd *cmd);
#ifdef __cplusplus
}
#endif
#endif