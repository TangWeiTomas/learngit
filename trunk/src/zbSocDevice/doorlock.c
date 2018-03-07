/***********************************************************************************
 * 文 件 名   : doorlock.c
 * 负 责 人   : Edward
 * 创建日期   : 2017年2月21日
 * 文件描述   : 门锁接口
 * 版权说明   : Copyright (c) 2008-2017   无锡飞雪网络科技有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

/*********************************************************************
* INCLUDES
*/
#include "doorlock.h"
#include "GwComDef.h"
#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"
#include "interface_srpcserver_defs.h"
#include "zbSocCmd.h"
#include "errorCode.h"
#include "zbSocPrivate.h"

#include "doorlock_Yaotai.h"
#include "doorlock_Level.h"
#include "doorlock_ygs.h"
#include "doorlock_modules.h"
#include "doorlock_jiulian.h"
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

/*门锁回调函数*/
zbSocDoorLockCallbacks_t* zbSocDoorLockCallbacks[]=
{
	&zbSocDoorLock_modulesCallBack_t,
	&zbSocDoorLock_YaoTaiCallBack_t,
	&zbSocDoorLock_LevelCallBack_t,
	&zbSocDoorLock_YgsCallBack_t,
	&zbSocDoorLock_JiuLianCallBack_t,
};

/*********************************************************************
* LOCAL VARIABLES
*/

zbSocDoorLockAppCallbacks_t *zbSocGetCallBacks(uint16_t deviceid)
{
	uint8_t cnt = 0;;
	for(cnt=0;cnt < ARRAY_SIZE(zbSocDoorLockCallbacks) ;cnt++)
	{
		if(zbSocDoorLockCallbacks[cnt]->DeviceID == deviceid)
		{
			return zbSocDoorLockCallbacks[cnt]->zbDoorLockAppCallbacks;
		}
	}

	return NULL;
}

/*********************************************************************
* LOCAL FUNCTIONS
*/
void SRPC_DoorLock_CfmCB(uint16_t cmds,uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status )
{
		hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,cmds);
    
    cmdSetStringVal(&cmd,ieeeAddr,IEEE_ADDR_SIZE);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

uint8_t SRPC_DoorLock_GetSupperted(epInfo_t **epInfo,uint8_t* ieeeAddr,uint8_t endpoint)
{
		epInfo_t *ep = NULL;
	
		uint8_t ret = YY_STATUS_SUCCESS;

		if(ieeeAddr == NULL)
			return YY_STATUS_NODE_NO_EXIST;

		ep = vdevListGetDeviceByIeeeEp(ieeeAddr,endpoint);

	 if(ep != NULL)
	 {
			*epInfo = ep;
			if(ep->onlineflag == true)
				ret = YY_STATUS_SUCCESS;
			else
				ret = YY_STATUS_OUTLINE;
	 }
	 else
	 {
			ret = YY_STATUS_NODE_NO_EXIST;
	 }
			
	return ret;
}

/*********************************************************************
* GLOBAL FUNCTIONS
*/
void SRPC_DoorLockCtrlReqCB(uint16_t cmd ,hostCmd *msg)
{
	uint8_t ret = 0;
	uint8_t ieeeAddr[IEEE_ADDR_SIZE],endPoint,switchCmd;
	zbSocDoorLockAppCallbacks_t *appCallBacks = NULL;
	epInfo_t *epInfo = NULL;

	log_debug("SRPC_DoorLockCtrlReqCB++\n");

	cmdGetStringVal(msg, &ieeeAddr[0],IEEE_ADDR_SIZE);
	cmdGet8bitVal(msg, &endPoint);
	cmdGet8bitVal(msg, &switchCmd);

	ret = SRPC_DoorLock_GetSupperted(&epInfo,ieeeAddr,endPoint);

	if(ret != YY_STATUS_NODE_NO_EXIST )
	{
		if(switchCmd > DOORLOCK_CMD_NORMAL_OPEN)
			ret = YY_STATUS_UNSUPPORT_PARAM;
		else
		{
			appCallBacks  =zbSocGetCallBacks(epInfo->deviceID);
			if(appCallBacks != NULL && appCallBacks->pfnDoorLock != NULL)
			{
				appCallBacks->pfnDoorLock(epInfo,switchCmd);
			}
			else
			{
				ret = YY_STATUS_UNSUPPORT_CMD;
			}
		}
	}

	SRPC_DoorLock_CfmCB(cmd,ieeeAddr,endPoint,ret);
	log_debug("SRPC_DoorLockCtrlReqCB--\n");
}

void SRPC_DoorLockCtrlIndCB(epInfo_t *epInfo,uint8_t result,uint8_t dlst,uint8_t optype,uint16_t uid,uint8_t timeVail,uint8_t times[6])
{
		log_debug("SRPC_DoorLockCtrlIndCB++\n");

		hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_STATUS_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁命令状态
    cmdSet8bitVal(&cmd,result);
    cmdSet8bitVal(&cmd,dlst);
    //D12 开门类型
    cmdSet8bitVal(&cmd,optype);
    //D13-D16 UID0-UID3
//  cmdSetStringVal(&cmd,uid,4);
		cmdSet16bitVal(&cmd,uid);
    cmdSet8bitVal(&cmd,timeVail);
    
    if(timeVail == 0x01 && times != NULL)
    {
    	//D17-D22 门锁时间
    	cmdSetStringVal(&cmd,times,6);
    }
    
//		cmdSet8bitVal(&cmd,rssi);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
    
		log_debug("SRPC_DoorLockCtrlIndCB--\n");
}

void SRPC_DoorLockPowerReqCB(uint16_t cmd ,hostCmd *msg)
{
		uint8_t ret = 0;
		uint8_t ieeeAddr[IEEE_ADDR_SIZE],endPoint;
    zbSocDoorLockAppCallbacks_t *appCallBacks = NULL;
		epInfo_t *epInfo = NULL;

    log_debug("SRPC_DoorLockPowerReqCB++\n");

    cmdGetStringVal(msg, &ieeeAddr[0],IEEE_ADDR_SIZE);
    cmdGet8bitVal(msg, &endPoint);

		ret = SRPC_DoorLock_GetSupperted(&epInfo,ieeeAddr,endPoint);
		
		if(ret != YY_STATUS_NODE_NO_EXIST )
		{
			appCallBacks  =zbSocGetCallBacks(epInfo->deviceID);
			if(appCallBacks != NULL && appCallBacks->pfnDoorLockGetPower != NULL)
			{
				 appCallBacks->pfnDoorLockGetPower(epInfo);
			}
			else
			{
				ret = YY_STATUS_UNSUPPORT_CMD;
			}
		}
	  SRPC_DoorLock_CfmCB(cmd,ieeeAddr,endPoint,ret);
		log_debug("SRPC_DoorLockPowerReqCB--\n");
}

void SRPC_DoorLockPowerIndCB(epInfo_t *epInfo,uint8_t result,uint8_t powerstu,uint8_t optype,uint16_t powerValue)
{
		log_debug("SRPC_DoorLockPowerIndCB++\n");

		hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_POWER_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁命令状态
    cmdSet8bitVal(&cmd,result);
		cmdSet8bitVal(&cmd,powerstu);
    cmdSet8bitVal(&cmd,optype);
   	cmdSet16bitVal(&cmd,powerValue);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
    log_debug("SRPC_DoorLockPowerIndCB--\n");
}

void SRPC_DoorLockAlarmIndCB(epInfo_t *epInfo,uint8_t type,uint8_t timevail,uint8_t times[6])
{
		hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_ALARM_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11
    cmdSet8bitVal(&cmd,type);
		cmdSet8bitVal(&cmd,timevail);

		if(timevail == 0x01)
			cmdSetStringVal(&cmd,times,6);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_DoorLockDeviceInfoReqCB(uint16_t cmd ,hostCmd *msg)
{
		uint8_t ret = 0;
		uint8_t ieeeAddr[IEEE_ADDR_SIZE],endPoint;
		zbSocDoorLockAppCallbacks_t *appCallBacks = NULL;
		epInfo_t *epInfo = NULL;

    log_debug("SRPC_DoorLockCtrlReqCB++\n");

    cmdGetStringVal(msg, &ieeeAddr[0],IEEE_ADDR_SIZE);
    cmdGet8bitVal(msg, &endPoint);
  
		ret = SRPC_DoorLock_GetSupperted(&epInfo,ieeeAddr,endPoint);

		if(ret != YY_STATUS_NODE_NO_EXIST )
		{
			appCallBacks  =zbSocGetCallBacks(epInfo->deviceID);
			if(appCallBacks != NULL && appCallBacks->pfnDoorLockState != NULL)
			{
				ret = appCallBacks->pfnDoorLockState(epInfo);
			}
			else
			{
				ret = YY_STATUS_UNSUPPORT_CMD;
			}
		}
		
		SRPC_DoorLock_CfmCB(cmd,ieeeAddr,endPoint,ret);
		log_debug("SRPC_DoorLockCtrlReqCB--\n");
}

void SRPC_DoorLockDeviceInfoIndCB(epInfo_t *epInfo,uint8_t result,uint8_t dlstu,uint8_t pwrstu,uint8_t ddlstu,uint8_t rssi)
{
		hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_GET_INFO_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11
    cmdSet8bitVal(&cmd,result);
	cmdSet8bitVal(&cmd,dlstu);
	cmdSet8bitVal(&cmd,pwrstu);
	cmdSet8bitVal(&cmd,ddlstu);
	cmdSet8bitVal(&cmd,rssi);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_DoorSetPINCodeReqCB(uint16_t cmd ,hostCmd *msg)
{
		uint8_t ret = 0;
		uint8_t ieeeAddr[IEEE_ADDR_SIZE],endPoint;
    zbSocDoorLockAppCallbacks_t *appCallBacks = NULL;
		epInfo_t *epInfo = NULL;
		
		uint8_t pwdType;
		uint8_t pwdOpert;
		uint16_t userid;
		uint8_t pwd[24] = {0};
		uint8_t pwdlen;

    log_debug("SRPC_DoorLockPasswdManageCB++\n");

    cmdGetStringVal(msg, &ieeeAddr[0],IEEE_ADDR_SIZE);
    cmdGet8bitVal(msg, &endPoint);

		ret = SRPC_DoorLock_GetSupperted(&epInfo,ieeeAddr,endPoint);

		if(ret != YY_STATUS_NODE_NO_EXIST )
		{
			cmdGet8bitVal(msg, &pwdType);
			cmdGet8bitVal(msg, &pwdOpert);
			cmdGet16bitVal(msg, &userid);

			if((pwdOpert > DOORLOCK_USR_CLEAR)||(pwdType > DOORLOCK_PWD_PINID))
			{
				ret = YY_STATUS_UNSUPPORT_PARAM;
			}
			else
			{
				if(pwdOpert == DOORLOCK_USR_ADD)
				{
					cmdGet8bitVal(msg, &pwdlen);
					
					if(pwdlen < 6 || pwdlen > 24)
					{
						ret = YY_STATUS_FAIL;
						goto result ;
					}

					cmdGetStringVal(msg,pwd,pwdlen);
				}

				appCallBacks  =zbSocGetCallBacks(epInfo->deviceID);
				if(appCallBacks != NULL && appCallBacks->pfnDoorLockSetPinCode != NULL)
				{
					ret = appCallBacks->pfnDoorLockSetPinCode(epInfo, pwdType, pwdOpert, userid, pwd, pwdlen);
				}
				else
				{
					ret = YY_STATUS_UNSUPPORT_CMD;
				}
			}
		}
		
result:		
		SRPC_DoorLock_CfmCB(cmd,ieeeAddr,endPoint,ret);
		
		log_debug("SRPC_DoorLockPasswdManageCB--\n");
}

void SRPC_DoorLockPINCodeIndCB(epInfo_t *epInfo,uint8_t result)
{
		log_debug("SRPC_DoorLockPINCodeIndCB++\n");

		hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_PWD_MNG_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁命令状态
    cmdSet8bitVal(&cmd,result);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
    log_debug("SRPC_DoorLockPINCodeIndCB--\n");
}

void SRPC_DoorSetRFIDCodeReqCB(uint16_t cmd ,hostCmd *msg)
{
		uint8_t ret = 0;
		uint8_t ieeeAddr[IEEE_ADDR_SIZE],endPoint;
    zbSocDoorLockAppCallbacks_t *appCallBacks = NULL;
		epInfo_t *epInfo = NULL;

		uint8_t optType = 0;
		uint16_t usrid = 0;
		uint8_t idlen = 0;
		uint8_t id[128] = {0};
		
    log_debug("SRPC_DoorSetRFIDCodeReqCB++\n");

    cmdGetStringVal(msg, &ieeeAddr[0],IEEE_ADDR_SIZE);
    cmdGet8bitVal(msg, &endPoint);

		ret = SRPC_DoorLock_GetSupperted(&epInfo,ieeeAddr,endPoint);

		if(ret != YY_STATUS_NODE_NO_EXIST )
		{
			cmdGet8bitVal(msg, &optType);
			cmdGet16bitVal(msg, &usrid);
			
			if(optType > DOORLOCK_USR_CLEAR)
			{
				ret = YY_STATUS_UNSUPPORT_PARAM;
			}
			else
			{
				if(optType == DOORLOCK_USR_ADD)
				{
					cmdGet8bitVal(msg, &idlen);
					
					cmdGetStringVal(msg,id,idlen);
				}

				appCallBacks  =zbSocGetCallBacks(epInfo->deviceID);
				if(appCallBacks != NULL && appCallBacks->pfnDoorLockSetRFIDCode != NULL)
				{
					ret = appCallBacks->pfnDoorLockSetRFIDCode(epInfo, optType, usrid, idlen, id);
				}
				else
				{
					ret = YY_STATUS_UNSUPPORT_CMD;
				}
			}
		}
				
		SRPC_DoorLock_CfmCB(cmd,ieeeAddr,endPoint,ret);
		
		log_debug("SRPC_DoorSetRFIDCodeReqCB--\n");
}

void SRPC_DoorLockRFIDCodeIndCB(epInfo_t *epInfo,uint8_t result)
{
		log_debug("SRPC_DoorLockRFIDCodeIndCB++\n");

		hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_CARD_MNG_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁命令状态
    cmdSet8bitVal(&cmd,result);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
    log_debug("SRPC_DoorLockRFIDCodeIndCB--\n");
}

void SRPC_DoorLockSetFingerCodeReqCB(uint16_t cmd,hostCmd *msg)
{
		uint8_t ret = 0;
		uint8_t ieeeAddr[IEEE_ADDR_SIZE],endPoint;
		zbSocDoorLockAppCallbacks_t *appCallBacks = NULL;
		epInfo_t *epInfo = NULL;

		uint8_t optType = 0;
		uint16_t usrid = 0;
		
		log_debug("SRPC_DoorLockSetFingerCodeReqCB++\n");

		cmdGetStringVal(msg, &ieeeAddr[0],IEEE_ADDR_SIZE);
		cmdGet8bitVal(msg, &endPoint);

		ret = SRPC_DoorLock_GetSupperted(&epInfo,ieeeAddr,endPoint);

		if(ret != YY_STATUS_NODE_NO_EXIST )
		{
			cmdGet8bitVal(msg, &optType);

			cmdGet16bitVal(msg, &usrid);
			
			if(optType > DOORLOCK_USR_CLEAR || optType == DOORLOCK_USR_ADD)
			{
				ret = YY_STATUS_UNSUPPORT_PARAM;
			}
			else
			{
				appCallBacks	=zbSocGetCallBacks(epInfo->deviceID);
				if(appCallBacks != NULL && appCallBacks->pfnDoorLockSetFingerCode != NULL)
				{
					ret = appCallBacks->pfnDoorLockSetFingerCode(epInfo, optType, usrid);
				}
				else
				{
					ret = YY_STATUS_UNSUPPORT_CMD;
				}
			}
		}
			
		SRPC_DoorLock_CfmCB(cmd,ieeeAddr,endPoint,ret);
		
		log_debug("SRPC_DoorLockSetFingerCodeReqCB--\n");

}


void SRPC_DoorLockFingerCodeIndCB(epInfo_t *epInfo,uint8_t result)
{
		log_debug("SRPC_DoorLockRFIDCodeIndCB++\n");

		hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_FIGNER_MNG_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁命令状态
    cmdSet8bitVal(&cmd,result);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
    log_debug("SRPC_DoorLockRFIDCodeIndCB--\n");
}

//Door open pattern
void  SRPC_DoorLockGetDoorOpenPatternReqCB(uint16_t cmd,hostCmd *msg)
{
	
}

void  SRPC_DoorLockSetDoorOpenPatternReqCB(uint16_t cmd,hostCmd *msg)
{
	
}


void SRPC_DoorLocDoorOpenPatternIndCB(epInfo_t *epInfo,uint8_t result,uint8_t type,uint8_t disable)
{
	log_debug("SRPC_DoorLocDoorOpenPatternIndCB++\n");

	hostCmd cmd;
	cmd.idx = 0;

	makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
	//D0 D1 Opcode
	cmdSet16bitVal(&cmd,DOORLOCK_OPEN_TYPE_IND);
	//D2-D9 控制结果
	cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
	//D10 控制结果
	cmdSet8bitVal(&cmd,epInfo->endpoint);
	//D11 门锁命令状态
	cmdSet8bitVal(&cmd,result);
	cmdSet8bitVal(&cmd,type);
	cmdSet8bitVal(&cmd,disable);

	makeMsgEnder(&cmd);
	cmdMsgSend(cmd.data,cmd.idx);
	log_debug("SRPC_DoorLocDoorOpenPatternIndCB--\n");
}

void SRPC_DoorLockGetShortPollRateReqCB(uint16_t cmd ,hostCmd *msg)
{
	
}

void SRPC_DoorLockSetShortPollRateReqCB(uint16_t cmd ,hostCmd *msg)
{
		uint8_t ret = 0;
		uint8_t ieeeAddr[IEEE_ADDR_SIZE],endPoint;
		uint16_t timeInter = 0;
		zbSocDoorLockAppCallbacks_t *appCallBacks = NULL;
		epInfo_t *epInfo = NULL;

    log_debug("SRPC_DoorLockSetShortPollRateReqCB++\n");

    cmdGetStringVal(msg, &ieeeAddr[0],IEEE_ADDR_SIZE);
    cmdGet8bitVal(msg, &endPoint);
  	cmdGet16bitVal(msg, &timeInter);
		
		ret = SRPC_DoorLock_GetSupperted(&epInfo,ieeeAddr,endPoint);
		
		if(ret != YY_STATUS_NODE_NO_EXIST )
		{
			//换算出1/4seconds
			timeInter = timeInter /250;
			
			if(timeInter <4 || timeInter > 40)
			{
				ret = YY_STATUS_UNSUPPORT_PARAM;
			}
			else
			{	
				appCallBacks  =zbSocGetCallBacks(epInfo->deviceID);
				if(appCallBacks != NULL && appCallBacks->pfnDoorLockSetPollRate != NULL)
				{
					ret = appCallBacks->pfnDoorLockSetPollRate(epInfo,timeInter);
				}
				else
				{
					ret = YY_STATUS_UNSUPPORT_CMD;
				}
			}
		}
		
		SRPC_DoorLock_CfmCB(cmd,ieeeAddr,endPoint,ret);
		log_debug("SRPC_DoorLockSetShortPollRateReqCB--\n");
}


void SRPC_DoorLockShortPollRateIndCB(epInfo_t *epInfo,uint8_t result,uint16_t times)
{
		hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_SET_POLLRATE_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11
    cmdSet8bitVal(&cmd,result);

		if(result == SUCCESS)
			cmdSet16bitVal(&cmd, times);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

//门锁用户操作设置上报
void SRPC_DoorLockUserOptIndCB(epInfo_t *epInfo,uint16_t userid,uint8_t userType,uint8_t userAccess,uint8_t userOpt)
{
	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,DOORLOCK_USER_OPT_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11
	cmdSet8bitVal(&cmd,userType);
	cmdSet8bitVal(&cmd,userAccess);
	cmdSet8bitVal(&cmd,userOpt);

	cmdSet16bitVal(&cmd,userid);
		
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}
/*********************************************************************/
