/**************************************************************************************************
 * Filename:       zbSocMasterControl.h
 * Author:             edward
 * E-Mail:          ouxiangping@feixuekj.cn
 * Description:    特殊设备，主控设备管理器
 *
 *  Copyright (C) 2014 fei xue keji Company - http://www.feixuekj.cn
 *
 * Version:         1.00  (2014-11-30,10:03)    :   Create the file.
 *
 *
 *************************************************************************/

#ifndef __ZBSOC_MASTER_CONTROL_H__
#define __ZBSOC_MASTER_CONTROL_H__

#include "Types.h"
#include "zbSocUart.h"
#include "zbSocCmd.h"
#include "comParse.h"
#include "globalVal.h"

void zbSoc_MasterControlRegister(epInfoExtended_t *epInfoEx);
uint8_t zbSoc_MasterControlSetOnOffState(epInfo_t *epInfo,uint8_t switchcmd);
uint8_t zbSoc_MasterControlCheck(uint8_t* ieeeAddr,uint8_t endpoint);
uint8_t zbSoc_MasterControlReportResolve(hostCmd *cmd,epInfo_t *epInfo);
uint8_t zbSoc_MasterControlReadRspResolve(hostCmd *cmd,epInfo_t *epInfo);


#endif
