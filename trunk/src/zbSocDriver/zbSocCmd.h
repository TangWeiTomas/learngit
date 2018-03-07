/**************************************************************************************************
 * Filename:       zbSocCmd.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-11-30,10:01)    :   Create the file.
 *
 *
 *************************************************************************/

#ifndef ZLLSOCCMD_H
#define ZLLSOCCMD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"
#include "zbSocUart.h"

/********************************************************************/
extern uint8_t IRC_Remote_Learn_Count ;

uint8_t DevideID[6];

uint8_t DevicesNum;

extern uint8_t zbTransSeqNumber;

/********************************************************************/
// ZLL Soc Types
typedef enum
{
    afAddrNotPresent = 0,
    afAddrGroup      = 1,
    afAddr16Bit      = 2,
    afAddr64Bit      = 3,
    afAddrBroadcast  = 15
} afAddrMode_t;

#define Z_EXTADDR_LEN 8

#define MinSocCmdPacketLength      5 //FLG(1)+LEN(1)+CMD(2)+...+FCS(1)

//包头起始标志
#define SOC_MSG_FLAG    0xFE

//数据包部分位置标志
#define SOC_MSG_FLAG_POS    	0
#define SOC_MSG_LEN_POS    		1
#define SOC_MSG_CMD0_POS    	2
#define SOC_MSG_DATALEN_POS    	10


// Endpoint information record entry
typedef struct
{
    uint8_t IEEEAddr[8];
    uint16_t nwkAddr; 				// Network address
    uint16_t profileID; 			// Profile identifier
    uint16_t deviceID; 				// Device identifier
    uint16_t onlineTimeoutCounter; 	//设备超时时间记录,单位为分钟
    uint8_t endpoint; 				// Endpoint identifier
    uint8_t version; 				// Version
    uint8_t registerflag; 			//register flag    只有在服务器上注册了的设备才可以发送数据到服务器端
    uint8_t onlineflag;    			// 节点是否在线，1 在线，0不在线
    uint8_t onlineDevRssi;    		// 节点信号量
    uint8_t capbility;     		 	//register flag
    char*  deviceName;
} epInfo_t;

typedef struct
{
    epInfo_t * epInfo;
    uint16_t prevNwkAddr;   	// Precious network address
    uint8_t type;   			// new / updated / old
} epInfoExtended_t;


#define EP_INFO_TYPE_EXISTING 				0
#define EP_INFO_TYPE_NEW 					1
#define EP_INFO_TYPE_UPDATED 				2
#define EP_INFO_TYPE_REMOVED 				4

#define IRC_CTRL_ENTER_LEARN_MODE  			0x0001
#define IRC_CTRL_EXIT_LEARN_MODE    		0x0002
#define IRC_CTRL_SET_LEARN_ADDR     		0x0003
#define IRC_CTRL_LEARN_STATE_IND    		0x0004
#define IRC_CTRL_SEND_CMD_ADDR      		0x0010

#define IRC_REMOTE_CTRL_ENTER_LEARN_MODE  	0x1001
#define IRC_REMOTE_CTRL_EXIT_LEARN_MODE     0x1002
#define IRC_REMOTE_LEARN_DATA_IND           0x1003
#define IRC_REMOTE_CTRL_SEND_CMD_DATA       0x1010

#define ENVI_DEV_VOICE  					0x02
#define ENVI_DEV_LIGHT    					0x06
#define ENVI_DEV_PM25     					0x05
#define ENVI_DEV_TEMP_HUM   				0x04



/******************************************/
void zbSoc_RevertFactorySettingCmd(void);

//void zbSoc_RevertOneDevFactorySettingCmd(uint16_t dstAddr,uint8_t* ieee);
void zbSoc_RevertOneDevFactorySettingCmd(uint16_t dstAddr,uint8_t endpoint);

void zbSoc_Permit_Join_Req(uint8_t duration);

void zbSoc_QueryDeviceVerion(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

void zbSoc_SetGenOnOffState(uint8_t state, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

void zbSoc_QueryLightValueState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

void zbSoc_QuerySwitchSocketValueState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,uint8_t attrs);

void zbSoc_QueryDoorLockPowerValueState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

void zbSoc_SetEndSensorIntervalReportReq(uint16_t intervalTime, uint16_t dstAddr,uint8_t endpoint, uint8_t addrMode);

void zbSoc_IRCDevCtrlLearnModeCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint16_t ctrlcmd);

void zbSoc_IRCDevCtrlSetLearnAddrCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t cmdaddr);

void zbSoc_IRCDevCtrlSendAddrCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t cmdaddr);
void zbSoc_IRC_RemoteIModeCtrlCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t ctrlcmd);


void zbSoc_RemoteIRCDevCtrlLearnModeCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint16_t ctrlcmd);

void zbSoc_RemoteIRCDevCtrlCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint16_t ircDataLen,uint8_t *ircData);

void zbSoc_CurtainDevPercentageCmdReq(uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);
void zbSoc_CurtainDevCtrlCmdReq(uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

void zbSoc_AddGroupMemberCmd(uint16_t groupId, uint16_t dstAddr, uint8_t endpoint,uint8_t addrMode);

void zbSoc_RemoveGroupMemberCmd(uint16_t groupId, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);


void zbSoc_RemoveSceneMemberCmd(uint16_t groupId, uint8_t sceneId, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

void zbSoc_RecallSceneCmd(uint16_t groupId, uint8_t sceneId, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

void zbSoc_SetCentralAirCmdReq(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint16_t airDataLen,uint8_t *airData);


void zbSoc_ProcessEvent(epInfo_t *epinfo,uint8_t state);

uint8_t zbMakeMsgEnder(hostCmd *cmd);


/*********************************************/
/*******  	Diagnostic Cluster Cmds 	******/
/*********************************************/
void zclDiagnostic_GetLastMessageLqi(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);
void zclDiagnostic_GetLastMessageRssi(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

//ZCL Set API's

//ZCL Get API's
void zbSocMsg_ProcessIncoming(hostCmd *cmd);

void zbSoc_DevRegisterReporting(epInfoExtended_t *epInfoEx);
void processRpcSysAppFn_DevReadResp(hostCmd *cmd,uint16_t clusterID,uint16_t nwkAddr, uint8_t endpoint);
void processRpcSysAppFn_DevStateReport(hostCmd *cmd,uint16_t nwkAddr, uint8_t endpoint);
void processRpcSysAppFn_DevHeartBeatReport(hostCmd *cmd,uint16_t nwkAddr, uint8_t endpoint);

void zbSoc_GetCoordVersionCmd(void);
void zbSoc_SetCoordResetCmd(void);

void zbSoc_SetTempIntervalReportReq(uint16_t intervalTime, uint16_t dstAddr,uint8_t endpoint, uint8_t addrMode);
void zbSoc_SetDevValidReq(bool onoff, uint16_t dstAddr,uint8_t endpoint, uint8_t addrMode);
void zbSoc_getHumitureState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);
#ifdef __cplusplus
}
#endif

#endif /* ZLLSOCCMD_H */
