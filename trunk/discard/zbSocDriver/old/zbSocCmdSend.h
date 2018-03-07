/**************************************************************************************************
 * Filename:       zbSocCmdSend.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-01,11:23)    :   Create the file.
 *
 *
 *************************************************************************/

#ifndef ZLLSOC_CMD_SEND_H
#define ZLLSOC_CMD_SEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "comParse.h"

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
    uint8_t endpoint; 				// Endpoint identifier
    uint16_t profileID; 			// Profile identifier
    uint16_t deviceID; 				// Device identifier
    uint8_t version; 				// Version
    char*  deviceName;
    uint8_t registerflag; 			//register flag    只有在服务器上注册了的设备才可以发送数据到服务器端
    uint8_t onlineflag;    			// 节点是否在线，1 在线，0不在线
    uint8_t onlineDevRssi;    		// 节点信号量
    uint16_t onlineTimeoutCounter; 	//设备超时时间记录,单位为分钟
    uint8_t capbility;     		 	//register flag
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

void zbSoc_RevertOneDevFactorySettingCmd(uint16 dstAddr,uint8 endpoint);

void zbSoc_OpenNwkCmd(uint8 duration);

void zbSoc_CoorSystemResetCmd(void);

void zbSoc_SetGenOnOffState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode);

void zbSoc_QueryLightValueState(uint16 dstAddr, uint8 endpoint, uint8 addrMode);

void zbSoc_QueryDoorLockPowerValueState(uint16 dstAddr, uint8 endpoint, uint8 addrMode);

void zbSoc_SetEndSensorIntervalReportReq(uint16 intervalTime, uint16 dstAddr,uint8 endpoint, uint8 addrMode);

void zbSoc_IRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ctrlcmd);

void zbSoc_IRCDevCtrlSetLearnAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr);

void zbSoc_IRCDevCtrlSendAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr);


void zbSoc_RemoteIRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ctrlcmd);

void zbSoc_RemoteIRCDevCtrlCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ircDataLen,uint8 *ircData);


void zbSoc_CurtainDevCtrlCmdReq(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdvalue);

void zbSoc_AddGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint,uint8 addrMode);

void zbSoc_RemoveGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint, uint8 addrMode);

void zbSoc_AddSceneMemberCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr,
                             uint8 endpoint, uint8 addrMode,uint16 deviceid,
                             uint8 data1,uint8 data2,uint8 data3,uint8 data4);

void zbSoc_RemoveSceneMemberCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr, uint8 endpoint, uint8 addrMode);

void zbSoc_RecallSceneCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr, uint8 endpoint, uint8 addrMode);


#ifdef __cplusplus
}
#endif

#endif /* ZLLSOC_CMD_SEND_H */
