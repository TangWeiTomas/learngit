/**************************************************************************
 * Filename:       template.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    
 * Description:    门锁控制ZCL命令
 
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/


/*********************************************************************
* INCLUDES
*/

#include <stdint.h>
#include "zbSocCmd.h"
#include "GwComDef.h"
#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"
#include "interface_srpcserver_defs.h"
#include "zbSocCmd.h"
#include "errorCode.h"
#include "zbSocPrivate.h"
#include "doorlock_zbSocCmds.h"

/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/

/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/

       
/*********************************************************************
* LOCAL VARIABLES
*/


/*********************************************************************
* LOCAL FUNCTIONS
*/


/*********************************************************************
* GLOBAL FUNCTIONS
*/


/*设置DataRequest间隔*/
void doorlock_SetShortPollRate(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,uint16_t times)
{
		hostCmd cmd;
		cmd.idx =0;

		cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
		cmdSet8bitVal(&cmd, 0);//len
		cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
		cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
		cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
		cmdSet16bitVal_lh(&cmd, dstAddr);
		cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
		cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_POLL_CONTROL);//ZCL CLUSTER ID
		cmdSet8bitVal(&cmd, 0);//DataLen
		cmdSet8bitVal(&cmd, addrMode);//Addr mode
		cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
		cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
		cmdSet8bitVal(&cmd, COMMAND_POLL_CONTROL_SET_SHORT_POLL_INTERVAL);//ZCL COMMAND ID

		cmdSet16bitVal_lh(&cmd, times);
		
		zbMakeMsgEnder(&cmd);
	 
		usleep(1000);//300ms
		zbSocCmdSend(cmd.data,cmd.idx);
}


 /*********************************************************************
 * @fn					doorLock_SetLockOrUnLock
 *
 * @brief 			控制门锁开关
 *
 * @param 			dstAddr  - 目标地址.
 * @param 			endpoint - 目标端口号.
 * @param 			addrMode - 发送模式.
 * @param 			dlStatus - 门锁状态.
 * @param 			PinCodeLen - 密码长度.
 * @param 			*PinCode - 密码.
 *
 * @return			void
 */
void doorLock_SetLockOrUnLock(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t dlStatus,uint8_t PinCodeLen,uint8_t *PinCode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, dlStatus);//ZCL COMMAND ID
    cmdSet8bitVal(&cmd, PinCodeLen);//Doorlock pin code length

    if(PinCodeLen > 0 && PinCode != NULL)
    	cmdSetStringVal(&cmd,PinCode,PinCodeLen);//Doorlock PIN code

    zbMakeMsgEnder(&cmd);
   
	usleep(1000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}


/*设置密码*/
void doorlock_SetPINCode(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,zclDoorLockSetPINCode_t *PinCode)
{
		hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, COMMAND_CLOSURES_SET_PIN_CODE);//ZCL COMMAND ID

		cmdSet16bitVal_lh(&cmd,PinCode->userID);
    cmdSet8bitVal(&cmd, PinCode->userStatus);
    cmdSet8bitVal(&cmd, PinCode->userType);
		cmdSetStringVal(&cmd,PinCode->pPIN,PinCode->pPIN[0]+1);
		
    zbMakeMsgEnder(&cmd);
   
		usleep(1000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}

/*清除密码*/
void doorlock_ClearPINCode(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,uint16_t userid)
{
		hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, COMMAND_CLOSURES_CLEAR_PIN_CODE);//ZCL COMMAND ID

		cmdSet16bitVal_lh(&cmd,userid);
    
    zbMakeMsgEnder(&cmd);
   
		usleep(1000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}

/*清除所有密码*/
void doorlock_ClearAllPINCode(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
		hostCmd cmd;
		cmd.idx =0;

		cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
		cmdSet8bitVal(&cmd, 0);//len
		cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
		cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
		cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
		cmdSet16bitVal_lh(&cmd, dstAddr);
		cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
		cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK);//ZCL CLUSTER ID
		cmdSet8bitVal(&cmd, 0);//DataLen 
		cmdSet8bitVal(&cmd, addrMode);//Addr mode
		cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
		cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
		cmdSet8bitVal(&cmd, COMMAND_CLOSURES_CLEAR_ALL_PIN_CODES);//ZCL COMMAND ID
		
		zbMakeMsgEnder(&cmd);
	 
		usleep(1000);//300ms
		zbSocCmdSend(cmd.data,cmd.idx);

}

/*********************************************************************
* @fn          funtion_name
*
* @brief       Add device to descovery list.
*
* @param       pSimpleDescRsp - SimpleDescRsp containing epInfo of new EP.
*
* @return      index of device or 0xFF if no room in list
*/


/*********************************************************************/

