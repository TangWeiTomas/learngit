/**************************************************************************************************
 * Filename:       interface_srpcserver.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:
 *
 */

#ifndef _SRPC_SERVER_H_
#define _SRPC_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "errorCode.h"
#include "comParse.h"


#define Off    	0
#define On    	1

#define Light_White    0
#define Light_RGB      1


extern bool g_getCoordVersion;

void SRPC_ProcessIncoming(hostCmd *cmd);
void SRPC_GetDevVersionInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t version);
void SRPC_ConfigApSSidCmdCfm(uint8_t status);

void cmdMsgSend(uint8_t* cmdMsg,uint16_t cmdMsgLen);

uint8_t SRPC_GetCoorVersionCmdInd(uint8_t version);

void SRPC_ConfigSsidPasswdCfm(uint8_t status);

void SRPC_ConfigEthernetCmdCfm(uint8_t status);

//
void SRPC_OpenNwkCfm(uint8_t status);
uint8_t SRPC_MtUtilSetChannelsCfm(uint8_t result);

void SRPC_Mt_Network_Conflict_Ind(uint8_t mStatus);
void SRPC_GetMacAddrCfm(uint8_t* macAddr);
void SRPC_Mt_Network_SerialState_Ind(void);

void SRPC_RevertFactorySettingCfm(uint8_t status);

void SRPC_RevertOneFactorySettingCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);


void SRPC_heartPacketInd(uint8_t devCount,uint8_t* pBuf,uint16_t pBufLen);

void SRPC_QueryAllUnregisterDevCfm(uint8_t devCount,uint8_t* buf,uint16_t bufLen);


void SRPC_ComDevRegisterInd(uint8_t* ieeeAddr,uint16_t deviceid,uint8_t portVal);



void SRPC_PeriodTimerSwitchCtrlCfm(uint8_t status,uint8_t taskId);

void SRPC_DeletePeriodTimerSwitchCtrlCfm(uint8_t status,uint8_t timeTaskId);
void SRPC_CtlPeriodTimerSwitchCtrlCfm(uint8_t status,uint8_t timeTaskId);

//组管理及场景管理
void SRPC_AddDevGroupCfm(uint8_t status);

void SRPC_GatewayFactoryInd(void);


void SRPC_SwitchCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);
void SRPC_SwitchQueryCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_SwitchStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint8_t rssi);
void SRPC_SwitchSocketValueInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t type,uint32_t values);

//本地红外学习
void SRPC_EnterIRCLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_ExitIRCLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_SetIRCLearnAddrCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_IRCLearnRetInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t addr);

void SRPC_SendIRCCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

//远程红外学习
void SRPC_EnterRemoteIRCLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_ExitRemoteIRCLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_SendRemoteIRCCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_RemoteIRCLearnRetInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t dataLen,uint8_t *data);

void SRPC_RemoteIRCCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);
void SRPC_ComAlarmPowerValueInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t deviceID,uint8_t value);
void SRPC_RemoteIRCLearnDataRetInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t dataLen,uint8_t *data);

void SRPC_RemoteIrcCodeLibDataRetInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t type,uint16_t dataLen,uint8_t *data);
void SRPC_WinCurtainCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

uint8_t SRPC_UpDateMasterHostFWReq(hostCmd *cmd);
uint8_t SRPC_UpDateMasterHostFWCfm(uint8_t result,uint8_t actionOfprogress,uint8_t progress );


void SRPC_HumitureLightCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_HumitureLightStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t temp,uint16_t hum,uint16_t ilum);

void SRPC_QueryHumitureLightStateCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint16_t temp,uint16_t hum,uint16_t light);


void SRPC_HumitureIntervalCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_HumitureStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t date);

void SRPC_QueryHumitureStateCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_EnvironmentTempHumStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t temp,uint16_t hum);

void SRPC_EnvironmentLightStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t lightState);

void SRPC_EnvironmentPM25StateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t pmState);

void SRPC_EnvironmentNoiceStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t noiceState);


//通用报警设备
void SRPC_ComAlarmCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_ComAlarmStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t deviceID,uint8_t status);
void SRPC_ComAlarmDeviceStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t deviceID,uint8_t value);

//智能门锁设备
void SRPC_DoorLockCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_DoorLockCtrlInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint8_t rssi);
void SRPC_QueryDoorLockSpeedCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint16_t mSpeed);
void SRPC_DoorLockPowerValueInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint8_t rssi);
void SRPC_SetDoorLockSpeedCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint16_t mSpeed);
void SRPC_QueryPowerValueCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

//智能取电开关设备
void SRPC_PowerSwitchCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

void SRPC_PowerSwitchCtrlInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint8_t rssi);
void SRPC_DoorLockPowerValueResp(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

//中央空调
void SRPC_CentralAirInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t airLength,uint8_t *airData);

uint8_t SRPC_GetCoordPanidAndChannelsInd(uint8_t IEEEAddrs[8],uint16_t Panid,uint16_t channels,uint8_t dev_state);
void SRPC_SetCoordResetInd(uint8_t status);
void SRPC_Util_Get_Device_Info_Cfm(uint8_t status,uint8_t ieeeaddr[8],uint16_t shortAddr,uint8_t devcietype,uint8_t devciestate,uint8_t NumassocDevcies,uint16_t *AssocDeviceList);

void SRPC_QueryPowerSwitchValueInd(uint8_t* ieeeAddr,uint8_t endpoint,uint32_t status,uint8_t rssi);

uint8_t SRPC_PermissionRequest_Ind(void);
void SRPC_PeriodTimerSwitchCtrlInd(uint8_t devCount,uint8_t* pBuf,uint16_t pBufLen);


bool makeMsgEnder(hostCmd *cmd);
#ifdef __cplusplus
}
#endif

#endif //RECV_MSG_HANDLE_H
