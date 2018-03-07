/**************************************************************************
 * Filename:       doorlock_zbSocCmds.h
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    门锁控制ZCL命令
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/

#ifndef __DOORLOCK_ZBSOC_CMDS_H__
#define __DOORLOCK_ZBSOC_CMDS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>

/*********************************************************************
 * CONSTANTS
 */
 
/*********************************************************************
 * MACROS
 */
#define COMMAND_CLOSURES_LOCK_DOOR                         0x00 // M  zclDoorLock_t
#define COMMAND_CLOSURES_UNLOCK_DOOR                       0x01 // M  zclDoorLock_t
#define COMMAND_CLOSURES_TOGGLE_DOOR                       0x02 // O  zclDoorLock_t
#define COMMAND_CLOSURES_SET_PIN_CODE                      0x05 // O  zclDoorLockSetPINCode_t
#define COMMAND_CLOSURES_CLEAR_PIN_CODE                    0x07 // O  zclDoorLockUserID_t
#define COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES               0x08 // O  no payload
	
#define COMMAND_POLL_CONTROL_SET_SHORT_POLL_INTERVAL       0x03 // O, zclCmdSetShortPollIntervalPayload_t
/*门锁开关状态上报*/
#define ATTRID_CLOSURES_LOCK_STATE                         0x0000

//Short poll 间隔属性
#define ATTRID_POLL_CONTROL_SHORT_POLL_INTERVAL            0x0002   	// M, R,  UINT16

//门锁上报的状态
#define CLOSURES_LOCK_STATE_LOCKED                         0x01
#define CLOSURES_LOCK_STATE_UNLOCKED                       0x02

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
	uint16_t userID;
	uint8_t userStatus; 	// e.g. USER_STATUS_AVAILABLE
	uint8_t userType; 	// e.g. USER_TYPE_UNRESTRICTED_USER
	uint8_t *pPIN;		// variable length string
} zclDoorLockSetPINCode_t;

/*********************************************************************
 * VARIABLES
 */
 
  
/*********************************************************************
 * FUNCTIONS
 */
 
extern void doorlock_SetShortPollRate(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,uint16_t times);
extern void doorLock_SetLockOrUnLock(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t dlStatus,uint8_t PinCodeLen,uint8_t *PinCode);
extern void doorlock_SetPINCode(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,zclDoorLockSetPINCode_t *PinCode);
extern void doorlock_ClearPINCode(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,uint16_t userid);
extern void doorlock_ClearAllPINCode(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __DOORLOCK_ZBSOC_CMDS_H__ */

