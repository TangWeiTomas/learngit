#ifndef __MT_ZBSOCCMD_H__
#define __MT_ZBSOCCMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "zbSocCmd.h"
//#include "hal_types.h"
#include "comParse.h"


/*******************************/
/*** MT Util Commands ***/
/*******************************/

//#define SUCCESS		0
//#define FAILED		1
#define SERIAL_CMD_TIMEOUT		5000

typedef enum
{
  DEV_HOLD,                                // Initialized - not started automatically
  DEV_INIT,                                // Initialized - not connected to anything
  DEV_NWK_DISC,                            // Discovering PAN's to join
  DEV_NWK_JOINING,                         // Joining a PAN
  DEV_NWK_SEC_REJOIN_CURR_CHANNEL,         // ReJoining a PAN in secure mode scanning in current channel, only for end devices
  DEV_END_DEVICE_UNAUTH,                   // Joined but not yet authenticated by trust center
  DEV_END_DEVICE,                          // Started as device after authentication
  DEV_ROUTER,                              // Device joined, authenticated and is a router
  DEV_COORD_STARTING,                      // Started as Zigbee Coordinator
  DEV_ZB_COORD,                            // Started as Zigbee Coordinator
  DEV_NWK_ORPHAN,                          // Device has lost information about its parent..
  DEV_NWK_KA,                              // Device is sending KeepAlive message to its parent
  DEV_NWK_BACKOFF,                         // Device is waiting before trying to rejoin
  DEV_NWK_SEC_REJOIN_ALL_CHANNEL,          // ReJoining a PAN in secure mode scanning in all channels, only for end devices
  DEV_NWK_TC_REJOIN_CURR_CHANNEL,          // ReJoining a PAN in Trust center mode scanning in current channel, only for end devices
  DEV_NWK_TC_REJOIN_ALL_CHANNEL            // ReJoining a PAN in Trust center mode scanning in all channels, only for end devices
} devStates_t;

typedef enum
{
	MT_UTIL_GET_DEVICE_INFO 			= 0x00,
	MT_UTIL_GET_NV_INFO 				= 0x01,
	MT_UTIL_SET_PANID 					= 0x02,
	MT_UTIL_SET_CHANNELS 				= 0x03,
	MT_UTIL_SET_SECLEVEL 				= 0x04,
	MT_UTIL_SET_PRECFGKEY 				= 0x05,
	MT_UTIL_SET_CALLBACK_SUB_CMD 		= 0x06,
	MT_UTIL_SET_KEY_EVENT 				= 0x07,
	MT_UTIL_SET_TIME_ALIVE 				= 0x09,
	MT_UTIL_SET_LED_CONTROL 			= 0x0A,
	MT_UTIL_LOOKBACK 					= 0x10,
	MT_UTIL_DATA_REQ 					= 0x11,
	MT_UTIL_SRC_MATCH_ENABLE 			= 0x20,
	MT_UTIL_SRC_MATCH_ADD_ENTRY 		= 0x21,
	MT_UTIL_SRC_MATCH_DEL_ENTRY 		= 0x22,
	MT_UTIL_SRC_MATCH_CHECK_SRC_ADDR 	= 0x23,
	MT_UTIL_SRC_MATCH_ACK_ALL_PENDING 	= 0x24,
	MT_UTIL_SRC_MATCH_CHECK_ALL_PENDING = 0x25,
	MT_UTIL_ADDRMGR_EXT_ADDR_LOOKUP 	= 0x27,
	MT_UTIL_ADDRMGR_NWK_ADDR_LOOKUP 	= 0x41,
	MT_UTIL_APSME_LINK_KEY_DATA_GET 	= 0x44,
	MT_UTIL_APSME_LINK_KEY_NV_ID_GET 	= 0x45,
	MT_UTIL_APSME_REQUEST_KEY_CMD 		= 0x4b,
	MT_UTIL_ASSOC_COUNT 				= 0x48,
	MT_UTIL_ASSOC_FIND_DEVICE 			= 0x49,
	MT_UTIL_ASSOC_GET_WITH_ADDRESS 		= 0x4A,
	MT_UTIL_BIND_ADD_ENTRY 				= 0x4D,
	MT_UTIL_ZCL_KEY_EST_INIT_EST 		= 0x80,
	MT_UTIL_ZCL_KEY_EST_SIGN 			= 0x81,
	MT_UTIL_SRNG_GEN 					= 0x4C
}mtUtilCmdType_t;

/*******************************/
/*** MT SYS Commands ***/
/*******************************/

typedef enum
{
	MT_SYS_RESET_REQ 		= 0x00,
	MT_SYS_OSAL_NV_WRITE	= 0x09
}mtSysCmdType_t;

typedef enum
{
	MT_SYS_RESET_IND 			= 0x80,
	MT_SYS_OSAL_TIMER_EXPIRED	= 0x81
}mtSysCmdCbType_t;

/*******************************/
/*** MT ZDO Commands ***/
/*******************************/

#define SINGLE_DEVICE_RESPONSE		0x00
#define EXTENDED					0X01

typedef enum
{
	MT_ZDO_IEEE_ADDR_REQ 		= 0x01,
	MT_ZDO_SIMPLE_DESC_REQ 		= 0x04,
	MT_ZDO_ACTIVE_ED_REQ 		= 0x05, 
	MT_ZDO_EXT_NWK_INFO  		= 0x50
}mtZdoCmdType_t;

typedef enum
{
	MT_ZDO_NWK_CONFLICT_IND		=0x54,
	MT_ZDO_IEEE_ADDR_RSP		=0x81,
	MT_ZDO_SIMPLE_DESC_RSP		=0x84,
	MT_ZDO_ACTIVE_EP_RSP		=0x85,
	MT_ZDO_STATE_CHANGE_IND 	=0xC0,
	MT_ZDO_END_DEVICE_ANNCE_IND	=0xC1,
	MT_ZDO_LEAVE_IND			=0xC9
}mtZdoCmdCbType_t;

/*******************************/
/*** 		Fuctions 		***/
/*******************************/

extern bool mt_getChangeCoordinatorChannels(void);
extern void mt_setChangeCoordinatorChannels(bool state);
extern int  mt_Util_Set_Channels_sreq(uint8_t channel);
extern int8_t mt_Util_Set_Channels_srsp(hostCmd *cmd);
extern int8_t mt_Sys_Reset_sreq(void);
extern int8_t mt_Sys_Reset_srsp(hostCmd *cmd);
int8_t mt_Zdo_Active_ep_req(uint16_t ShortAddr);

extern int8_t mt_Sys_Osal_Nv_Write_sreq(uint16_t id,uint8_t offset,uint8_t vlen,uint8_t *value);
extern int8_t mt_Sys_Osal_Nv_Write_srsp(hostCmd *cmd);
extern int8_t mt_Zdo_Ext_Nwk_Info_sreq(void);
extern int8_t mt_Zdo_Ext_Nwk_Info_srsp(hostCmd *cmd);
extern int8_t mt_Zdo_Ieee_addr_rsp(hostCmd *cmd,epInfo_t *epinfo);
uint16_t mt_zdo_getCoorPaind(void);
uint8_t mt_zdo_getCoorChannel(void);
uint8_t mt_zdo_getCoorState(void);



#ifdef __cplusplus
}
#endif

#endif //ZB_SOC_PRIVATE_H

