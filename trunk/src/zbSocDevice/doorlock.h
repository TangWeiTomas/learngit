/***********************************************************************************
 * 文 件 名   : doorlock.h
 * 负 责 人   : Edward
 * 创建日期   : 2017年2月21日
 * 文件描述   : 门锁接口
 * 版权说明   : Copyright (c) 2008-2017   无锡飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#ifndef __DOORLOCK_H__
#define __DOORLOCK_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include "zbSocCmd.h"

/*********************************************************************
 * CONSTANTS
 */
 
 /*开门常用命令*/
#define DOORLOCK_CMD_CLOSE						0x00
#define DOORLOCK_CMD_OPEN						0x01
#define DOORLOCK_CMD_NORMAL_OPEN				0x02
/*门锁状态*/
#define DOORLOCK_STATUS_CLOSE					0x00
#define DOORLOCK_STATUS_OPEN					0x01
#define DOORLOCK_STATUS_NORMAL_OPEN				0x02
/*开门类型*/
#define DOORLOCK_TYPE_CARD						0X00
#define DOORLOCK_TYPE_PWD						0X01
#define DOORLOCK_TYPE_FINGER					0X02
#define DOORLOCK_TYPE_REMOTE					0X03
#define DOORLOCK_TYPE_OTHER						0X04
#define DOORLOCK_TYPE_KEY						0x05

/*电量上报类型*/
#define DOORLOCK_POWER_TYPE_UNKNOW				0XFF
#define DOORLOCK_POWER_TYPE_STATUS				0X00
#define DOORLOCK_POWER_TYPE_VALUE				0X01
#define DOORLOCK_POWER_TYPE_PERCENT				0X02
/*上报类型校验*/
#define DOORLOCK_POWER_TYPE_STATUS_BIT			0X01
#define DOORLOCK_POWER_TYPE_VALUE_BIT			0X02
#define DOORLOCK_POWER_TYPE_PERCENT_BIT			0X04

/*电量状态*/
#define DOORLOCK_POWER_ST_NORMAL				0X00
#define DOORLOCK_POWER_ST_LOW					0X01
#define DOORLOCK_POWER_NORMAL					0X02
#define DOORLOCK_POWER_LOW						0x03

/*报警类型*/
#define DOORLOCK_ALARM_PWD						0X00
#define DOORLOCK_ALARM_CARD						0X01
#define DOORLOCK_ALARM_FINGER					0X02
#define DOORLOCK_ALARM_PWD_LOCKED				0X03
#define DOORLOCK_ALARM_VILENCE					0X04
#define DOORLOCK_ALARM_BELL						0X05
#define DOORLOCK_ALARM_ULOCKED					0X06
#define DOORLOCK_ALARM_LOW_BATTERY				0X07

/*时间有效性*/
#define DOORLOCK_TIME_VAIL						0x00
#define DOORLOCK_TIME_UNVAIL					0x01
/*密码/指纹/卡片操作类型*/
#define DOORLOCK_USR_ADD						0X00
#define DOORLOCK_USR_DEL						0x01
#define DOORLOCK_USR_CLEAR						0X02
/*密码操作*/
#define DOORLOCK_PWD_ADMIN						0X00
#define DOORLOCK_PWD_NORMAL						0X01
#define DOORLOCK_PWD_CYCLE						0X02
#define DOORLOCK_PWD_TIME						0X03
#define DOORLOCK_PWD_PINID						0x04
/*反锁状态*/
#define DOORLOCK_DLSTU_UNLOCK					0X00
#define DOORLOCK_DLSTU_LOCKED					0X01
#define DOORLOCK_DLSTU_UNSUP					0XFF

/*记录类型*/
#define RECORD_TYPE_0_CARD						0
#define RECORD_TYPE_1_CARD						1
#define RECORD_TYPE_2_CARD						2
#define RECORD_TYPE_3_CARD						3
#define RECORD_TYPE_4_CARD						4
#define RECORD_TYPE_5_CARD						5
#define RECORD_TYPE_6_CARD						6
#define RECORD_TYPE_7_CARD						7
#define RECORD_TYPE_8_CARD						8
#define RECORD_TYPE_9_CARD						9
#define RECORD_TYPE_10_CARD						10
#define RECORD_TYPE_11_CARD						11
#define RECORD_TYPE_12_CARD						12
#define RECORD_TYPE_13_CARD						13
#define RECORD_TYPE_14_CARD						14
#define RECORD_TYPE_15_CARD						15
#define RECORD_TYPE_18_CARD						18
#define RECORD_TYPE_21_PWD						21
#define RECORD_TYPE_22_FINGER					22
#define RECORD_TYPE_24_REMOTE					24
#define RECORD_TYPE_27_SYS_START				27
#define RECORD_TYPE_28_CARD						28
#define RECORD_TYPE_29_CARD						29
#define RECORD_TYPE_30_INKEY					30
#define RECORD_TYPE_31_KEY						31

/*权限类型*/
#define DOORLOCK_AUTH_TYPE_ADMIN				0X00
#define DOORLOCK_AUTH_TYPE_NORMAL				0X01
#define DOORLOCK_AUTH_TYPE_TEMP					0X02 

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
 
typedef uint8_t (*zbSoc_DoorLock_t)(epInfo_t *epInfo,uint8_t status);
typedef uint8_t (*zbSoc_DoorLockGetPower_t)(epInfo_t *epInfo); 
typedef uint8_t (*zbSoc_DoorLockGetState_t)(epInfo_t *epInfo); 
typedef uint8_t (*zbSoc_DoorLockSetPinCode_t)(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen); 
typedef uint8_t (*zbSoc_DoorLockSetRFIDCode_t)(epInfo_t *epInfo,uint8_t optType,uint16_t userid,uint8_t idlen,uint8_t *id); 
typedef uint8_t (*zbSoc_DoorLockSetFINGERCode_t)(epInfo_t *epInfo,uint8_t optType,uint16_t userid); 
typedef uint8_t (*zbSoc_DoorLockGetDoorOpenPattern_t)(epInfo_t *epInfo); 
typedef uint8_t (*zbSoc_DoorLockSetDoorOpenPattern_t)(epInfo_t *epInfo,uint8_t status); 
typedef uint8_t (*zbSoc_DoorLockGetPollRate_t)(epInfo_t *epInfo); 
typedef uint8_t (*zbSoc_DoorLockSetPollRate_t)(epInfo_t *epInfo,uint16_t times); 

typedef struct
{
	zbSoc_DoorLock_t 										pfnDoorLock;
	zbSoc_DoorLockGetPower_t 						pfnDoorLockGetPower;
	zbSoc_DoorLockGetState_t 						pfnDoorLockState;
	zbSoc_DoorLockSetPinCode_t 					pfnDoorLockSetPinCode;
	zbSoc_DoorLockSetRFIDCode_t 				pfnDoorLockSetRFIDCode;
	zbSoc_DoorLockSetFINGERCode_t 			pfnDoorLockSetFingerCode;
	zbSoc_DoorLockGetDoorOpenPattern_t	pfnDoorLockGetDoorOpenPattern;
	zbSoc_DoorLockSetDoorOpenPattern_t	pfnDoorLockSetDoorOpenPattern;
	zbSoc_DoorLockGetPollRate_t					pfnDoorLockGetPollRate;
	zbSoc_DoorLockSetPollRate_t					pfnDoorLockSetPollRate;
}zbSocDoorLockAppCallbacks_t;

typedef struct
{
	uint16_t DeviceID;
	zbSocDoorLockAppCallbacks_t *zbDoorLockAppCallbacks;
}zbSocDoorLockCallbacks_t;

/*********************************************************************
 * VARIABLES
 */
 
extern zbSocDoorLockCallbacks_t* zbSocDoorLockCallbacks[];   
/*********************************************************************
 * FUNCTIONS
 */
 
void SRPC_DoorLockCtrlReqCB(uint16_t cmd ,hostCmd *msg);
void SRPC_DoorLockCtrlIndCB(epInfo_t *epInfo,uint8_t result,uint8_t dlst,uint8_t optype,uint16_t uid,uint8_t timeVail,uint8_t times[6]);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __DOORLOCK_H__ */
