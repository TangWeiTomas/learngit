/**************************************************************************************************
 * Filename:       cmdMsgSend.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Send Msg to Remote Server.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:
 *
 */

#ifndef CMD_MSG_SEND_H
#define CMD_MSG_SEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "comParse.h"




void cmdMsgSend(uint8* cmdMsg,uint16 cmdMsgLen);


void zigbeeDev_ConfigSsidPasswdCfm(uint8 status);

void zigbeeDev_ConfigEthernetCmdCfm(uint8 status);

//
void zigbeeDev_OpenNwkCfm(uint8 status);

void zigbeeDev_GetMacAddrCfm(uint8* macAddr);

void zigbeeDev_RevertFactorySettingCfm(uint8 status);

void zigbeeDev_RevertOneFactorySettingCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);


void roomfairy_LightCtrlCfm(uint8 status);

void zigbeeDev_RoomfairyRegisterInd(void);

void zigbeeDev_heartPacketInd(uint8 devCount,uint8* pBuf,uint16 pBufLen);

void zigbeeDev_QueryAllUnregisterDevCfm(uint8 devCount,uint8* buf,uint16 bufLen);


void zigbeeDev_ComDevRegisterInd(uint8* ieeeAddr,uint16 deviceid,uint8 portVal);



void zigbeeDev_PeriodTimerSwitchCtrlCfm(uint8 status,uint8 taskId);

void zigbeeDev_DeletePeriodTimerSwitchCtrlCfm(uint8 status);


//组管理及场景管理
void zigbeeDev_AddDevGroupCfm(uint8 status);



void zigbeeDev_SwitchCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_SwitchStateInd(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi);


//本地红外学习
void zigbeeDev_EnterIRCLearnModeCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_ExitIRCLearnModeCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_SetIRCLearnAddrCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_IRCLearnRetInd(uint8* ieeeAddr,uint8 endpoint,uint8 addr);

void zigbeeDev_SendIRCCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

//远程红外学习
void zigbeeDev_EnterRemoteIRCLearnModeCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_ExitRemoteIRCLearnModeCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_SendRemoteIRCCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_RemoteIRCLearnRetInd(uint8* ieeeAddr,uint8 endpoint,uint16 dataLen,uint8 *data);


void zigbeeDev_WinCurtainCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);



void zigbeeDev_HumitureLightCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_HumitureLightStateInd(uint8* ieeeAddr,uint8 endpoint,uint16 temp,uint16 hum,uint16 ilum);

void zigbeeDev_QueryHumitureLightStateCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint16 temp,uint16 hum,uint16 light);


void zigbeeDev_HumitureIntervalCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_HumitureStateInd(uint8* ieeeAddr,uint8 endpoint,uint16 temp,uint16 hum);

void zigbeeDev_QueryHumitureStateCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint16 temp,uint16 hum);


void zigbeeDev_EnvironmentTempHumStateInd(uint8* ieeeAddr,uint8 endpoint,uint16 temp,uint16 hum);

void zigbeeDev_EnvironmentLightStateInd(uint8* ieeeAddr,uint8 endpoint,uint8 lightState);

void zigbeeDev_EnvironmentPM25StateInd(uint8* ieeeAddr,uint8 endpoint,uint8 pmState);

void zigbeeDev_EnvironmentNoiceStateInd(uint8* ieeeAddr,uint8 endpoint,uint8 noiceState);


//通用报警设备
void zigbeeDev_ComAlarmCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_ComAlarmStateInd(uint8* ieeeAddr,uint8 endpoint,uint16 deviceID,uint8 status);

//智能门锁设备
void zigbeeDev_DoorLockCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_DoorLockCtrlInd(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi);

void zigbeeDev_DoorLockPowerValueInd(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi);

void zigbeeDev_QueryPowerValueCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

//智能取电开关设备
void zigbeeDev_PowerSwitchCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status);

void zigbeeDev_PowerSwitchCtrlInd(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi);


#ifdef __cplusplus
}
#endif

#endif //CMD_MSG_SEND_H
