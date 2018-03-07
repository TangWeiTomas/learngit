/**************************************************************************
 * Filename:       doorlock_jiulian.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    久联门锁接口
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/


/*********************************************************************
* INCLUDES
*/
#include <time.h>
#include "doorlock_jiulian.h"
#include "doorlock.h"
#include "GwComDef.h"
#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"
#include "interface_srpcserver_defs.h"
#include "zbSocCmd.h"
#include "errorCode.h"
#include "zbSocPrivate.h"
#include "doorlock_zbSocCmds.h"
#include "GwComDef.h"

/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/


#define ATTRID_BASIC_SERIALNET_OVER_EVENT			0xFFFF		//透传功能

#define JIULIAN_PROTOCOL_SOF	0X02
#define JIULIAN_PROTOCOL_ST		0x01
#define MIM_FRAM_SIZE	5


//开锁方式

#define JIULIAN_OPEN_TYPE_FINGER	0X01
#define JIULIAN_OPEN_TYPE_PWD		0x02
#define JIULIAN_OPEN_TYPE_CARD		0x03
#define JIULIAN_OPEN_TYPE_FORCE		0x0D
#define JIULIAN_OPEN_TYPE_REMOTE	0x14

/*********************************************************************
* TYPEDEFS
*/
	
typedef enum
{
	DOORLOCK_CMD_TIME_SYNC			= 0x0003,
	DOORLOCK_CMD_PWR_IND			= 0x0502,
	DOORLOCK_CMD_BATTRY 			= 0x0504,
	DOORLOCK_CMD_USR_REG_IND		= 0x0507,
	DOORLOCK_CMD_USR_DEL_IND		= 0x0508,
	DOORLOCK_CMD_OPEN_LOG_IND		= 0x0509,
	DOORLOCK_CMD_ALARM_LOG_IND		= 0x050A,
	DOORLOCK_CMD_SERACH 			= 0x050E,
	DOORLOCK_CMD_RESET				= 0x0901,
	DOORLOCK_CMD_JOIN				= 0x0902,
}zcl_doorlockCmds_t;


typedef struct 
{
	epInfo_t *epinfo;
	uint8_t result;
	uint8_t doorlockstu;
	uint8_t optType;
	uint16_t userid;
}JiuLianReportData_t;

/*********************************************************************
* LOCAL FUNCTIONS
*/
static void zbSocDoorlock_SetDoorLockPkt(hostCmd *cmds,uint16_t cmd,uint8_t *data,uint16_t datalen);
void zbSocDoorlock_JiuLianSendMsg(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t len,uint8_t* buf);
uint8_t zbSocDoorlock_JiuLianOpenCb(epInfo_t *epInfo,uint8_t status);
uint8_t zbSocDoorlock_JiuLianGetPowerCb(epInfo_t *epInfo);
uint8_t zbSocDoorlock_JiuLianGetStatusCb(epInfo_t *epInfo);
uint8_t zbSocDoorlock_JiuLianSetPInCb(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen);
bool zbSocDoorlock_StartTimerReport(JiuLianReportData_t *reportdata);

/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* LOCAL VARIABLES
*/
static uint8_t UserOperate = DOORLOCK_USR_ADD;
static JiuLianReportData_t JiuLianReportData;
static tu_evtimer_t *JiuLianReportTimer = NULL;

/*********************************************************************
* GLOBAL VARIABLES
*/

//门锁操作回调函数
zbSocDoorLockAppCallbacks_t zbSocDoorLock_JiuLianAppCallbacks = 
{
	zbSocDoorlock_JiuLianOpenCb,
	zbSocDoorlock_JiuLianGetPowerCb,
	zbSocDoorlock_JiuLianGetStatusCb,
	zbSocDoorlock_JiuLianSetPInCb,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,	
};      

//注册门锁信息
zbSocDoorLockCallbacks_t	zbSocDoorLock_JiuLianCallBack_t = 
{
	 ZCL_HA_DEVICEID_FXZB_DOORLOCK_JIULIAN,
	 &zbSocDoorLock_JiuLianAppCallbacks 
};

/********************************************************************
*
*						门锁命令下发
*
********************************************************************/

/*********************************************************************
* @fn		   zbSocDoorlock_JiuLianSendMsg
*
* @brief	   发送透传数据
*
* @param	   epInfo - 节点信息
			   cmd	  - 节点上报的数据
*
* @return	   void
*/
void zbSocDoorlock_JiuLianSendMsg(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t len,uint8_t* buf)
{
	hostCmd cmd;
	cmd.idx =0;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);//len 预留位
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
	cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
	cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
	//发送给所有的路由设备,都开放网络
	cmdSet16bitVal_lh(&cmd, dstAddr);
	cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
	cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
	cmdSet8bitVal(&cmd, 0);//DataLen 预留位
	cmdSet8bitVal(&cmd, addrMode);//Addr mode
	cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
	cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
	cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

	cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_SERIALNET_OVER_EVENT);//Attr ID
	cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

	cmdSet8bitVal(&cmd, len);//数据长度
	cmdSetStringVal(&cmd,buf,len);//数据
   
	zbMakeMsgEnder(&cmd);
	
	usleep(1000);//300ms
	zbSocCmdSend(cmd.data,cmd.idx);
}

//远程开关门
uint8_t zbSocDoorlock_JiuLianOpenCb(epInfo_t *epInfo,uint8_t status)
{
	
	hostCmd cmd;
	cmd.idx=0;
	uint8_t state = 0;
	
	if(epInfo == NULL)
		return FAILED;

	switch(status)
	{
		case DOORLOCK_STATUS_CLOSE://关门
			state = 0xB4;
			zbSocDoorlock_SetDoorLockPkt(&cmd,DOORLOCK_CMD_SERACH,&state,1);		
			break;
		case DOORLOCK_STATUS_OPEN://开门
			state = 0xB1;
			zbSocDoorlock_SetDoorLockPkt(&cmd,DOORLOCK_CMD_SERACH,&state,1);
			break;
		case DOORLOCK_STATUS_NORMAL_OPEN://常开
			//state = DOORLOCK_STATUS_OPEN;
			//zbSocDoorlock_SetDoorLockPkt(&cmd,YGS_CMD_TYPE_REQ,YGS_CMD_REQ_SET_OPENATTR,&state,1);
			return FAILED;
			break;
			default:break;
	}

	zbSocDoorlock_JiuLianSendMsg(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);

	return SUCCESS;
}

//获取电压
uint8_t zbSocDoorlock_JiuLianGetPowerCb(epInfo_t *epInfo)
{
	hostCmd cmd;
	cmd.idx=0;
	uint8_t state = 0;
	
	if(epInfo == NULL)
		return FAILED;

	state = 0xA1;
	
	zbSocDoorlock_SetDoorLockPkt(&cmd,DOORLOCK_CMD_SERACH,&state,1);
	zbSocDoorlock_JiuLianSendMsg(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	
	return SUCCESS;
}

uint8_t zbSocDoorlock_JiuLianGetStatusCb(epInfo_t *epInfo)
{
	zbSocDoorlock_JiuLianGetPowerCb(epInfo);
}

uint8_t zbSocDoorlock_JiuLianSetPInCb(epInfo_t *epInfo,uint8_t pwdType,uint8_t pwdOpert,uint16_t userid,uint8_t *pwd,uint8_t pwdlen)
{
	hostCmd cmd;
    cmd.idx=0;
	
	uint8_t cnt = 0;
	uint8_t idx = 0;
	uint8_t buf[8] = {0};

	if((pwdOpert == DOORLOCK_USR_ADD) && ((pwdlen != 6)||(pwd==NULL)))
		return FAILED;

	switch (pwdOpert)
	{
		case DOORLOCK_USR_ADD://添加密码
		{
			buf[idx++] = 0xC2;
			buf[idx++] = userid & 0x00ff;
			buf[idx++] = (userid>>8) & 0x00ff;

			for(cnt = 0; cnt < 6;cnt++)
			{
				buf[idx++] =  pwd[cnt];
			}
			
			zbSocDoorlock_SetDoorLockPkt(&cmd,DOORLOCK_CMD_SERACH,buf,idx-1);	
			UserOperate = DOORLOCK_USR_ADD;
		}
		break;
		case DOORLOCK_USR_DEL:
		{
			buf[idx++] = 0xC3;
			buf[idx++] = userid & 0x00ff;
			buf[idx] = (userid>>8) & 0x00ff;

			zbSocDoorlock_SetDoorLockPkt(&cmd,DOORLOCK_CMD_SERACH,buf,idx);	
			UserOperate = DOORLOCK_USR_DEL;
		}
		break;
		case DOORLOCK_USR_CLEAR:
		{
			buf[idx++] = 0xC3;
			buf[idx++] = 0xff;
			buf[idx++] = 0xff;

			zbSocDoorlock_SetDoorLockPkt(&cmd,DOORLOCK_CMD_SERACH,buf,idx);
			UserOperate = DOORLOCK_USR_DEL;
		}
		break;
		default:return FAILED;
	}

	zbSocDoorlock_JiuLianSendMsg(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	return SUCCESS;
}

/********************************************************************
*
*						门锁命令解析
*
********************************************************************/

//同步时间
static void zbSocDoorlock_JiuLianSyncTime(epInfo_t *epInfo)
{	
	hostCmd cmds;
	cmds.idx = 0;
	uint8_t idx = 0;

	
	uint8_t doorlocktime[6]={0};
	//获取时间
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);

	doorlocktime[idx++] = p->tm_year-100;
	doorlocktime[idx++] = p->tm_mon+1;
	doorlocktime[idx++] = p->tm_mday;
	doorlocktime[idx++] = p->tm_hour;
	doorlocktime[idx++] = p->tm_min;
	doorlocktime[idx] 	= p->tm_sec;

	log_debug("%d%d%d %d:%d:%d\n",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	log_debug("%d/%d/%d/%d/%d/%d\n",doorlocktime[0],doorlocktime[1],doorlocktime[2],doorlocktime[3],doorlocktime[4],doorlocktime[5]);

	//组包
	zbSocDoorlock_SetDoorLockPkt(&cmds,DOORLOCK_CMD_TIME_SYNC,doorlocktime,idx);

	//发送数据
	zbSocDoorlock_JiuLianSendMsg(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmds.idx,cmds.data);
}


//电池检测上报
static void zbSocDoorlock_JiuLianPowerInd(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t datafieldlen = 0;
	uint8_t powerPercent = 0;


	if(epInfo == NULL || cmd == NULL)
		return;
	
	cmdGet8bitVal (cmd,&datafieldlen);
	cmdGet8bitVal (cmd,&powerPercent);

	if(powerPercent > 100 )
		powerPercent = 100;
	else if( powerPercent < 20)
	{
		vdevListSetDoorAlarm(epInfo,DOORLOCK_POWER_ST_LOW);
		SRPC_DoorLockAlarmIndCB(epInfo,DOORLOCK_ALARM_LOW_BATTERY,0x00,NULL);
	}
	
	vdevListSetDoorBattery(epInfo,DOORLOCK_POWER_TYPE_PERCENT,powerPercent);	
	
	//上报门锁状态数据
	uint8_t doorlockstu = vdevListGetDoorState(epInfo);
	uint8_t	batAlarmFalg =	vdevListGetDoorAlarm(epInfo);
	uint8_t doorlockdlk = vdevListGetDoorDLockStu(epInfo);

	SRPC_DoorLockDeviceInfoIndCB(epInfo,SUCCESS,doorlockstu ,batAlarmFalg,doorlockdlk,epInfo->onlineDevRssi);
	//上报门锁电量信息
	SRPC_DoorLockPowerIndCB(epInfo,SUCCESS,batAlarmFalg,DOORLOCK_POWER_TYPE_PERCENT,powerPercent);
}

//同步注册用户信息
static void zbSocDoorlock_JiuLianRegUserInfoSync(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t datafieldlen = 0;
	uint8_t openType = 0;
	uint16_t userid;
	uint8_t userType;
	uint8_t userAccess;
	uint8_t userOpt;

	if(epInfo == NULL || cmd == NULL)
		return;
	
	cmdGet8bitVal (cmd,&datafieldlen);
	cmdGet16bitVal_lh(cmd, &userid);
	cmdGet8bitVal(cmd,&openType);

	switch(openType)
	{
		case JIULIAN_OPEN_TYPE_FINGER:
			userType = DOORLOCK_TYPE_FINGER;

		break;
		case JIULIAN_OPEN_TYPE_PWD:
			userType = DOORLOCK_TYPE_PWD;

		break;
		case JIULIAN_OPEN_TYPE_CARD:
			userType = DOORLOCK_TYPE_CARD;

		break;
		default:
			userType = DOORLOCK_TYPE_CARD;
		break;
	}

	//小于3的deviceID都为管理用户
	if(userid >= 0x02)
		userAccess = DOORLOCK_AUTH_TYPE_ADMIN;
	else
		userAccess = DOORLOCK_AUTH_TYPE_NORMAL;

	if(UserOperate == DOORLOCK_USR_ADD)
		userOpt = 0x01;
	
	SRPC_DoorLockUserOptIndCB(epInfo,userid,userType,userAccess,UserOperate);
	SRPC_DoorLockPINCodeIndCB(epInfo,SUCCESS);
}

//同步删除用户信息
static void zbSocDoorlock_JiuLianDelUserInfoSync(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t datafieldlen = 0;
	uint16_t userid;
	uint8_t userType;
	uint8_t userAccess;

	if(epInfo == NULL || cmd == NULL)
		return;
	
	cmdGet8bitVal (cmd,&datafieldlen);
	cmdGet16bitVal_lh(cmd, &userid);

	userType = DOORLOCK_TYPE_PWD;

	//小于3的deviceID都为管理用户
	if(userid >= 0x02)
		userAccess = DOORLOCK_AUTH_TYPE_ADMIN;
	else
		userAccess = DOORLOCK_AUTH_TYPE_NORMAL;
	
	SRPC_DoorLockUserOptIndCB(epInfo,userid,userType,userAccess,UserOperate);
	SRPC_DoorLockPINCodeIndCB(epInfo,SUCCESS);
}

//同步开锁记录命令
static void zbSocDoorlock_JiuLianOpenLogSync(epInfo_t *epInfo,hostCmd *cmd)
{
	
	uint8_t datafieldlen = 0;
	uint8_t openType = 0;
	uint16_t userid;
	uint8_t userType;
	uint8_t userAccess;

	if(epInfo == NULL || cmd == NULL)
		return;
	
	cmdGet8bitVal (cmd,&datafieldlen);
	cmdGet16bitVal_lh(cmd, &userid);
	cmdGet8bitVal (cmd,&openType);

	switch(openType)
	{
		case JIULIAN_OPEN_TYPE_FINGER:
			userType = DOORLOCK_TYPE_FINGER;

		break;
		case JIULIAN_OPEN_TYPE_PWD:
			userType = DOORLOCK_TYPE_PWD;

		break;
		case JIULIAN_OPEN_TYPE_CARD:
			userType = DOORLOCK_TYPE_CARD;

		break;
		case JIULIAN_OPEN_TYPE_FORCE:
			userType = DOORLOCK_TYPE_OTHER;
			break;
		case JIULIAN_OPEN_TYPE_REMOTE:
			userType = DOORLOCK_TYPE_REMOTE;
			break;
		default:
			userType = JIULIAN_OPEN_TYPE_PWD;
		break;
	}
	
	vdevListSetDoorState(epInfo,DOORLOCK_STATUS_OPEN);

	SRPC_DoorLockCtrlIndCB(epInfo,SUCCESS,DOORLOCK_STATUS_OPEN,userType,userid,0x00,NULL);

	//设置上报参数
	JiuLianReportData.epinfo = epInfo;
	JiuLianReportData.result = SUCCESS;
	JiuLianReportData.doorlockstu = DOORLOCK_STATUS_CLOSE;
	JiuLianReportData.optType = userType;
	JiuLianReportData.userid = userid;

	zbSocDoorlock_StartTimerReport(&JiuLianReportData);
}

//同步报警消息命令
static void zbSocDoorlock_JiuLianAlarmSync(epInfo_t *epInfo,hostCmd *cmd)
{
	
	uint8_t datafieldlen = 0;
	uint8_t alarms = 0;
	uint8_t alarmReport = 0;
	
	if(epInfo == NULL || cmd == NULL)
		return;
	
	cmdGet8bitVal (cmd,&datafieldlen);
	cmdGet8bitVal (cmd,&alarms);

	if(alarms == 0x0A)
	{
		SRPC_DoorLockCtrlIndCB(epInfo,SUCCESS,DOORLOCK_STATUS_OPEN,DOORLOCK_TYPE_KEY,0xFFFF,0x00,NULL);
	}
	else if(alarms == 0x0B)
	{
		alarmReport = DOORLOCK_ALARM_VILENCE;
		SRPC_DoorLockAlarmIndCB(epInfo,alarmReport,0x00,NULL);
	}
	else if(alarms == 0x0c)
	{
		alarmReport = DOORLOCK_ALARM_PWD;
		SRPC_DoorLockAlarmIndCB(epInfo,alarmReport,0x00,NULL);
	}
}


/*********************************************************************
* @fn          zbSocDoorlock_JiuLianProtocolAnalysis
*
* @brief    	协议解析
*
* @param       epInfo - 节点信息
			   cmd	  - 节点上报的数据
*
* @return      void
*/
static void zbSocDoorlock_JiuLianProtocolAnalysis(epInfo_t *epInfo,hostCmd *cmd)
{
	//数据包长度
	uint8_t datalen = 0;
	uint8_t datasof = 0;
	uint16_t doorlockCmd = 0;
	uint8_t doorlockState = 0;

	log_debug("zbSocDoorlock_JiuLianProtocolAnalysis++\n");
	
	cmdGet8bitVal(cmd,&datalen);

	if(datalen < MIM_FRAM_SIZE)
		return;
	
	cmdGet8bitVal (cmd,&datasof);
	cmdGet16bitVal(cmd,&doorlockCmd);
	cmdGet8bitVal (cmd,&doorlockState);

	if(!doorlockState)
		return;

	switch(doorlockCmd)
	{
		//同步时间
		case DOORLOCK_CMD_TIME_SYNC:
			zbSocDoorlock_JiuLianSyncTime(epInfo);
			break;
		//电池上报
		case DOORLOCK_CMD_PWR_IND:
			zbSocDoorlock_JiuLianPowerInd(epInfo,cmd);
			break; 
		//同步注册用户ID命令
		case DOORLOCK_CMD_USR_REG_IND:
			zbSocDoorlock_JiuLianRegUserInfoSync(epInfo,cmd);
			break;
		//同步删除用户ID命令
	  	case DOORLOCK_CMD_USR_DEL_IND:
			zbSocDoorlock_JiuLianDelUserInfoSync(epInfo,cmd);
			break;
		//同步开锁记录命令
	  	case DOORLOCK_CMD_OPEN_LOG_IND:
			zbSocDoorlock_JiuLianOpenLogSync(epInfo,cmd);
			break;
		//同步报警消息命令
	  	case DOORLOCK_CMD_ALARM_LOG_IND:
			zbSocDoorlock_JiuLianAlarmSync(epInfo,cmd);
			break;
	}

	log_debug("zbSocDoorlock_JiuLianProtocolAnalysis--\n");
}

/*********************************************************************
* @fn          zbSocDoorlock_JiuLianSerialNetProcess
*
* @brief       久联门锁数据上报处理
*
* @param       epInfo - 节点信息
			   cmd	  - 节点上报的数据
*
* @return      void
*/
void zbSocDoorlock_JiuLianSerialNetProcess(epInfo_t *epInfo,hostCmd *cmd)
{
	uint16_t attrID  = 0;
	uint8_t dataType = 0;
	
	log_debug("zbSocDoorlock_JiuLianSerialNetProcess++\n");

	if(epInfo == NULL)
		return;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);

	if((attrID == ATTRID_BASIC_SERIALNET_OVER_EVENT) && (dataType == ZCL_DATATYPE_OCTET_STR))
	{
		log_debug("attrID = ATTRID_BASIC_SERIALNET_OVER_EVENT\n");
		//协议解析
		zbSocDoorlock_JiuLianProtocolAnalysis(epInfo,cmd);
	}

	log_debug("zbSocDoorlock_JiuLianSerialNetProcess--\n");	
}

/********************************************************************
*
*						久联门锁协议组包工具函数
*
********************************************************************/
static void zbSocDoorlock_SetDoorLockPkt(hostCmd *cmds,uint16_t cmd,uint8_t *data,uint16_t datalen)
{
	log_debug("zbSocDoorlock_SetDoorLockPkt++!\n");

	uint16_t fcs=0;
	uint16_t cnt;

	cmdSet8bitVal(cmds, JIULIAN_PROTOCOL_SOF);	//起始位
	cmdSet16bitVal_lh(cmds, cmd); 				//设置命令字
	cmdSet8bitVal(cmds, JIULIAN_PROTOCOL_ST);	//状态字
	
    cmdSet8bitVal(cmds, datalen);				//数据长度

	if((data != NULL)&&(datalen > 0))
		cmdSetStringVal(cmds, data, datalen);	//数据位

    for(cnt=0; cnt<cmds->idx; cnt++)
    {
        fcs += (cmds->data[cnt]&0x00ff);
    }

    cmds->data[cmds->idx++] = (uint8_t)(fcs & 0x00ff);

	log_debug("zbSocDoorlock_SetDoorLockPkt--\n");
}


/*********************************************************************
* @fn          zbSocDoorlock_TimerReportCb
*
* @brief       定时上报门锁的状态
*
* @param       reportdata - 门锁状态
*
* @return      bool
*/

static void zbSocDoorlock_TimerReportCb(void *args)
{
	log_debug("zbSocDoorlock_TimerReportCb++\n");
	JiuLianReportData_t *reportdata = (JiuLianReportData_t*)args;
	SRPC_DoorLockCtrlIndCB(reportdata->epinfo,reportdata->result,reportdata->doorlockstu,reportdata->optType,reportdata->userid,0x00,NULL);
	vdevListSetDoorState(reportdata->epinfo, reportdata->doorlockstu);
	log_debug("zbSocDoorlock_TimerReportCb--\n");
}

/*********************************************************************
* @fn          zbSocDoorlock_StartTimerReport
*
* @brief       定时上报门锁的状态
*
* @param       reportdata - 门锁状态
*
* @return      bool
*/

bool zbSocDoorlock_StartTimerReport(JiuLianReportData_t *reportdata)
{
	int ret = 0;

	log_debug("zbSocDoorlock_StartTimerReport++\n");

	if(JiuLianReportTimer == NULL)
		JiuLianReportTimer = tu_evtimer_new(main_base_event_loop);

	if(JiuLianReportTimer == NULL)
		return false;
		
	ret =tu_set_evtimer(JiuLianReportTimer, CONNECT_REPORT_TIME,ONCE,zbSocDoorlock_TimerReportCb,reportdata);
	
	if(ret == -1)
	{
		log_debug("tu_set_evtimer failed\n");
		tu_evtimer_free(JiuLianReportTimer);
		JiuLianReportTimer = NULL;
		return false;
	}
	log_debug("zbSocDoorlock_StartTimerReport--\n");
	return true;
}

/*********************************************************************/

