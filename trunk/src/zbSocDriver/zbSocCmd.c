/**************************************************************************************************
 * Filename:       zbSocCmd.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-11-30,10:03)    :   Create the file.
 *
 *
 *************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "Zigbee_device_Heartbeat_Manager.h"
#include "zbSocUart.h"
#include "zbSocCmd.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "errorCode.h"
#include "interface_devicelist.h"
#include "logUtils.h"
#include "PackageUtils.h"
#include "interface_srpcserver.h"
#include "One_key_match.h"
#include "zbSocMasterControl.h"
#include "interface_vDeviceList.h"
#include "Out_Of_Power.h"
#include "mt_zbSocCmd.h"
#include "event_manager.h"

#include "doorlock_Yaotai.h"
#include "electricityMeter.h"
#include "waterMeter_xx.h"

#include "zbSocZcl.h"
/*********************************************************************
 * MACROS
 */
#define ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))

/*********************************************************************
 * CONSTANTS
 */


/************************************************************
 * TYPEDEFS
 */
typedef void (*zbSoc_DeviceStateReport_CallBack_t)(hostCmd *cmd,epInfo_t *epInfo);
typedef void (*zbSoc_DeviceHeartReport_CallBack_t)(hostCmd *cmd,epInfo_t *epInfo);
typedef void (*zbSoc_DeviceReadResp_CallBack_t)(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);

typedef struct
{
	uint16_t DeviceID;
	zbSoc_DeviceStateReport_CallBack_t 	StateReport_cb;
	zbSoc_DeviceReadResp_CallBack_t 	ReadResp_cb;
	zbSoc_DeviceHeartReport_CallBack_t 	HeartReport_cb;
}zbSocReportCallbacks_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8_t zbTransSeqNumber = 0;

/*********************************************************************
 * LOCAL VARIABLES
 */

void zbSoc_DoorLockStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_PowerSwitchStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_SwitchOnOffStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_EnvtSensorStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_AlarmSensorStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_InfdSensorStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_LXSensorStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_AirControlSensorStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_CurtainSensorStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_NewCurtainSensorStateReport(hostCmd *cmd,epInfo_t *epInfo);

void zbSoc_DoorLockReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_PowerSwitchReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_SwitchOnOffReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_EnvtSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_AlarmSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_InfdSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_LXSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_AirControlSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_CurtainSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_NewCurtainSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);




void zbSoc_DoorLockHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_PowerSwitchHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_SwitchOnOffHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_EnvtSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_AlarmSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_InfdSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_LXSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_AirControlSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_CurtainSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_NewCurtainSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo);



void zbSoc_MeterStateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_MeterReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_MeterHeartReport(hostCmd *cmd,epInfo_t *epInfo);


void zbSoc_IAS_ZONE_StateReport(hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_IAS_ZONE_ReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo);
void zbSoc_IAS_ZONE_HeartReport(hostCmd *cmd,epInfo_t *epInfo);


void zclDiagnostic_ProcessDiagnosticMsg(epInfo_t *epInfo,uint16_t clusterID,uint16_t attrid,uint8_t datatype,hostCmd *cmd);


static zbSocReportCallbacks_t zbSocStateReportCallbacks[]=
{
#if DEVICE_LIWEI_DOOR_SUPPERT
	{
	ZB_DEV_LEVEL_DOORLOCK,	
	zbSoc_DoorLockStateReport,
	zbSoc_DoorLockReadResp,
	zbSoc_DoorLockHeartReport
	},
#endif
	{
	ZB_DEV_ONOFF_DOORLOCK,
	zbSoc_DoorLockStateReport,
	zbSoc_DoorLockReadResp,
	zbSoc_DoorLockHeartReport
	},
	{
	ZCL_HA_DEVICEID_FXZB_DOORLOCK_YAOTAI,
	zbSoc_DoorLockStateReport,
	zbSoc_DoorLockReadResp,
	zbSoc_DoorLockHeartReport
	},
	{
	ZCL_HA_DEVICEID_FXZB_DOORLOCK_YGS,
	zbSoc_DoorLockStateReport,
	zbSoc_DoorLockReadResp,
	zbSoc_DoorLockHeartReport
	},
	{
	ZCL_HA_DEVICEID_FXZB_DOORLOCK_JIULIAN,
	zbSoc_DoorLockStateReport,
	zbSoc_DoorLockReadResp,
	zbSoc_DoorLockHeartReport
	},
	{
	ZB_DEV_POWER_SWITCH,
	zbSoc_PowerSwitchStateReport,
	zbSoc_PowerSwitchReadResp,
	zbSoc_PowerSwitchHeartReport
	},
	{
	ZB_DEV_ONOFF_PLUG,
	zbSoc_SwitchOnOffStateReport,
	zbSoc_SwitchOnOffReadResp,
	zbSoc_SwitchOnOffHeartReport
	},
	{
	ZB_DEV_ONOFF_SWITCH,
	zbSoc_SwitchOnOffStateReport,
	zbSoc_SwitchOnOffReadResp,
	zbSoc_SwitchOnOffHeartReport
	},
	{
	ZB_DEV_TEMP_HUM,
	zbSoc_EnvtSensorStateReport,
	zbSoc_EnvtSensorReadResp,
	zbSoc_EnvtSensorHeartReport
	},
	{
	ZB_DEV_DOOR_SENSOR,
	zbSoc_AlarmSensorStateReport,
	zbSoc_AlarmSensorReadResp,
	zbSoc_AlarmSensorHeartReport
	},
	{
	ZB_DEV_INFRARED_BODY_SENSOR,
	zbSoc_AlarmSensorStateReport,
	zbSoc_AlarmSensorReadResp,
	zbSoc_AlarmSensorHeartReport
	},
	{
	ZB_DEV_IRC_LEARN_CTRL,
	zbSoc_InfdSensorStateReport,
	zbSoc_InfdSensorReadResp,
	zbSoc_InfdSensorHeartReport
	},
	{
	ZB_DEV_MASTER_CONTROL,
	zbSoc_LXSensorStateReport,
	zbSoc_LXSensorReadResp,
	zbSoc_LXSensorHeartReport
	},
	{
	ZB_DEV_CENTRAL_AIR,
	zbSoc_AirControlSensorStateReport,
	zbSoc_AirControlSensorReadResp,
	zbSoc_AirControlSensorHeartReport
	},
	{
	ZB_DEV_WIN_CURTAIN,
	zbSoc_CurtainSensorStateReport,
	zbSoc_CurtainSensorReadResp,
	zbSoc_CurtainSensorHeartReport
	},
	{
	ZB_DEV_METER_ENERGY_DEVICE,		//智能电表
	zbSoc_MeterStateReport,
	zbSoc_MeterReadResp,
	zbSoc_MeterHeartReport
	},
    {
	ZB_DEV_WATER_METER ,		//智能电表
	waterMeter_StateReport,
	waterMeter_ReadResp,
	waterMeter_HeartReport,
	},
		/*窗帘控制器*/
	{
		ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE,
		zbSoc_NewCurtainSensorStateReport,
		zbSoc_NewCurtainSensorReadResp,
		zbSoc_NewCurtainSensorHeartReport
	},
	{
		ZCL_HA_DEVICEID_WINDOW_COVERING_DEVICE_FXKJ,
		zbSoc_NewCurtainSensorStateReport,
		zbSoc_NewCurtainSensorReadResp,
		zbSoc_NewCurtainSensorHeartReport
	},
	{
	ZCL_HA_DEVICEID_IAS_ZONE,
	zbSoc_IAS_ZONE_StateReport,
	zbSoc_IAS_ZONE_ReadResp,
	zbSoc_IAS_ZONE_HeartReport,
	},
	{
	0x00,NULL,NULL,NULL},
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
//static void zbSocCmdSend(uint8_t* buf, uint8_t len);

/*********************************************************************
 * API FUNCTIONS
 */

/************************************************************************
* 函数名 :makeMsgEnder(FS_uint8 data)
* 描述   :  设置数据包的包尾数据
* 输入   ：*cmd:数据包指针,dir 为数据包的方向
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t zbMakeMsgEnder(hostCmd *cmd)
{
    cmd->data[SOC_MSG_LEN_POS] = ((cmd->idx-4)&0xff);
    cmd->data[SOC_MSG_DATALEN_POS] = ((cmd->idx-SOC_MSG_DATALEN_POS-1)&0xff);
    cmdSetFCS(cmd);
    return true;
}

/************************************************************************
* 函数名 :zbSoc_RevertFactorySettingCmd(uint8_t status)
* 描述   :   恢复协调器出厂设置命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_RevertFactorySettingCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;

    log_debug("zbSoc_RevertFactorySettingCmd !\n");

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER); 	//可以Reserved
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint	//可以Reserved
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, afAddrBroadcast);//Addr mode			//可以Reserved
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control					//可以Reserved
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number //可以Reserved
    cmdSet8bitVal(&cmd, APP_ZCL_CMD_REVERT_FACTORY_SETTING);//cmd ID
    cmdSet16bitVal_lh(&cmd, 0x0);//ZCL CLUSTER ID   				//可以Reserved

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    usleep(30);
}


/************************************************************************
* 函数名 :zbSoc_RevertOneDevFactorySettingCmd(uint8_t status)
* 描述   :   恢复单个设备至出厂设置命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_RevertOneDevFactorySettingCmd(uint16_t dstAddr,uint8_t endpoint)

//void zbSoc_RevertOneDevFactorySettingCmd(uint16_t dstAddr,uint8_t* ieee)
{
    hostCmd cmd;
    cmd.idx=0;

    log_debug("zbSoc_RevertOneDevFactorySettingCmd +++++++++++++++!\n");

		
    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给指定设备
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, afAddr16Bit);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, APP_ZCL_CMD_BASIC_REVERT_FACTORY_DEFAULT);//cmd ID
	
		/*
		cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_ZDO);//CMD0
    cmdSet8bitVal(&cmd, ZDO_MGMT_LEAVE_REQ);//CMD1
    cmdSet16bitVal_lh(&cmd, dstAddr);
		cmdSetStringVal_lh(&cmd, ieee, IEEE_ADDR_SIZE);
		cmdSet8bitVal(&cmd,0x00);
		*/
		
		zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);

    log_debug("zbSoc_RevertOneDevFactorySettingCmd ----------------!\n");

}

/************************************************************************
* 函数名 :zbSoc_GetCoordVersionCmd(void)
* 描述   :   获取协调器固件版本
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_GetCoordVersionCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;

    log_debug("zbSoc_GetCoordVersionCmd++\n");

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER);
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, afAddrBroadcast);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, APP_ZCL_CMD_QUERY_DEVICEID);//cmd ID
	cmdSet16bitVal(&cmd, 0x00);//Reserved	

    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
	
    //wait for message to be consumed
    usleep(30);
}


/************************************************************************
* 函数名 :zbSoc_SetCoordResetCmd(void)
* 描述   :   复位协调器
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_SetCoordResetCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;
	
    log_debug("zbSoc_GetCoordVersionCmd++\n");

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER);
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, afAddrBroadcast);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, APP_ZCL_CMD_COOR_SYSTEM_RESET);//cmd ID
	cmdSet16bitVal(&cmd, 0x00);//Reserved	

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);
}

//开放网络命令
//duration:
//0x00:关闭网络
//0xff:常开
//0x01~0xfe开放时间
/*****************************************************************************
 * 函 数 名  : zbSoc_Permit_Join_Req
 * 负 责 人  : Edward
 * 创建日期  : 2016年6月15日
 * 函数功能  : 协调器及路由设备开放网络
 * 输入参数  : uint8_t duration  开放网络时间
 			   	0x00:关闭网络
				0xff:常开
				0x01~0xfe开放时间
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void zbSoc_Permit_Join_Req(uint8_t duration)
{
	
	hostCmd cmd;
	cmd.idx=0;

	log_debug("zbSoc_Permit_Join_Req: duration %ds\n", duration);

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_ZDO);//CMD0
    cmdSet8bitVal(&cmd, 0x36);//CMD1
    cmdSet8bitVal(&cmd, afAddrBroadcast);//addr mode
	cmdSet16bitVal_lh(&cmd,MT_NWKADDR_BROADCAST_ALL_ROUTER);
	cmdSet8bitVal(&cmd, duration);//间隔时间
	cmdSet8bitVal(&cmd, 0x01);

	zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
    
}

/************************************************************************
* 函数名 :zbSoc_SetGenOnOffState(uint8_t state, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   发送On/Off命令请求
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_SetGenOnOffState(uint8_t state, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_ON_OFF);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, state);//ZCL COMMAND ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}


void zbSoc_SetGenOnOffDefaultState(uint8_t state, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, 0x09);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number

	cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);
	cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_KG_OPERATION);//Attr ID
	cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, 0x02);//date length
	cmdSet8bitVal(&cmd, endpoint);
	cmdSet8bitVal(&cmd, state);
	
    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

void zbSoc_getGenOnOffDefaultState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;
	
    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_KG_DEFAULT_STATUS);//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}



/************************************************************************
* 函数名 :zbSoc_QueryLightValueState(uint8_t state, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   查询开关状态值
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_QueryLightValueState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);					//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);		//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);			//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_ON_OFF);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);					//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);			//Addr mode
    cmdSet8bitVal(&cmd, 0x00);				//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);		//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_ON_OFF);	//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :zbSoc_QuerySwitchSocketValueState(uint8_t state, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   查询开关状态值
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_QuerySwitchSocketValueState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,uint8_t attrs)
{
    hostCmd cmd;
    cmd.idx =0;
	
	uint16_t attrid = 0;
	
//	ATTRID_BASIC_CARACITY_VALUE(0x0017) 	功率
//	ATTRID_BASIC_POWER_VALUE   (0x0018)		电量
//	ATTRID_BASIC_VOLTAGE_VALUE (0x0019)		电压
//	ATTRID_BASIC_CURRENT_VALUE (0x0021)		电流

	switch(attrs)
	{
		case 0:
			attrid = ATTRID_BASIC_CARACITY_VALUE;
		break;
		case 1:
			attrid = ATTRID_BASIC_POWER_VALUE;
		break;
		case 2:
			attrid = ATTRID_BASIC_VOLTAGE_VALUE;
		break;
		case 3:
			attrid = ATTRID_BASIC_CURRENT_VALUE;
		break;	
		default:
			attrid = ATTRID_BASIC_CURRENT_VALUE;
		break;
	}

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);					//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);		//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);			//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);					//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);			//Addr mode
    cmdSet8bitVal(&cmd, 0x00);				//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);		//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, attrid);	//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :zbSoc_QueryDoorLockPowerValueState(uint8_t state, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   查询门锁电量值
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_QueryDoorLockPowerValueState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_POWER_VALUE);//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

void zbSoc_QueryDoorLockSpeedValueState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_MOTOR_SPEED_CHECK);//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

void zbSoc_SetDoorLockSpeedValueState(uint16_t dstAddr, uint8_t endpoint, uint16_t mTimes,uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;
	
    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_MOTOR_SPEED);//Attr ID
	cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

	cmdSet8bitVal(&cmd,0x03);		//数据长度
	cmdSet8bitVal(&cmd,0x01);		//设置电机
	cmdSet16bitVal_lh(&cmd,mTimes);	//时间参数
	
    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_QueryPowerSwitchValueState(uint8_t state, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   查询取电开关电量值
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_QueryPowerSwitchValueState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_MS_SIMPLE_METER);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_MS_ELECTRICAL_METER_MEASURED_VALUE);//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

void zbSoc_QueryDeviceVerion(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;
    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);									//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);	//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);				//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);							//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd,ZCL_CLUSTER_ID_GEN_BASIC);		//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);									//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);							//Addr mode
    cmdSet8bitVal(&cmd, 0x00);								//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);				//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);						//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_ZCL_VERSION);		//Attr ID
    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :zbSoc_SetTempIntervalReportReq(uint16_t intervalTime, uint16_t dstAddr,uint8_t endpoint, uint8_t addrMode)
* 描述   :   配置温度上报的间隔时间
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
//void zbSoc_SetTempIntervalReportReq(uint16_t intervalTime, uint16_t dstAddr,uint8_t endpoint, uint8_t addrMode)
//{
//    hostCmd cmd;
//    cmd.idx =0;

//    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
//    cmdSet8bitVal(&cmd, 0);//len 预留位
//    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
//    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
//    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
//    //发送给所有的路由设备,都开放网络
//    cmdSet16bitVal_lh(&cmd, dstAddr);
//    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
//    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT);//ZCL CLUSTER ID
//    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
//    cmdSet8bitVal(&cmd, addrMode);//Addr mode
//    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
//    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
//    cmdSet8bitVal(&cmd, ZCL_CMD_CONFIG_REPORT);//ZCL COMMAND ID

//    cmdSet8bitVal(&cmd, 0x00);//Direction
//    cmdSet16bitVal_lh(&cmd, ATTRID_MS_REPORT_INTERVAL_VALUE);//Attr ID
//    cmdSet8bitVal(&cmd, ZCL_DATATYPE_UINT8);//Data Type
//    cmdSet16bitVal_lh(&cmd, intervalTime);//minReportIn
//    cmdSet16bitVal_lh(&cmd, 0xffff);//maxReportIn

//    zbMakeMsgEnder(&cmd);

//    zbSocCmdSend(cmd.data,cmd.idx);
//}


/************************************************************************
* 函数名 :zbSoc_SetDevValidReq(bool onoff, uint16_t dstAddr,uint8_t endpoint, uint8_t addrMode)
* 描述   :   配置通用节点设备启用/禁用的接口
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_SetDevValidReq(bool onoff, uint16_t dstAddr,uint8_t endpoint, uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;
#if 0
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

//  cmdSet8bitVal(&cmd, 0x00);//Direction
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_DEVICE_ENABLED);//Attr ID
//  cmdSet8bitVal(&cmd, ZCL_DATATYPE_BOOLEAN);//Data Type
	cmdSet8bitVal(&cmd, ZCL_DATATYPE_UINT8);//Data Type

    cmdSet8bitVal(&cmd, onoff);
#else
	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_ON_OFF);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, onoff);//ZCL COMMAND ID
#endif
    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_IRCDevCtrlLearnModeCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t ctrlcmd)
* 描述   :   控制红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_IRCDevCtrlLearnModeCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint16_t ctrlcmd)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, 0x2);
    cmdSet16bitVal(&cmd, ctrlcmd);//红外学习模式命令

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_IRCDevCtrlSetLearnAddrCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t cmdaddr)
* 描述   :   控制红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_IRCDevCtrlSetLearnAddrCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t cmdaddr)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, 0x3);
    cmdSet16bitVal(&cmd, IRC_CTRL_SET_LEARN_ADDR);//红外学习模式命令
    cmdSet8bitVal(&cmd, cmdaddr);

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_IRCDevCtrlSendAddrCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t cmdaddr)
* 描述   :   控制红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_IRCDevCtrlSendAddrCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t cmdaddr)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, 0x3);
    cmdSet16bitVal(&cmd, IRC_CTRL_SEND_CMD_ADDR);//红外学习模式命令
    cmdSet8bitVal(&cmd, cmdaddr);

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_RemoteIRCDevCtrlLearnModeCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t ctrlcmd)
* 描述   :   控制远程红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_RemoteIRCDevCtrlLearnModeCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint16_t ctrlcmd)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_LONG_OCTET_STR);//Data Type

    cmdSet16bitVal_lh(&cmd, 0x0002);
    cmdSet16bitVal(&cmd, ctrlcmd);//红外学习模式命令

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_IRC_RemoteIModeCtrlCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t ctrlcmd)
* 描述   :   控制远程红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_IRC_RemoteIModeCtrlCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t ctrlcmd)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, 0x01);
    cmdSet8bitVal(&cmd, ctrlcmd);//红外学习模式命令

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_RemoteIRCDevCtrlCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t ctrlcmd)
* 描述   :   控制远程红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_RemoteIRCDevCtrlCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint16_t ircDataLen,uint8_t *ircData)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

//    cmdSet16bitVal_lh(&cmd, ircDataLen);//红外学习模式命令
	cmdSet8bitVal(&cmd,ircDataLen+1);
	cmdSet8bitVal(&cmd,0x03);//发送远程红外码
    cmdSetStringVal(&cmd,ircData,ircDataLen);//红外学习模式命令

    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :zbSoc_IrcRemoteDevCtrlCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t ctrlcmd)
* 描述   :   控制远程红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_IrcRemoteDevCtrlCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t ircDataLen,uint8_t *ircData)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

//  cmdSet16bitVal_lh(&cmd, ircDataLen);//红外学习模式命令
	cmdSet8bitVal(&cmd,ircDataLen+1);
	cmdSet8bitVal(&cmd,0x03);//发送远程红外码
    cmdSetStringVal(&cmd, ircData,ircDataLen);//红外学习模式命令
	
    zbMakeMsgEnder(&cmd);
	
    zbSocCmdSend(cmd.data,cmd.idx);
    usleep(300000);
    log_debug("zbSoc_IrcRemoteDevCtrlCmd++\n");
}


/************************************************************************
* 函数名 :zbSoc_CurtainDevCtrlCmdReq( uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   发送窗帘控制器控制命令
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_CurtainDevCtrlCmdReq(uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_ON_OFF);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, cmdvalue);//ZCL COMMAND ID    
    							  //COMMAND_OPEN(0x04开)
    							  //COMMAND_CLOSE(0x05关)
    							  //COMMAND_STOP(0x06暂停)

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_CurtainDevCtrlCmdReq( uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   发送窗帘控制器控制命令
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_NewCurtainDevCtrlCmdReq(uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, cmdvalue);//ZCL COMMAND ID    
    							  //COMMAND_OPEN(0x00开)
    							  //COMMAND_CLOSE(0x01关)
    							  //COMMAND_STOP(0x02暂停)

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_CurtainDevCtrlCmdReq( uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   发送窗帘控制器控制命令
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_NewCurtainGetPercentCmdReq(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
	hostCmd cmd;
    cmd.idx =0;
    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);									//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);	//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);				//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);							//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd,ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING);		//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);									//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);							//Addr mode
    cmdSet8bitVal(&cmd, 0x00);								//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);				//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);						//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_CLOSURES_CURRENT_POSITION_LIFT_PERCENTAGE);		//Attr ID
    
    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}


void zbSoc_NewCurtainGetStatusCmdReq(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
	hostCmd cmd;
    cmd.idx =0;
    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);									//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);	//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);				//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);							//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd,ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING);		//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);									//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);							//Addr mode
    cmdSet8bitVal(&cmd, 0x00);								//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);				//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);						//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_CLOSURES_WINDOW_COVERING_STATUS);		//Attr ID
    
    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_CurtainDevPercentageCmdReq( uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   发送窗帘控制器百分比控制命令
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_CurtainDevPercentageCmdReq(uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
	hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_ON_OFF);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number

	cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);
	cmdSet16bitVal_lh(&cmd, ATTRID_MOVE_LEVEL);//Attr ID
	cmdSet8bitVal(&cmd, ZCL_DATATYPE_UINT8);//Data Type
	cmdSet8bitVal(&cmd, cmdvalue);
	
    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    
}

/************************************************************************
* 函数名 :zbSoc_CurtainDevPercentageCmdReq( uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   发送窗帘控制器百分比控制命令
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_NewCurtainDevPercentageCmdReq(uint8_t cmdvalue,uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
	hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number

	cmdSet8bitVal(&cmd, COMMAND_CLOSURES_GO_TO_LIFT_PERCENTAGE);
	cmdSet8bitVal(&cmd, cmdvalue);
	
    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_RemoveGroupMemberCmd(uint16_t groupId, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   添加组成员命令
* 输入   :  groupId - Group ID of the Scene.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_AddGroupMemberCmd(uint16_t groupId, uint16_t dstAddr, uint8_t endpoint,uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给指定的节点设备
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_GROUPS);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, COMMAND_GROUP_ADD);//cmd ID
    cmdSet16bitVal_lh(&cmd, groupId);//ZCL Group ID

    cmdSet16bitVal_lh(&cmd, 0x00);//Null group name - Group Name not pushed to the devices

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

}

/************************************************************************
* 函数名 :zbSoc_RemoveGroupMemberCmd(uint16_t groupId, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
* 描述   :   移除组成员命令
* 输入   :  groupId - Group Id
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_RemoveGroupMemberCmd(uint16_t groupId, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    if(dstAddr== 0xffff || endpoint == 0xff)
    {
        dstAddr=groupId;
        addrMode=0x01;
    }

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给指定的节点设备
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_GROUPS);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, COMMAND_GROUP_GET_REMOVE);//cmd ID
    cmdSet16bitVal_lh(&cmd, groupId);//ZCL Group ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :zbSoc_RemoveSceneMemberCmd(uint16_t groupId, uint8_t sceneId, uint16_t dstAddr,
                                                                uint8_t endpoint, uint8_t addrMode)
* 描述   :   移除场景成员
* 输入   :  groupId - Group Id
                    sceneId - Scene ID of the Scene.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_RemoveSceneMemberCmd(uint16_t groupId, uint8_t sceneId, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    if(dstAddr== 0xffff || endpoint == 0xff)
    {
        dstAddr=groupId;
        addrMode=0x01;
    }

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给指定的节点设备
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_SCENES);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, COMMAND_SCENE_GET_REMOVE);//cmd ID
    cmdSet16bitVal_lh(&cmd, groupId);//ZCL Group ID
    cmdSet8bitVal(&cmd, sceneId++);//Scene ID

    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}


/*********************************************************************
 * @fn      zbSoc_RecallSceneCmd
 *
 * @brief   Recall Scene.
 *
 * @param   groupId - Group ID of the Scene.
 * @param   sceneId - Scene ID of the Scene.
 * @param   dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
 * @param   endpoint - endpoint of the Light.
 * @param   addrMode - Unicast or Group cast.

 * @return  none
 */
void zbSoc_RecallSceneCmd(uint16_t groupId, uint8_t sceneId, uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    log_debug("zbSoc_RecallSceneCmd++\n");

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    cmdSet16bitVal_lh(&cmd, dstAddr); //发送给指定的节点设备
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_SCENES);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, COMMAND_SCENE_RECALL);//cmd ID
    cmdSet16bitVal_lh(&cmd, groupId);//ZCL Group ID
    cmdSet8bitVal(&cmd, sceneId++);//Scene ID

    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}




/*****************************************************************************
 * 函 数 名  : zbSoc_SetCentralAirCmdReq
 * 负 责 人  : Edward
 * 创建日期  : 2016年3月28日
 * 函数功能  : 设置中央空调控制器
 * 输入参数  : uint16_t dstAddr     设备短地址
               uint8_t endpoint     设备端口号
               uint8_t addrMode     发送模式
               uint16_t ircDataLen  数据长度
               uint8_t *ircData     数据内容
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void zbSoc_SetCentralAirCmdReq(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint16_t airDataLen,uint8_t *airData)
{
	hostCmd cmd;
	cmd.idx =0;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);//len 预留位
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
	cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
	cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
	cmdSet16bitVal_lh(&cmd, dstAddr);//发送给所有的路由设备,都开放网络
	cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
	cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
	cmdSet8bitVal(&cmd, 0);//DataLen 预留位
	cmdSet8bitVal(&cmd, addrMode);//Addr mode
	cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
	cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
	cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

	cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_KTWKQ_OPER);//Attr ID
	cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

	cmdSet8bitVal(&cmd,airDataLen);
	cmdSetStringVal(&cmd,airData,airDataLen);

	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
	
}

void zbSoc_getHumitureState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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
	cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID
	cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_TEMP_HUM_VALUE);//Attr ID

	zbMakeMsgEnder(&cmd);

	zbSocCmdSend(cmd.data,cmd.idx);
}

/*****************************************************************************
 * 函 数 名  : zclDiagnostic_GetLastMessageRssi
 * 负 责 人  : Edward
 * 创建日期  : 2017年6月23日
 * 函数功能  : 获取设备相对于其父节点的RSSI
 * 输入参数  : uint16_t dstAddr     设备短地址
               uint8_t endpoint     设备端口号
               uint8_t addrMode     发送模式
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/

void zclDiagnostic_GetLastMessageRssi(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
	hostCmd cmd;
	cmd.idx =0;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);//len 预留位
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
	cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
	cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
	cmdSet16bitVal_lh(&cmd, dstAddr);//发送给所有的路由设备,都开放网络
	cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
	cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_HA_DIAGNOSTIC);//ZCL CLUSTER ID
	cmdSet8bitVal(&cmd, 0);//DataLen 预留位
	cmdSet8bitVal(&cmd, addrMode);//Addr mode
	cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
	cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
	cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID

	cmdSet16bitVal_lh(&cmd, ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI);//Attr ID

	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
}


/*****************************************************************************
 * 函 数 名  : zclDiagnostic_GetLastMessageLqi
 * 负 责 人  : Edward
 * 创建日期  : 2017年6月23日
 * 函数功能  : 获取设备相对于其父节点的LQI
 * 输入参数  : uint16_t dstAddr     设备短地址
               uint8_t endpoint     设备端口号
               uint8_t addrMode     发送模式
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/

void zclDiagnostic_GetLastMessageLqi(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
	hostCmd cmd;
	cmd.idx =0;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);//len 预留位
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
	cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
	cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
	cmdSet16bitVal_lh(&cmd, dstAddr);//发送给所有的路由设备,都开放网络
	cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
	cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_HA_DIAGNOSTIC);//ZCL CLUSTER ID
	cmdSet8bitVal(&cmd, 0);//DataLen 预留位
	cmdSet8bitVal(&cmd, addrMode);//Addr mode
	cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
	cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
	cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID

	cmdSet16bitVal_lh(&cmd, ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI);//Attr ID

	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
}


//Diagnostic数据处理
void zclDiagnostic_ProcessDiagnosticMsg(epInfo_t *epInfo,uint16_t clusterID,uint16_t attrid,uint8_t datatype,hostCmd *cmd)
{
	uint8_t data = 0;
	if(epInfo == NULL)
		return;

	switch (attrid)
	{
		case ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI:
			if((datatype == ZCL_DATATYPE_INT8)&&(cmd != NULL))
			{
				cmdGet8bitVal(cmd, &data);
				vdevListUpdateDeviceRssi(epInfo,data);
			}
		break;
		
		case ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI:
			//pass
		break;
		default:break;
	}
	
}

//底层事件触发处理接口
void zbSoc_ProcessEvent(epInfo_t *epinfo,uint8_t state)
{
	log_debug("zbSoc_ProcessEvent++\n");

	event_ProcessActionEvent(epinfo,state);

	log_debug("zbSoc_ProcessEvent--\n");
}

/************************************************************************
* 函数名 :zbSoc_DevRegisterReporting(epInfoExtended_t *epInfoEx)
* 描述   :   处理节点设备的注册信息并上报给服务器
* 输入   ：
* 输出   ：无
* 返回   ：无
************************************************************************/
void zbSoc_DevRegisterReporting(epInfoExtended_t *epInfoEx)
{
    log_debug("zbSoc_DevRegisterReporting, deviceID=%04x\n",epInfoEx->epInfo->deviceID);

    SRPC_ComDevRegisterInd(epInfoEx->epInfo->IEEEAddr,epInfoEx->epInfo->deviceID,epInfoEx->epInfo->endpoint);
}

void zbSoc_DoorLockStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

    uint16_t attrID;
    uint8_t dataType;

	log_debug("zbSoc_DoorLockStateReport++\n");
	switch(epInfo->deviceID)
	{
#if DEVICE_LIWEI_DOOR_SUPPERT

	//智能门锁--力维
	case ZB_DEV_LEVEL_DOORLOCK:
	{
		doorLevel_ReviceMsgProcess_Event(epInfo,cmd);
		//zbDevDoorLock_StatusReportProcess(epInfo,cmd,rssi);
	}
	break;
#endif

	case ZCL_HA_DEVICEID_FXZB_DOORLOCK_YAOTAI:
		doorLockYT_MessageHandle_Event(epInfo,cmd);
	break;

	case ZCL_HA_DEVICEID_FXZB_DOORLOCK_YGS:
		zbSoc_DoorLock_YgsSerialMsg_Event(epInfo,cmd);
	break;

	case ZCL_HA_DEVICEID_FXZB_DOORLOCK_JIULIAN:
		//久联门锁协议处理
		zbSocDoorlock_JiuLianSerialNetProcess(epInfo,cmd);
	break;

	//智能门锁
	case ZB_DEV_ONOFF_DOORLOCK:
     zbSoc_DoorLock_modulesIncomming(epInfo,cmd);
	break;
	default:break;
	}
	log_debug("zbSoc_DoorLockStateReport--\n");
}


void zbSoc_PowerSwitchStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	  ASSERT(cmd != NULL && epInfo != NULL);
	  uint8_t status;
	  uint16_t attrID;
	  uint8_t dataType;
	  
	  if(epInfo->deviceID != ZB_DEV_POWER_SWITCH)
	    return ;
	 
	  cmdGet16bitVal_lh(cmd, &attrID);
	  cmdGet8bitVal(cmd, &dataType);//get data type;
	  cmdGet8bitVal(cmd, &status);

	  //接收到返回，关闭重发机制
	  zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

	  if(epInfo->registerflag == true)
	  {
		//控制取电开关的状态变化上报
		SRPC_PowerSwitchCtrlInd(epInfo->IEEEAddr,epInfo->endpoint,status,epInfo->onlineDevRssi);
		zbSoc_ProcessEvent(epInfo,status);
		Out_Of_Power_Stop();
	  }
	  
	  //更新设备状态
	  vdevListSetPowerSwitchState(epInfo,status);
}


void zbSoc_SwitchOnOffStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint8_t status;
	uint16_t attrID;
  	uint8_t dataType;
  	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType); //get data type;
	cmdGet8bitVal(cmd, &status);
	
	if(attrID == ZB_ATTID_PLUG_ONOFF)
	{
		//接收到返回，关闭重发机制
		zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

		if(epInfo->registerflag== true)
		{
			//开关节点的开关状态变化上报
		    SRPC_SwitchStateInd(epInfo->IEEEAddr,epInfo->endpoint,status,epInfo->onlineDevRssi);
		    Out_Of_Power_Stop();
			zbSoc_ProcessEvent(epInfo,status);
		}

		//更新设备状态值
		vdevListSetSwitchState(epInfo,status);
	}
	else if(attrID == ATTRID_BASIC_KG_OPERATION) //设置开关状态默认上报
	{
		if(status & 0x01)
		{
			SRPC_SwitchDefaultStateInd(epInfo->IEEEAddr,epInfo->endpoint,0x01);
		}
		else
		{
			SRPC_SwitchDefaultStateInd(epInfo->IEEEAddr,epInfo->endpoint,0x00);
		}
	}
	else
	{
		log_debug("[Error] Other attrID = %04x\n",attrID);
	}
}

//环境传感器:温度，湿度
void zbSoc_EnvtSensorStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint16_t attrID;
  	uint8_t dataType;

  	uint8_t ret = 0;
	uint16_t temp = 0,hum = 0;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);//get data type;

	if(attrID == ZB_ATTID_TEMPERATURE)
	{
	    cmdGet16bitVal_lh(cmd, &temp);
	    vdevListSetSensertemp(epInfo,temp);
	}
	else if(attrID == ZB_ATTID_HUMIDITY)
	{
	    cmdGet16bitVal_lh(cmd, &hum);
	    vdevListSetSenserHumidity(epInfo,hum);
	    ret = 1;
	}
	else
	{
	    log_debug("[ERROR]TEMP_HUM Others attrID = %04x.\n",attrID);
	}

	if(ret)
	{
		hum = vdevListGetSenserHumidity(epInfo);
		temp = vdevListGetSensertemp(epInfo);
		
		if(epInfo->registerflag == true)
		{
			//需要调整
			SRPC_HumitureStateInd(epInfo->IEEEAddr,epInfo->endpoint,((temp&0x00ff)<<8|(hum&0x00ff)));
		}
	}
	
}

void zbSoc_AlarmSensorStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint16_t attrID;
  	uint8_t dataType;

	switch(epInfo->deviceID){
	case ZB_DEV_DOOR_SENSOR:
	{
		uint8_t sensorFlag;
		cmdGet16bitVal_lh(cmd, &attrID);
		cmdGet8bitVal(cmd, &dataType);//get data type;
		cmdGet8bitVal(cmd, &sensorFlag);//报警状态值

		log_debug("Alarm = %d\n",sensorFlag);
		log_debug("attrID = %d\n",attrID);	
		if(epInfo->registerflag == true)
		{
			if(attrID == ZB_ATTID_PLUG_ONOFF)
			{
				log_debug("State report\n");
		    	SRPC_ComAlarmStateInd(epInfo->IEEEAddr,epInfo->endpoint,epInfo->deviceID,sensorFlag);
				//启动离人断电检测
				if(sensorFlag == 0)
				{
					Out_Of_Power_Start(OUT_OF_POWER_TIME);
				}
				//执行场景或事件
				zbSoc_ProcessEvent(epInfo,sensorFlag);
			}
			else if(attrID == ATTRID_BASIC_POWER_VALUE)
			{
				//电量接口
				log_debug("power report\n");
				SRPC_ComAlarmPowerValueInd(epInfo->IEEEAddr,epInfo->endpoint,epInfo->deviceID,sensorFlag);
			}
		}
	}
	break;
	case ZB_DEV_INFRARED_BODY_SENSOR:
	{
			cmdGet16bitVal_lh(cmd, &attrID);
	    cmdGet8bitVal(cmd, &dataType);//get data type;

	    if(epInfo->registerflag == true)
	    {	
	    	//红外人体感应，检测信息上报
				if((attrID == ZB_ATTID_PLUG_ONOFF)&&(dataType==ZCL_DATATYPE_UINT8))
				{
					uint8_t attrData = 0;
					cmdGet8bitVal(cmd, &attrData);
					log_debug("State report %d\n",attrData);
		        	SRPC_ComAlarmStateInd(epInfo->IEEEAddr,epInfo->endpoint,epInfo->deviceID,attrData);

					//启动事件刷新定时器,当间隔90s没有上报就上报没人状态
					//PanalarmDevice_UpdateDeviceState(epInfo);
					
					Out_Of_Power_Stop();
					zbSoc_ProcessEvent(epInfo,attrData);
				}
			
				//红外人体感应电量上报
				if((attrID == ATTRID_BASIC_POWER_VALUE)&&(dataType==ZCL_DATATYPE_UINT8))
				{
					uint8_t powerValue = 0;
					cmdGet8bitVal(cmd,&powerValue);
					log_debug("IRC PowerValue = %d\n",powerValue);

					SRPC_ComAlarmPowerValueInd(epInfo->IEEEAddr,epInfo->endpoint,epInfo->deviceID,powerValue);
					vdevListSetPowerSwitchBattery(epInfo,powerValue);
				}
			
				//启动/禁止红外人体感应设备状态上报上报
				if((attrID == ATTRID_BASIC_IRSENSOR_ONOFF)&&(dataType==ZCL_DATATYPE_UINT8))
				{
					uint8_t mEable = 0;
					cmdGet8bitVal(cmd,&mEable);
					log_debug("IRC working state = %d\n",mEable);

					//接收到返回，关闭重发机制
		    		zblist_remove(epInfo->nwkAddr,epInfo->endpoint);
					
					SRPC_ComAlarmDeviceStateInd(epInfo->IEEEAddr,epInfo->endpoint,epInfo->deviceID,mEable);
					vdevListSetAlarmisEnable(epInfo,mEable);
				}
			}
	}
	break;
	
	default:break;
	}
  	
}

//红外转发器设备
void zbSoc_InfdSensorStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint16_t attrID;
  	uint8_t dataType;
	uint8_t status;

    uint8_t len;
#ifdef IRC_OLD_VERSION
    uint16_t cmdval;
#else
	uint8_t cmdval;
#endif

    cmdGet16bitVal_lh(cmd, &attrID);
    cmdGet8bitVal(cmd, &dataType);//get data type;
    //Attr Data
    cmdGet8bitVal(cmd, &len);

#ifdef IRC_OLD_VERSION
    cmdGet16bitVal(cmd, &cmdval);//命令值
    cmdGet8bitVal(cmd, &status);

    if(epInfo->registerflag == true)
    {
        switch(cmdval)
        {
            case IRC_CTRL_ENTER_LEARN_MODE:
                SRPC_EnterIRCLearnModeCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS);
            	break;
            case IRC_CTRL_EXIT_LEARN_MODE:
                SRPC_ExitIRCLearnModeCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS);
            	break;
            case IRC_CTRL_SET_LEARN_ADDR:
                SRPC_SetIRCLearnAddrCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS);
            	break;
            case IRC_CTRL_LEARN_STATE_IND:
            {
                uint8_t addr;
                cmdGet8bitVal(cmd, &addr);
                SRPC_IRCLearnRetInd(epInfo->IEEEAddr,epInfo->endpoint,addr);
            }
            break;
            case IRC_CTRL_SEND_CMD_ADDR:
            {
                SRPC_SendIRCCtrlCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS);
            }
            break;
            case IRC_REMOTE_CTRL_ENTER_LEARN_MODE:
                SRPC_EnterRemoteIRCLearnModeCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS);
                break;
            case IRC_REMOTE_CTRL_EXIT_LEARN_MODE:
                SRPC_ExitRemoteIRCLearnModeCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS);
                break;
            case IRC_REMOTE_LEARN_DATA_IND:
            {
                uint16_t dataLen;
                uint8_t   data[512]= {0};
                cmdGet16bitVal(cmd, &dataLen);
                cmdGetStringVal(cmd, data,dataLen);
                SRPC_RemoteIRCLearnRetInd(epInfo->IEEEAddr,epInfo->endpoint,dataLen,data);
            }
            break;
            case IRC_REMOTE_CTRL_SEND_CMD_DATA:
                SRPC_SendRemoteIRCCtrlCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS);
				
				break;
        }
        Out_Of_Power_Stop();
    }
#else
	cmdGet8bitVal(cmd, &cmdval);//命令值
    if(epInfo->registerflag == true)
    {
		switch(cmdval)
		{
			case 0x01:
			case 0x02:
			case 0x03:
			{
				uint8_t state = 0;
				cmdGet8bitVal(cmd,&state);
				log_debug("IRC = %d\n",state);

				//接收到返回，关闭重发机制
    			zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

				if(state == 0)
				{
				//	SRPC_RemoteIRCCfm(epInfo->IEEEAddr,endpoint,YY_STATUS_SUCCESS);
				}
				else
				{
				//	SRPC_RemoteIRCCfm(epInfo->IEEEAddr,endpoint,YY_STATUS_FAIL);
				}
			}
			break;
			case 0x04:
			{
				hostCmd Rdata;
				Rdata.idx = 0;
				
				static uint8_t   Segment_data1[100]= {0};
				static uint8_t   Segment_data2[100]= {0};
				static uint8_t   Segment_data3[100]= {0};
				static uint8_t   Segment_data1_length = 0;
				static uint8_t   Segment_data2_length = 0;
				static uint8_t   Segment_data3_length = 0;
				uint8_t index = 0;
				cmdGet8bitVal(cmd,&index);
				if(index == 1)
				{
					memset(Segment_data1,0,sizeof(Segment_data1));
					Segment_data1_length = len-2;
					cmdGetStringVal(cmd, Segment_data1,Segment_data1_length);
					IRC_Remote_Learn_Count = (IRC_Remote_Learn_Count&0xf)|0x01;
				}
				else if (index == 2)
				{
					memset(Segment_data2,0,sizeof(Segment_data3));
					Segment_data2_length = len-2;
					cmdGetStringVal(cmd, Segment_data2,Segment_data2_length);
					IRC_Remote_Learn_Count = (IRC_Remote_Learn_Count&0xf)|(0x01<<1);
				}
				else if (index == 3)
				{
					memset(Segment_data3,0,sizeof(Segment_data3));
					Segment_data3_length = len-2;
					cmdGetStringVal(cmd, Segment_data3,Segment_data3_length);
					IRC_Remote_Learn_Count = (IRC_Remote_Learn_Count&0xf)|(0x01<<2);
				}

				if (IRC_Remote_Learn_Count == 0x7)
				{
					IRC_Remote_Learn_Count = 0;
					cmdSetStringVal(&Rdata,Segment_data1,Segment_data1_length);
					cmdSetStringVal(&Rdata,Segment_data2,Segment_data2_length);
					cmdSetStringVal(&Rdata,Segment_data3,Segment_data3_length);
					log_debug("Length = %d\n",Rdata.idx);
					Rdata.size = Rdata.idx;

					log_debug_array(Rdata.data,Rdata.size,NULL);
					
					//请求原始红外码
					if(IRC_Remote_Learn_Device_Data_Type == 0)
					{
						SRPC_RemoteIRCLearnDataRetInd(epInfo->IEEEAddr,epInfo->endpoint,Rdata.idx,Rdata.data);
					}
					else
					{
						hostCmd dstCmd = {{0},0,0};
						OneKeyMatchDevice(&Rdata,&dstCmd);//一键匹配
						log_debug("CodeLen = %d\n",dstCmd.idx);
						SRPC_RemoteIrcCodeLibDataRetInd(epInfo->IEEEAddr,epInfo->endpoint,IRC_Remote_Learn_Device_Type,dstCmd.idx,dstCmd.data);
					}
				}
			}
			break;
		}
		Out_Of_Power_Stop();
	}
#endif
}

//龙溪主控
void zbSoc_LXSensorStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);
	zbSoc_MasterControlReportResolve(cmd,epInfo);
	Out_Of_Power_Stop();
}

void zbSoc_AirControlSensorStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint16_t attrID;
  	uint8_t dataType;
	uint8_t status;
	uint8_t length = 0;
	uint8_t airAttribute[10] = {0};

	cmdGet16bitVal_lh(cmd, &attrID);
    cmdGet8bitVal(cmd, &dataType);//get data type;
	log_debug("attrID = %d ,dataType = %d\n",attrID,dataType);
	
	if ((attrID == ATTRID_BASIC_KTWKQ_OPER)&&(dataType == ZCL_DATATYPE_OCTET_STR))//心跳包
	{
		cmdGet8bitVal(cmd,&length);
		log_debug("len = %d\n",length);
		if(length == 9)//数据长度
		{
			cmdGetStringVal(cmd,airAttribute,length);
			//更新内存中该设备的数据
			//pepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
			
			uint8_t value[9] ={0};
			log_debug("DataUpdate:");
//			memcpy(vepInfo->dataSegment,airAttribute,length);
//			vepInfo->length = length; 
//			vdevListSetCentralAir(pepInfo,length,airAttribute);
			vdevListSetCentralAirState(epInfo,airAttribute,length);

			SRPC_CentralAirInd(epInfo->IEEEAddr,epInfo->endpoint,length,airAttribute);

			vdevListGetCentralAirState(epInfo,value,9);
			log_debug_array(value,length,NULL);
		}
		Out_Of_Power_Stop();
	}
}

void zbSoc_CurtainSensorStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint16_t attrID;
  	uint8_t dataType;
	uint8_t status = 0;
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType); //get data type;
	
	//窗帘状态上报
	if(((attrID == ATTRID_BASIC_CL_STATUS)||(attrID == ATTRID_ON_OFF)) &&(dataType == ZCL_DATATYPE_UINT8))
	{
		//接收到返回，关闭重发机制
		zblist_remove(epInfo->nwkAddr,epInfo->endpoint);
		
		cmdGet8bitVal(cmd, &status);
		SRPC_WinCurtainStatusInd(epInfo->IEEEAddr,epInfo->endpoint,status);
		zbSoc_ProcessEvent(epInfo,status);
	}
	
	//上报窗帘行程百分比
	if((attrID == ATTRID_MOVE_LEVEL) &&(dataType == ZCL_DATATYPE_UINT8))
	{
		cmdGet8bitVal(cmd, &status);
		SRPC_WinCurtainPercentageStatusInd(epInfo->IEEEAddr,epInfo->endpoint,status);
	}
}
void zbSoc_NewCurtainSensorStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint16_t attrID;
  	uint8_t dataType;
	uint8_t status = 0;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType); //get data type;

	switch(attrID)
	{
		//状态上报
		case ATTRID_CLOSURES_WINDOW_COVERING_STATUS:
			//接收到返回，关闭重发机制
			zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

			cmdGet8bitVal(cmd, &status);
			if(status == 0x00)
				status = 0x04;
			else if(status == 0x01)
				status = 0x05;
			else 
				status = 0x06;
			
			SRPC_WinCurtainStatusInd(epInfo->IEEEAddr,epInfo->endpoint,status);
			zbSoc_ProcessEvent(epInfo,status);
			break;
		//百分比上报
		case ATTRID_CLOSURES_CURRENT_POSITION_LIFT_PERCENTAGE:
			cmdGet8bitVal(cmd, &status);
			SRPC_WinCurtainPercentageStatusInd(epInfo->IEEEAddr,epInfo->endpoint,status);
			break;
		default:break;
	}
}

void zbSoc_MeterStateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType); //get data type;

	if(attrID == ATTRID_BASIC_UART_MSG  && dataType == ZCL_DATATYPE_OCTET_STR)
	{
		zblist_remove(epInfo->nwkAddr,epInfo->endpoint);
		meter_SerialMsgProcess(epInfo,cmd);
	}
}

/*海曼人体感应上报*/
void zbSoc_IAS_ZONE_StateReport(hostCmd *cmd,epInfo_t *epInfo)
{
	ASSERT(cmd != NULL && epInfo != NULL);

	uint16_t attrID;
	uint8_t dataType;

	log_debug("zbSoc_IAS_ZONE_StateReport++\n");
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType); //get data type;

	switch(attrID)
	{
		/*红外人体感应电量*/
		case ATTRID_POWER_CFG_BATTERY_VOLTAGE:
		{
			uint8_t batteryUnit = 0;
			float battery = 0;
			cmdGet8bitVal(cmd, &batteryUnit);
			//进行分压计算 
			//当前电压数 = batteryUnit *100 mv
			//分压后数据 = 当前电压数/1000 * 100 / 6
			battery = (batteryUnit * 10)/6;//当前的电压mV
			vdevListSetAlarmBattery(epInfo, (uint8_t)battery);
			SRPC_ComAlarmPowerValueInd(epInfo->IEEEAddr,epInfo->endpoint,epInfo->deviceID,(uint8_t)battery);
		}
		break;
		/*红外人体感应电池报警状态*/
		case ATTRID_POWER_CFG_BAT_ALARM_STATE:
		{
			uint8_t batteryAlarm = 0;
			cmdGet8bitVal(cmd, &batteryAlarm);
			vdevListSetAlarmBatteryAlarm(epInfo,batteryAlarm);
		}
		break;
		default:break;
	}

	/*更新设备在线状态*/
	vdevListSetDevOnlineState(epInfo,true);
	
	log_debug("zbSoc_IAS_ZONE_StateReport--\n");
}


void zbSoc_DoorLockReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
    uint8_t status;
    uint8_t dataType;
	//epInfo_t * pepInfo = NULL;
	
    cmdGet16bitVal_lh(cmd, &attrID);
    cmdGet8bitVal(cmd, &status);
    cmdGet8bitVal(cmd, &dataType);//get data type;
	//获取版本
    if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}
	//门锁电量检测
	else if ((clusterID == ZCL_CLUSTER_ID_GEN_BASIC) && (attrID == ATTRID_BASIC_POWER_VALUE) && (dataType == ZCL_DATATYPE_UINT8))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		//SRPC_DoorLockPowerValueResp(epInfo->IEEEAddr,epInfo->endpoint,data);
		SRPC_DoorLockPowerValueInd(epInfo->IEEEAddr,epInfo->endpoint,data,epInfo!=NULL?epInfo->onlineDevRssi:0);
//				ZbSocHeartbeat_ZbDevicePowerValue_update(epInfo->IEEEAddr,epInfo->endpoint,1,&data);
//				devState_updateevicePowerValue(epInfo,data);
		vdevListSetDoorBattery(epInfo,DOORLOCK_POWER_TYPE_VALUE,data);

	}
	//数据类型兼容处理
	else if ((clusterID == ZCL_CLUSTER_ID_GEN_ON_OFF) && (attrID == ATTRID_ON_OFF) && ( (dataType == ZCL_DATATYPE_BOOLEAN) || (dataType == ZCL_DATATYPE_UINT8) )  ){
		uint8_t state;
		cmdGet8bitVal(cmd, &state);
		log_debug("state = %d\n",state);
		SRPC_DoorLockCtrlInd(epInfo->IEEEAddr,epInfo->endpoint,state,epInfo!=NULL?epInfo->onlineDevRssi:0);
//				devState_updateSwitchVal(epInfo,state);
		vdevListSetDoorState(epInfo,state);
	}
	//查询电机转动时间
	else if ((clusterID == ZCL_CLUSTER_ID_GEN_BASIC) && (attrID == ATTRID_BASIC_MOTOR_SPEED_CHECK) && (dataType == ZCL_DATATYPE_UINT16))
	{
		uint16_t mValue = 0;
		cmdGet16bitVal_lh(cmd,&mValue);
		SRPC_QueryDoorLockSpeedCfm(epInfo->IEEEAddr,epInfo->endpoint,YY_STATUS_SUCCESS,mValue);
	}
#if DEVICE_LIWEI_DOOR_OPEN_CNT
	else if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(attrID == 0x0038)&&(dataType == ZCL_DATATYPE_UINT32))
	{
		uint32_t mValue = 0;
		cmdGet32bitVal_lh(cmd,&mValue);
		SRPC_LevelDoorLockOpenCntInd(epInfo->IEEEAddr,epInfo->endpoint,mValue);
	}
#endif	
	//处理RSSI
	zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);

}

void zbSoc_PowerSwitchReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
		uint16_t attrID;
		uint8_t status;
		uint8_t dataType;
		//epInfo_t * pepInfo = NULL;
		
		cmdGet16bitVal_lh(cmd, &attrID);
		cmdGet8bitVal(cmd, &status);
		cmdGet8bitVal(cmd, &dataType);//get data type;
		//获取版本
		if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
		{
			uint8_t data = 0;
			cmdGet8bitVal(cmd,&data);
			SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
		}
		//查看取电电量查询反馈
		else if((clusterID == ZCL_CLUSTER_ID_MS_SIMPLE_METER) && (attrID == ATTRID_MS_ELECTRICAL_METER_MEASURED_VALUE) && (dataType == ZCL_DATATYPE_UINT32))
		{
			uint32_t data = 0;
			cmdGet32bitVal_lh(cmd,&data);
			log_debug("befault Volatile PWM: 0x%x\n",data);
			data = (uint32_t)((float)data/(1600.0)*1000);
			log_debug("after Volatile PWM: %d\n",data);
			SRPC_QueryPowerSwitchValueInd(epInfo->IEEEAddr,epInfo->endpoint,data,epInfo!=NULL?epInfo->onlineDevRssi:0);
//				ZbSocHeartbeat_ZbDevicePowerValue_update(epInfo->IEEEAddr,epInfo->endpoint,4,&data);
//				devState_updateevicePowerValue(epInfo,data);
			vdevListSetPowerSwitchBattery(epInfo,data);

		}
		//状态查询
		else if ((clusterID == ZCL_CLUSTER_ID_GEN_ON_OFF) && (attrID == ATTRID_ON_OFF) && ( (dataType == ZCL_DATATYPE_BOOLEAN) || (dataType == ZCL_DATATYPE_UINT8) )  )//数据类型兼容处理
		{
			uint8_t state;
			cmdGet8bitVal(cmd, &state);
			log_debug("state = %d\n",state);
			SRPC_PowerSwitchCtrlInd(epInfo->IEEEAddr,epInfo->endpoint,state,epInfo!=NULL?epInfo->onlineDevRssi:0);
			//devState_updateSwitchVal(epInfo,state);
			vdevListSetPowerSwitchState(epInfo,state);
		}

		//处理RSSI
		zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);
}

void zbSoc_SwitchOnOffReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
		uint16_t attrID;
		uint8_t status;
		uint8_t dataType;
		//epInfo_t * pepInfo = NULL;
		
		cmdGet16bitVal_lh(cmd, &attrID);
		cmdGet8bitVal(cmd, &status);
		cmdGet8bitVal(cmd, &dataType);//get data type;
		//获取版本
		if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
		{
			uint8_t data = 0;
			cmdGet8bitVal(cmd,&data);
			SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
		}
		//数据类型兼容处理
		else if ((clusterID == ZCL_CLUSTER_ID_GEN_ON_OFF) && (attrID == ATTRID_ON_OFF) && ( (dataType == ZCL_DATATYPE_BOOLEAN) || (dataType == ZCL_DATATYPE_UINT8) )  )
		{
			uint8_t state;
			cmdGet8bitVal(cmd, &state);
			log_debug("state = %d\n",state);
			SRPC_SwitchStateInd(epInfo->IEEEAddr,epInfo->endpoint,state,epInfo!=NULL?epInfo->onlineDevRssi:0);
			//devState_updateSwitchVal(epInfo,state);
			vdevListSetSwitchState(epInfo,state);
		}
		else if ((clusterID == ZCL_CLUSTER_ID_GEN_BASIC) && (attrID == ATTRID_BASIC_KG_DEFAULT_STATUS)&&  (dataType == ZCL_DATATYPE_UINT8))//数据类型兼容处理
		{
			uint8_t state;
			cmdGet8bitVal(cmd, &state);
			log_debug("state = %d\n",state);
			if(state & 0x01)
			{
				SRPC_SwitchDefaultStateInd(epInfo->IEEEAddr,epInfo->endpoint,0x01);
			}
			else
			{
				SRPC_SwitchDefaultStateInd(epInfo->IEEEAddr,epInfo->endpoint,0x00);
			}
		}
		//智能插座
		else if(clusterID == ZCL_CLUSTER_ID_GEN_BASIC)
		{
			int8_t types = -1;
			uint32_t values = 0;
			
			if(attrID == ATTRID_BASIC_CARACITY_VALUE)
			{
				types = 0;
			}
			else if(attrID == ATTRID_BASIC_POWER_VALUE)
			{
				types = 1;
			}
			else if(attrID == ATTRID_BASIC_VOLTAGE_VALUE)
			{
				types = 2;
			}
			else if(attrID == ATTRID_BASIC_CURRENT_VALUE)
			{
				types = 3;
			}
			else
			{
				types = -1;
				return ;
			}
			
//			zblist_remove(epInfo->nwkAddr,epInfo->endpoint);
			
			cmdGet32bitVal_lh(cmd,&values);
			SRPC_SwitchSocketValueInd(epInfo->IEEEAddr,epInfo->endpoint,types,values);
		}

		//处理RSSI
		zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);
}

void zbSoc_EnvtSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t status;
	uint8_t dataType;
	//epInfo_t * pepInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &status);
	cmdGet8bitVal(cmd, &dataType);//get data type;
	//获取版本
	if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}
	else if(attrID == ATTRID_BASIC_TEMP_HUM_VALUE && dataType == ZCL_DATATYPE_UINT16)
	{
			
		uint16_t date = 0;
		cmdGet16bitVal(cmd,&date);
		SRPC_HumitureStateInd(epInfo->IEEEAddr,epInfo->endpoint,date);
	//				devState_updateHumitureVal(epInfo,date&0xff00>>8,date&0x00ff);
		vdevListSetSenserHumidity(epInfo,date&0x00ff);
		vdevListSetSensertemp(epInfo,date&0xff00>>8);
	}

	//处理RSSI
	zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);
}

void zbSoc_AlarmSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t  status;
	uint8_t  dataType;
	//epInfo_t * pepInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &status);
	cmdGet8bitVal(cmd, &dataType);//get data type;

	//获取版本
	if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}
	//处理RSSI
	zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);
}

void zbSoc_InfdSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t status;
	uint8_t dataType;
	//epInfo_t * pepInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &status);
	cmdGet8bitVal(cmd, &dataType);//get data type;
	//获取版本
	if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}
	//处理RSSI
	zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);
}

void zbSoc_LXSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t status;
	uint8_t dataType;
	//epInfo_t * pepInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &status);
	cmdGet8bitVal(cmd, &dataType);//get data type;
	
	//获取版本
	if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID == ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}

	log_debug("attrID = %d,dataType = %d\n",attrID ,dataType);

	if((attrID == ATTRID_MASTER_CONTROL_READ_VALUE) && (dataType == ZCL_DATATYPE_CHAR_STR))
	{
		zbSoc_MasterControlReadRspResolve(cmd,epInfo);
	}
	
	//处理RSSI
	zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);
}

void zbSoc_AirControlSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t status;
	uint8_t dataType;
	//epInfo_t * pepInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &status);
	cmdGet8bitVal(cmd, &dataType);//get data type;
	//获取版本
	if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}
	//处理RSSI
	zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);

}

void zbSoc_CurtainSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t status;
	uint8_t dataType;
	//epInfo_t * pepInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &status);
	cmdGet8bitVal(cmd, &dataType);//get data type;
	//获取版本
	if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}

	//处理RSSI
	zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);

}

void zbSoc_NewCurtainSensorReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t status;
	uint8_t dataType;
	//epInfo_t * pepInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &status);
	cmdGet8bitVal(cmd, &dataType);//get data type;
	//获取版本
	if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}

	if((clusterID == ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_CLOSURES_CURRENT_POSITION_LIFT_PERCENTAGE || attrID == ATTRID_CLOSURES_CURRENT_POSITION_TILT_PERCENTAGE))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		//SRPC_WinCurtainPercentageStatusInd(epInfo->IEEEAddr,epInfo->endpoint,data);
		//SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}

	if((clusterID == ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_CLOSURES_WINDOW_COVERING_STATUS))
	{
		uint8_t data = 0;
		cmdGet8bitVal(cmd,&data);
		
		if(data == 0x00)
			data = 0x04;
		else if(data == 0x01)
			data = 0x05;
		else 
			data = 0x06;
			
		SRPC_WinCurtainStatusInd(epInfo->IEEEAddr,epInfo->endpoint,data);
		//SRPC_WinCurtainPercentageStatusInd(epInfo->IEEEAddr,epInfo->endpoint,data);
		//SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
	}
	
	//处理RSSI
	zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);

}


void zbSoc_MeterReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
    uint8_t status;
    uint8_t dataType;
	uint8_t data;
	
	switch (clusterID)
	{
		case ZCL_CLUSTER_ID_GEN_BASIC :
		{
			 cmdGet16bitVal_lh(cmd, &attrID);
			 cmdGet8bitVal(cmd, &status);
			 if((status == SUCCESS) && (attrID == ATTRID_BASIC_ZCL_VERSION))
			 {
			 	 cmdGet8bitVal(cmd, &dataType);
				 cmdGet8bitVal(cmd,&data);
				 SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
			 }
		}
		break;
		case ZCL_CLUSTER_ID_HA_DIAGNOSTIC:
			cmdGet16bitVal_lh(cmd, &attrID);
			cmdGet8bitVal(cmd, &status);
			cmdGet8bitVal(cmd, &dataType);
			//处理RSSI
			zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);
		break;
		default:break;
	}
	
}

void zbSoc_IAS_ZONE_ReadResp(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
    uint8_t status;
    uint8_t dataType;
	uint8_t data;
	
	switch (clusterID)
	{
		case ZCL_CLUSTER_ID_GEN_BASIC :
		{
			 cmdGet16bitVal_lh(cmd, &attrID);
			 cmdGet8bitVal(cmd, &status);
			 if((status == SUCCESS) && (attrID == ATTRID_BASIC_ZCL_VERSION))
			 {
			 	 cmdGet8bitVal(cmd, &dataType);
				 cmdGet8bitVal(cmd,&data);
				 SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
			 }
		}
		break;
		case ZCL_CLUSTER_ID_HA_DIAGNOSTIC:
			cmdGet16bitVal_lh(cmd, &attrID);
			cmdGet8bitVal(cmd, &status);
			cmdGet8bitVal(cmd, &dataType);
			//处理RSSI
			zclDiagnostic_ProcessDiagnosticMsg(epInfo,clusterID,attrID,dataType,cmd);
		break;
		default:break;
	}
	
}


void zbSoc_DoorLockHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{	
	uint8_t cnt;
	
	uint16_t attrID;
	uint8_t dataType;
	uint8_t PowerValue = 0;
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	
	if(dataType == ZCL_DATATYPE_UINT8)//当前电量
	{
		cmdGet8bitVal(cmd,&PowerValue);
		//devState_updateevicePowerValue(epInfo,PowerValue);
		//vdevListSetDoorBattery(epInfo,PowerValue);
	}	

}

void zbSoc_PowerSwitchHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	
	if((attrID == ATTRID_ON_OFF )&&((dataType == ZCL_DATATYPE_BOOLEAN) || (dataType == ZCL_DATATYPE_UINT8) ))
	{
		uint8_t state;
		cmdGet8bitVal(cmd, &state);
		log_debug("state = %d\n",state);
	//	devState_updateSwitchVal(epInfo,state);
		vdevListSetPowerSwitchState(epInfo,state);
	}

	if((attrID == ATTRID_MS_ELECTRICAL_METER_MEASURED_VALUE) && (dataType == ZCL_DATATYPE_UINT32))
	{
		uint32_t data = 0;
		cmdGet32bitVal_lh(cmd,&data);
		log_debug("befault Volatile PWM: %d\n",data);
		data = (uint32_t)((float)data/1600.0*1000);
		log_debug("after Volatile PWM: %d\n",data);
	//	devState_updateevicePowerValue(epInfo,data);
		vdevListSetPowerSwitchBattery(epInfo,data);
		

	}

}

void zbSoc_SwitchOnOffHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	
	uint16_t attrID;
	uint8_t dataType;
	uint8_t NumCom = 0;
	uint8_t edpointData = 0;
	uint8_t data	= 0;
	epInfo_t *mEpInfo = NULL;

	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	
	if(dataType == ZCL_DATATYPE_UINT8)
	{
		cmdGet8bitVal(cmd,&data);
		NumCom = ((data&0xf0)>>4);
		switch(NumCom)
		{
			case 4:
				mEpInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,0x0b);
				edpointData = ((data&0x08)>>3);
				log_debug("edpointData = %d\n",edpointData);
	//			devState_updateSwitchVal(mEpInfo,edpointData);

				if(mEpInfo!=NULL)
					vdevListSetPowerSwitchState(mEpInfo,edpointData);
					
				if(mEpInfo!=NULL && mEpInfo->onlineflag == false)
				{
					
					SRPC_SwitchStateInd(mEpInfo->IEEEAddr,mEpInfo->endpoint,edpointData,epInfo->onlineDevRssi);
				}
			case 3:
				mEpInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,0x0a);
				edpointData = ((data&0x04)>>2);
				log_debug("edpointData = %d\n",edpointData);
	//			devState_updateSwitchVal(mEpInfo,edpointData);

				if(mEpInfo!=NULL)
					vdevListSetPowerSwitchState(mEpInfo,edpointData);
					
				if(mEpInfo!=NULL && mEpInfo->onlineflag == false)
				{
					SRPC_SwitchStateInd(mEpInfo->IEEEAddr,mEpInfo->endpoint,edpointData,epInfo->onlineDevRssi);
				}
			case 2:
				mEpInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,0x09);
				edpointData = ((data&0x02)>>1);
				log_debug("edpointData = %d\n",edpointData);
	//							devState_updateSwitchVal(mEpInfo,edpointData);

				if(mEpInfo!=NULL)
					vdevListSetPowerSwitchState(mEpInfo,edpointData);
				
				if(mEpInfo!=NULL && mEpInfo->onlineflag == false)
				{
					SRPC_SwitchStateInd(mEpInfo->IEEEAddr,mEpInfo->endpoint,edpointData,epInfo->onlineDevRssi);
				}
			case 1:
				mEpInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,0x08);
				edpointData = ((data&0x01)>>0);
				log_debug("edpointData = %d\n",edpointData);
	//							devState_updateSwitchVal(mEpInfo,edpointData);

				if(mEpInfo!=NULL)
					vdevListSetPowerSwitchState(mEpInfo,edpointData);
					
				if(mEpInfo!=NULL && mEpInfo->onlineflag == false)
				{
					SRPC_SwitchStateInd(mEpInfo->IEEEAddr,mEpInfo->endpoint,edpointData,epInfo->onlineDevRssi);
				}
			case 0:
				break;
		}
	}

}

void zbSoc_EnvtSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	

}
void zbSoc_AlarmSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);

	//状态
	if((attrID == ATTRID_BASIC_HEARTBEAT)&&(dataType==ZCL_DATATYPE_UINT8))
	{
		uint8_t devstate = 0;
		cmdGet8bitVal(cmd,&devstate);
		log_debug("State = %d\n",devstate);
		vdevListSetAlarmState(epInfo,devstate);
	}
	
	//电量
	if((attrID == ATTRID_BASIC_POWER_VALUE)&&(dataType==ZCL_DATATYPE_UINT8))
	{
		uint8_t powerValue = 0;
		cmdGet8bitVal(cmd,&powerValue);
		log_debug("Power = %d\n",powerValue);
		vdevListSetAlarmBattery(epInfo,powerValue);
	}
	
	//使能状态
	if((attrID == ATTRID_BASIC_IRSENSOR_ONOFF)&&(dataType==ZCL_DATATYPE_UINT8))
	{
		uint8_t mIrcable = 0;
		cmdGet8bitVal(cmd,&mIrcable);
		log_debug("IRC working state = %d\n",mIrcable);
		vdevListSetAlarmisEnable(epInfo,mIrcable);
	}
	
/*	
	//获取电量分压值
	if((attrID == ATTRID_BASIC_HEARTBEAT)&&(dataType==ZCL_DATATYPE_UINT8))
	{
		uint8_t powerValue = 0;
		cmdGet8bitVal(cmd,&powerValue);
		log_debug("IRC PowerValue = %d\n",powerValue);
//		devState_updateevicePowerValue(epInfo,powerValue);
		vdevListSetAlarmBattery(epInfo,powerValue);
	}
	//启动/禁止红外转发器状态
	if((attrID == ATTRID_BASIC_IRSENSOR_ONOFF)&&(dataType==ZCL_DATATYPE_UINT8))
	{
		uint8_t mIrcable = 0;
		cmdGet8bitVal(cmd,&mIrcable);
		log_debug("IRC working state = %d\n",mIrcable);
//		devState_updateSwitchVal(epInfo,mIrcable);
		vdevListSetAlarmisEnable(epInfo,mIrcable);
	}
*/
}

void zbSoc_InfdSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	

}

void zbSoc_LXSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	
}

void zbSoc_AirControlSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	
	uint8_t length = 0;
	uint8_t airAttribute[16] = {0};
	epInfo_t *pepInfo = NULL;
	
	log_debug("zbSoc_AirControlSensorHeartReport++\n");
	log_debug("attrID = %d , DataType = %d\n",attrID,dataType);
	
	if ((attrID == ATTRID_BASIC_HEARTBEAT)&&(dataType == ZCL_DATATYPE_OCTET_STR))//心跳包
	{
		cmdGet8bitVal(cmd,&length);
		log_debug("datalength = %d\n",length);
		if(length == 9)//数据长度
		{
			cmdGetStringVal(cmd,airAttribute,length);
			log_debug_array(airAttribute,length,NULL);
			//更新内存中该设备的数据
			pepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint);
			if(pepInfo != NULL)
			{
				//memcpy(vepInfo->dataSegment,airAttribute,length);
				//vepInfo->length = length; 
				//vdevListSetCentralAir(pepInfo,length,airAttribute);

				vdevListSetCentralAirState(pepInfo,airAttribute,length);
				//更新rssi
				if(pepInfo->onlineDevRssi!=epInfo->onlineDevRssi)
					pepInfo->onlineDevRssi = epInfo->onlineDevRssi;
			}
		}
	}

	log_debug("zbSoc_AirControlSensorHeartReport--\n");
}

void zbSoc_CurtainSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	
	uint8_t data	= 0;
	if(dataType == ZCL_DATATYPE_UINT8)
	{
		cmdGet8bitVal(cmd,&data);
	//					devState_updateSwitchVal(epInfo,data);
		vdevListSetCurtainState(epInfo,data);
	}

}

void zbSoc_NewCurtainSensorHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	
	cmdGet16bitVal_lh(cmd, &attrID);
	cmdGet8bitVal(cmd, &dataType);
	
	uint8_t data	= 0;
	if(dataType == ZCL_DATATYPE_UINT8)
	{
		cmdGet8bitVal(cmd,&data);
	//					devState_updateSwitchVal(epInfo,data);
		vdevListSetCurtainState(epInfo,data);
	}
}

void zbSoc_MeterHeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	//process 
}

void zbSoc_IAS_ZONE_HeartReport(hostCmd *cmd,epInfo_t *epInfo)
{
	//pass
}

void zbSoc_doDeviceStateReportCallBack(hostCmd *cmd,epInfo_t *epInfo)
{
	uint8_t cnt = 0;;

	for(cnt=0;cnt < ARRAY_SIZE(zbSocStateReportCallbacks) ;cnt++)
	{
		if(zbSocStateReportCallbacks[cnt].DeviceID == epInfo->deviceID)
		{
			if(zbSocStateReportCallbacks[cnt].StateReport_cb != NULL)
				zbSocStateReportCallbacks[cnt].StateReport_cb(cmd,epInfo);
		}
	}
}

void zbSoc_doDeviceReadRespCallBack(uint16_t clusterID,hostCmd *cmd,epInfo_t *epInfo)
{
	uint8_t cnt = 0;;
	uint8_t isSup = FALSE;


	for(cnt=0;cnt < ARRAY_SIZE(zbSocStateReportCallbacks) ;cnt++)
	{
		if(zbSocStateReportCallbacks[cnt].DeviceID == epInfo->deviceID)
		{
			if(zbSocStateReportCallbacks[cnt].ReadResp_cb != NULL)
				zbSocStateReportCallbacks[cnt].ReadResp_cb(clusterID,cmd,epInfo);
			isSup = TRUE;
		}
	}

	if(isSup)
	{
		switch(clusterID)
		{
			case ZCL_CLUSTER_ID_GEN_BASIC:
				{
						uint16_t attrID;
						uint8_t status;
						uint8_t dataType;
						
						cmdGet16bitVal_lh(cmd, &attrID);
						cmdGet8bitVal(cmd, &status);
						cmdGet8bitVal(cmd, &dataType);
						//获取版本
						if((clusterID == ZCL_CLUSTER_ID_GEN_BASIC)&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
						{
							uint8_t data = 0;
							cmdGet8bitVal(cmd,&data);
							SRPC_GetDevVersionInd(epInfo->IEEEAddr,epInfo->endpoint,data);
						}
				}
				break;
			default:break;
		}
	}
}

void zbSoc_doDeviceHeartReportCallBack(hostCmd *cmd,epInfo_t *epInfo)
{
	uint8_t cnt = 0;;

	for(cnt=0;cnt < ARRAY_SIZE(zbSocStateReportCallbacks) ;cnt++)
	{
		if(zbSocStateReportCallbacks[cnt].DeviceID == epInfo->deviceID)
		{
			if(zbSocStateReportCallbacks[cnt].HeartReport_cb != NULL)
				zbSocStateReportCallbacks[cnt].HeartReport_cb(cmd,epInfo);
		}
	}
}

/*************************************************************************************************
 * @fn      processRpcSysAppZclFoundation()
 *
 * @brief  process the ZCL Rsp from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
static void processRpcSysAppZclFoundation(hostCmd *cmd,
        uint8_t zclFrameLen, uint16_t clusterID, uint16_t nwkAddr, uint8_t endpoint)
{
    uint8_t transSeqNum, commandID;

    cmdGet8bitVal(cmd, &transSeqNum);
    cmdGet8bitVal(cmd, &commandID);

	
    if (commandID == ZCL_CMD_READ_RSP)
    {
    	//查询设备状态的时候(ZCL_CMD_READ)，使用此函数处理返回值
        processRpcSysAppFn_DevReadResp(cmd,clusterID,nwkAddr,endpoint);
    }
    else if ((commandID == ZCL_CMD_REPORT)&&(clusterID == ZCL_CLUSTER_ID_GEN_HEARTBEAT_REPORT))
    {
    	//心跳包处理
        processRpcSysAppFn_DevHeartBeatReport(cmd,nwkAddr,endpoint);
    }
    else if (commandID == ZCL_CMD_REPORT)
    {
    	//状态上报处理函数
        processRpcSysAppFn_DevStateReport(cmd,nwkAddr,endpoint);
    }
    else
    {
        //unsupported ZCL Rsp
        log_debug("Unsupported ZCL Rsp,commandID=%02x\n",commandID);
    }

    return;
}

/************************************************************************
* 函数名 :processRpcSysAppFn_DevStateReport(hostCmd *cmd)
* 描述   :   处理节点设备的状态主动上报给服务器
* 输入   ：
* 输出   ：无
* 返回   ：无
************************************************************************/
void processRpcSysAppFn_DevStateReport(hostCmd *cmd,uint16_t nwkAddr, uint8_t endpoint)
{
    uint8_t attrNum;
    uint16_t attrID;
    uint8_t dataType;
    epInfo_t *epInfo;
    uint8_t cnt;
    uint8_t rssi = 0;
    
    log_debug("processRpcSysAppFn_DevStateReport++\n");

    do{
	    	epInfo = vdevListGetDeviceByNaEp(nwkAddr, endpoint);
				if(epInfo == NULL){
					log_debug("NwkAddr:0x%x  EndPoint:0x%x is not found\n",nwkAddr,endpoint);
					mt_Zdo_Ieee_addr_req(nwkAddr);
					break;
				}

				cmdGet8bitVal(cmd, &rssi);
				cmdGet8bitVal(cmd, &attrNum);

				//更新rssi
				epInfo->onlineDevRssi = rssi;

		    log_debug("epInfo->deviceID = %04x,attrNum=%d,rssi = -%d\n",epInfo->deviceID,attrNum,(256-rssi));

				for(cnt=0; cnt<attrNum; cnt++)
				{
					zbSoc_doDeviceStateReportCallBack(cmd,epInfo);
				}	
			
				//修改在线状态
				if(epInfo->registerflag == true)
				{
					log_debug("epInfo->onlineflag = %d\n",epInfo->onlineflag);
					if(epInfo->onlineflag == false)
					{
						if(vdevListSetDevOnlineState(epInfo,true) !=0)
						{
							ZbSocHeartbeat_HeartPacketSend();
							//更新一下心跳包的定时器任务
							zbSoc_Heartbeat_DeviceList_Report_refresh();
						}
					}
				} 
		
    }while(0);
	
    log_debug("processRpcSysAppFn_DevStateReport--\n");

}

/************************************************************************
* 函数名 :processRpcSysAppFn_DevStateReport(hostCmd *cmd)
* 描述   :   处理节点设备的状态主动上报给服务器
* 输入   ：
* 输出   ：无
* 返回   ：无
************************************************************************/
void processRpcSysAppFn_DevReadResp(hostCmd *cmd,uint16_t clusterID,uint16_t nwkAddr, uint8_t endpoint)
{
	uint8_t ret = 0;
  epInfo_t *epInfo = NULL;

	log_debug("processRpcSysAppFn_DevReadResp++\n");
	
	do
	{
		epInfo = vdevListGetDeviceByNaEp(nwkAddr, endpoint);

		if(epInfo == NULL)
		{
			mt_Zdo_Ieee_addr_req(nwkAddr);
			break;
		}

		//更新
		if(epInfo->registerflag == true)
		{
			//zclGetStateCallBack(epInfo,cmd,);

			if(clusterID == ZCL_CLUSTER_ID_GEN_BASIC)
			{
				
			}
				//&&(dataType == ZCL_DATATYPE_UINT8)&&(attrID==ATTRID_BASIC_ZCL_VERSION))
	
			zbSoc_doDeviceReadRespCallBack(clusterID,cmd,epInfo);

			if(epInfo->onlineflag == false)
			{
				if(vdevListSetDevOnlineState(epInfo,true) !=0)
			    {
					ZbSocHeartbeat_HeartPacketSend();

			        //更新一下心跳包的定时器任务
			        zbSoc_Heartbeat_DeviceList_Report_refresh();
			    }
			}
		}  
		
	}while(0);
	
	log_debug("processRpcSysAppFn_DevReadResp--\n");
}

/************************************************************************
* 函数名 :processRpcSysAppFn_DevHeartBeatReport(hostCmd *cmd)
* 描述   :   处理节点设备的状态主动上报给服务器
* 输入   ：
* 输出   ：无
* 返回   ：无
************************************************************************/
void processRpcSysAppFn_DevHeartBeatReport(hostCmd *cmd,uint16_t nwkAddr, uint8_t endpoint)
{
   
    epInfo_t *epInfo;
    uint8_t ret=0;//设备状态更新标志
    uint8_t rssi = 0;
    uint8_t attrNum;
    uint8_t cnt;
     
	log_debug("processRpcSysAppFn_DevHeartBeatReport++\n");

	do
	{
		epInfo = vdevListGetDeviceByNaEp(nwkAddr, endpoint);

		if(epInfo == NULL)
		{
			log_err("Not found epInfo\n");
			//获取节点注册申明
			mt_Zdo_Ieee_addr_req(nwkAddr);
			break;
		}

		if(epInfo->nwkAddr == 0 || epInfo->deviceID == 0x0)
		{
			log_err("Router epInfo\n");
			break;
		}

		cmdGet8bitVal(cmd, &rssi);//get data type;
		cmdGet8bitVal(cmd, &attrNum);

		epInfo->onlineDevRssi = rssi;

		if(epInfo->registerflag == true)
		{
			for(cnt=0; cnt<attrNum; cnt++)
			{
				zbSoc_doDeviceHeartReportCallBack(cmd,epInfo);
			}	
		}

		if(vdevListSetDevOnlineState(epInfo,true) != 0)
		{
			ZbSocHeartbeat_HeartPacketSend();
		  	//更新一下心跳包的定时器任务
		  	zbSoc_Heartbeat_DeviceList_Report_refresh();
		}
	}while(0);
   
    log_debug("processRpcSysAppFn_DevHeartBeatReport--\n");
}

static void processRpcSysAppZclClusterFunction(hostCmd *cmd,
        uint8_t zclFrameLen, uint16_t clusterID, uint16_t nwkAddr, uint8_t endpoint)
{
	 	uint8_t transSeqNum, commandID;

    cmdGet8bitVal(cmd, &transSeqNum);
    cmdGet8bitVal(cmd, &commandID);
}

/*************************************************************************************************
 * @fn      processRpcSysAppZcl()
 *
 * @brief  process the ZCL Rsp from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
static void processRpcSysAppZcl(hostCmd *cmd)
{
    uint8_t zclHdrLen = 3;
    uint16_t nwkAddr, clusterID;
    uint8_t endpoint, zclFrameLen, zclFrameFrameControl;
		
    uint8_t appEndpoint;
		zclLibPlugin_t * pInPlugin = NULL;

    log_debug("processRpcSysAppZcl++\n");

    //This is a ZCL response
    //get app EP
    cmdGet8bitVal(cmd, &appEndpoint);
    cmdGet16bitVal_lh(cmd, &nwkAddr);
    cmdGet8bitVal(cmd, &endpoint);
    cmdGet16bitVal_lh(cmd, &clusterID);
    cmdGet8bitVal(cmd, &zclFrameLen);
    cmdGet8bitVal(cmd, &zclFrameFrameControl);

    //is it manufacturer specific
    if (zclFrameFrameControl & (1 << 2))
    {
        //currently not supported shown for reference
        uint16_t ManSpecCode;
        //manu spec code
        cmdGet16bitVal_lh(cmd, &ManSpecCode);
        //Manufacturer specif commands have 2 extra byte in te header
        zclHdrLen += 2;
        //supress warning
        (void)ManSpecCode;
    }

    //is this a foundation command
    if ((zclFrameFrameControl & 0x3) == 0)
    {
        //log_debug("processRpcSysAppZcl: Foundation messagex\n");
        processRpcSysAppZclFoundation(cmd, zclFrameLen, clusterID, nwkAddr, endpoint);
    }
	// Not a foundation type message, so it must be specific to the cluster ID.
	else 
	{	
		log_debug("ClusterID = 0x%x,zclFrameFrameControl = %d,%d\n",clusterID,zclFrameFrameControl,(zclFrameFrameControl & (1 << 2)));
		// We don't support any manufacturer specific command -- ignore it.
		if ((zclFrameFrameControl & (1 << 2))==0x00)
		{
			pInPlugin = zclFindplugin(clusterID);
			
			if(pInPlugin && pInPlugin->pfnIncomingHdlr)
			{
				pInPlugin->pfnIncomingHdlr(cmd, zclFrameLen, clusterID, nwkAddr, endpoint);
			}
		}
	}
	
	log_debug("processRpcSysAppZcl--\n");
}


static void processRpcSysAppCoordVersionRsp(hostCmd *cmd)
{
	log_debug("processRpcSysApp : cmd1 == 0x82\n");
	uint8_t version = 0;
	cmdGet8bitVal(cmd,&version);
	log_debug("Version:%d.%d\n",(version&0xff00>>4),version&0x00ff);
	zbSoc_Heartbeat_Uart_Report_refresh();
	if(g_getCoordVersion == true)
	{
		SRPC_GetCoorVersionCmdInd(version);
		g_getCoordVersion = false;
	}
}

static void processRpcSysAppSerialReceivedRsp(hostCmd *cmd)
{
	uint8_t status;
	cmdGet8bitVal(cmd, &status);

	log_debug("processRpcSysApp: rpcBuff[2]=%x \n", status);

	if (status == 0)
	{

#ifndef NDEBUG
		SRPC_Mt_Network_SerialState_Ind();
#endif
		zbSoc_Heartbeat_Uart_Report_refresh();
		log_debug("processRpcSysApp: Command Received Successfully\n");
	}
	else
	{
		log_debug("processRpcSysApp: Command Error\n");
	}
}

/*****************************************************************************
 * 函 数 名  : processRpcSysZdoNewDevIndicationCb
 * 负 责 人  : Edward
 * 创建日期  : 2016年3月28日
 * 函数功能  : 新设备注册上报信息
 * 输入参数  : epInfo_t *epInfo  设备节点信息
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static uint8_t processRpcSysZdoNewDevIndicationCb(epInfo_t *epInfo)
{
	uint8_t ret = 0;
    epInfo_t *devInfo = NULL;
    //Just add to device list to store
    //find the IEEE address. Any ep (0xFF), if the is the first simpleDesc for this nwkAddr
    //then devAnnce will enter a dummy entry with ep=0, other wise get IEEE from a previous EP

    devInfo = devListGetDeviceByNaEp(epInfo->nwkAddr, 0xFF);
    
    if(devInfo == NULL) //数据库中没有nwkAddr地址
    {
        devListAddDevice(epInfo);
    }
    else 
    {
    	//IEEE及Nwwk相等
		if((devInfo->nwkAddr == epInfo->nwkAddr)&&(memcmp(devInfo->IEEEAddr,epInfo->IEEEAddr,8)==0))
		{
			//更新设备在线信息
			if(vdevListSetDevOnlineState(epInfo,true) != 0)
			{
				ZbSocHeartbeat_HeartPacketSend();
				//更新一下心跳包的定时器任务
				zbSoc_Heartbeat_DeviceList_Report_refresh();
			}
		}
    }
    
    return 0;
}

static uint8_t processRpcSysZdoSimpleDescRspCb(epInfo_t *epInfo)
{
    epInfo_t *devInfo;
    epInfo_t* oldRec;
    epInfoExtended_t epInfoEx;
		uint8_t ret = 0;
    log_debug("processRpcSysZdoSimpleDescRspCb: NwkAddr:0x%04x  End:0x%02x \n", epInfo->nwkAddr,epInfo->endpoint);

    //find the IEEE address. Any ep (0xFF), if the is the first simpleDesc for this nwkAddr
    //then devAnnce will enter a dummy entry with ep=0, other wise get IEEE from a previous EP
    devInfo = devListGetDeviceByNaEp(epInfo->nwkAddr, 0xFF);

	if(devInfo == NULL)
	{
		log_debug("No Find NwkAddr\n");
//		log_debug("Start Request endpoint Info\n");
//		mt_Zdo_Ieee_addr_req(epInfo->nwkAddr);
		return 0;
	}

	//填充IEEE地址
    memcpy(epInfo->IEEEAddr, devInfo->IEEEAddr, Z_EXTADDR_LEN);

    //remove dummy ep, the devAnnce will enter a dummy entry with ep=0,
    //this is only used for storing the IEEE address until the  first real EP
    //is enter.
    //while(devListRemoveDeviceByNaEp(epInfo->nwkAddr, 0x00)!=NULL);

	//删除所有endpoint为0的设备
	while(devListRemoveDeviceByIeeeEp(epInfo->IEEEAddr,0x00)!=NULL);
	
    //devListRemoveDeviceByEp(0x00);

    //is this a new device or an update
    oldRec = devListGetDeviceByIeeeEp(epInfo->IEEEAddr, epInfo->endpoint);
	//更新信息
    if (oldRec != NULL)
    {
        //保存下注册信息
        epInfo->registerflag = oldRec->registerflag;

		//更新nwkAddr
        if (epInfo->nwkAddr != oldRec->nwkAddr)
        {
            epInfoEx.type = EP_INFO_TYPE_UPDATED;
            epInfoEx.prevNwkAddr = oldRec->nwkAddr;
            //devListRemoveDeviceByNaEp(oldRec->nwkAddr, oldRec->endpoint); //theoretically, update the database record in place is possible, but this other approach is selected to provide change logging. Records that are marked as deleted soes not have to be phisically deleted (e.g. by avoiding consilidation) and thus the database can be used as connection log
			//删除设备
			while(devListRemoveDeviceByIeeeEp(epInfo->IEEEAddr,oldRec->endpoint)!=NULL);
        }
        else
        {
            //not checking if any of the records has changed. assuming that for a given device (ieee_addr+endpoint_number) nothing will change except the network address.
            epInfoEx.type = EP_INFO_TYPE_EXISTING;
        }
    }
    //添加新设备
    else
    {	
    	//设备默认完成注册
        epInfo->registerflag = true;
        epInfoEx.type = EP_INFO_TYPE_NEW;
    }

    log_debug("processRpcSysZdoSimpleDescRspCb: NwkAddr:0x%04x Ep:0x%02x Type:0x%02x \n", epInfo->nwkAddr,epInfo->endpoint, epInfoEx.type);

	/*更新或修改设备信息*/
    if (epInfoEx.type != EP_INFO_TYPE_EXISTING)
    {
		epInfo->onlineTimeoutCounter = 0;
		epInfo->onlineflag = true;
		
		epInfoEx.epInfo = epInfo;
		
		devListAddDevice(epInfo);

#if USE_MASTER_CONTROL
		zbSoc_MasterControlRegister(&epInfoEx);
#else
        //新设备注册上报
        zbSoc_DevRegisterReporting(&epInfoEx);
#endif
    }
  
    //更新设备在线状态
    if(epInfo->registerflag == false)//EP_INFO_TYPE_EXISTING if registerflag = false
    {
        epInfo->onlineTimeoutCounter = 0;
        epInfoEx.epInfo = epInfo;
        zbSoc_DevRegisterReporting(&epInfoEx);
    }
    else if(epInfo->registerflag == true)//EP_INFO_TYPE_EXISTING if registerflag = true
    {
	 	//更新设备在线信息
	 	vdevListSetDevOnlineState(epInfo,true);
	 	ZbSocHeartbeat_HeartPacketSend();
		//更新一下心跳包的定时器任务
		zbSoc_Heartbeat_DeviceList_Report_refresh();
    }
	
    return 0;
}

static uint8_t processRpcSysZdoLeaveIndCb(uint16_t nwkAddr)
{
    epInfoExtended_t removeEpInfoEx;

    removeEpInfoEx.epInfo = devListRemoveDeviceByNaEp(nwkAddr, 0xFF);

	if(removeEpInfoEx.epInfo==NULL)
	{
		return 0;
	}
	
//	if(removeEpInfoEx.epInfo!=NULL)
//	{
//		vdevListRemoveDeviceByIeeeEp((removeEpInfoEx.epInfo)->IEEEAddr, (removeEpInfoEx.epInfo)->endpoint);
//	}
    
	//删除设备
    while(removeEpInfoEx.epInfo)
    {
    	if(removeEpInfoEx.epInfo!=NULL)
		{
			vdevListRemoveDeviceByIeeeEp((removeEpInfoEx.epInfo)->IEEEAddr, (removeEpInfoEx.epInfo)->endpoint);
		}
		
        removeEpInfoEx.type = EP_INFO_TYPE_REMOVED;
        removeEpInfoEx.epInfo = devListRemoveDeviceByNaEp(nwkAddr, 0xFF);
    }
	
	//更新心跳包,更新设备信息
	ZbSocHeartbeat_HeartPacketSend();
	zbSoc_Heartbeat_DeviceList_Report_refresh();
    return 0;
}

/*************************************************************************************************
 * @fn      processRpcSysZdoEndDeviceAnnceInd
 *
 * @brief   read and process the RPC ZDO message from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
void processRpcSysZdoEndDeviceAnnceInd(hostCmd *cmd)
{
    epInfo_t epInfo;
    
    uint16_t srcAddr;
	memset(&epInfo,0,sizeof(epInfo_t));
	
    cmdGet16bitVal_lh(cmd, &srcAddr);
    cmdGet16bitVal_lh(cmd, &epInfo.nwkAddr);
    cmdGetStringVal_lh(cmd, epInfo.IEEEAddr,8);
    cmdGet8bitVal(cmd, &epInfo.capbility);

	log_debug("processRpcSysZdoEndDeviceAnnceInd nwkAddr: %x, IEEE Addr: ",epInfo.nwkAddr);
	log_debug_array(epInfo.IEEEAddr,8,":");
    log_debug("epInfo.capbility=%02x\n",epInfo.capbility);

    //添加到数据库中
    processRpcSysZdoNewDevIndicationCb(&epInfo);

}

/*************************************************************************************************
 * @fn      processRpcSysZdoActiveEPRsp
 *
 * @brief   read and process the RPC ZDO message from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
void processRpcSysZdoActiveEPRsp(hostCmd *cmd)
{
    uint16_t nwkAddr;
    uint8_t activeEPCount,status;
    uint16_t srcAddr;
    uint8_t endpoint = 0;
	uint8_t mloop = 0;
	
	epInfo_t *epInfo = NULL;
	
    cmdGet16bitVal_lh(cmd, &srcAddr);
    cmdGet8bitVal(cmd, &status);
    cmdGet16bitVal_lh(cmd, &nwkAddr);
    cmdGet8bitVal(cmd, &activeEPCount);

	epInfo = devListGetDeviceByNaEp(nwkAddr,0XFF);
	
	if(epInfo != NULL) //数据库中UC已经存在
	{
		//获取端点列表
	    for(mloop = 0; mloop < activeEPCount; mloop++)
	    {
	    	cmdGet8bitVal(cmd,&endpoint);

			epInfo = devListGetDeviceByNaEp(nwkAddr,endpoint);
	    	if(epInfo == NULL)
	    	{
				mt_Zdo_Simple_Desc_Req(nwkAddr,endpoint);
	    	}
	    }
	}
	else
	{
		mt_Zdo_Ieee_addr_req(nwkAddr);
	}	
    
    log_debug("processRpcSysZdoActiveEPRsp nwkAddr: %x, Num Ep's: %d \n", nwkAddr, activeEPCount);
}

/*************************************************************************************************
 * @fn      processRpcSysZdoSimpleDescRsp
 *
 * @brief   read and process the RPC ZDO message from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
void processRpcSysZdoSimpleDescRsp(hostCmd *cmd)
{
    epInfo_t epInfo;
    uint8_t numInputClusters, numOutputClusters, clusterIdx;
    uint16_t  *inClusters, *outClusters;
    uint16_t srcAddr;
    uint8_t   len,status;

    cmdGet16bitVal_lh(cmd, &srcAddr);
    cmdGet8bitVal(cmd, &status);
    cmdGet16bitVal_lh(cmd, &epInfo.nwkAddr);
    cmdGet8bitVal(cmd, &len);
    cmdGet8bitVal(cmd, &epInfo.endpoint);
    cmdGet16bitVal_lh(cmd, &epInfo.profileID);
    cmdGet16bitVal_lh(cmd, &epInfo.deviceID);
    cmdGet8bitVal(cmd, &epInfo.version);

    epInfo.deviceName = NULL;

    //初始注册后，将设备置为在线状态
    epInfo.onlineflag = true; 

    cmdGet8bitVal(cmd, &numInputClusters);

    log_debug("numInputClusters = %d.\n",numInputClusters);

	//输入Clusters
    inClusters = (uint16_t*) malloc(2*numInputClusters);
    if(inClusters)
    {
        for(clusterIdx = 0; clusterIdx < numInputClusters; clusterIdx++)
        {
            cmdGet16bitVal_lh(cmd, &inClusters[clusterIdx]);
        }
    }

    cmdGet8bitVal(cmd, &numOutputClusters);

    log_debug("numOutputClusters= %d.\n",numOutputClusters);

	//输出Clusters
    outClusters = (uint16_t*) malloc(2*numOutputClusters);
    if(outClusters)
    {
        for(clusterIdx = 0; clusterIdx < numOutputClusters; clusterIdx++)
        {
            cmdGet16bitVal_lh(cmd, &outClusters[clusterIdx]);
        }
    }

    //if this is the TL endpoint then ignore it
    if( !((numInputClusters == 1) && (numOutputClusters == 1) && (epInfo.profileID) == (0xC05E)) )
    {
        processRpcSysZdoSimpleDescRspCb(&epInfo);
    }

    if(inClusters)
    {
        free(inClusters);
    }

    if(outClusters)
    {
        free(outClusters);
    }

}

void processRpcSysZdoLeaveInd(hostCmd *cmd)
{
    uint16_t nwkAddr;

    cmdGet16bitVal_lh(cmd, &nwkAddr);

    log_debug("processRpcSysZdoLeaveInd nwkAddr: %x\n", nwkAddr);

    processRpcSysZdoLeaveIndCb(nwkAddr);
}


void processRpcSysZdoIeeeAddrRsp(hostCmd *cmd)
{
	epInfo_t *epinfo = NULL;
	epInfo_t epInfo;
	memset(&epInfo,0,sizeof(epInfo_t));

   	if(mt_Zdo_Ieee_addr_rsp(cmd,&epInfo)==SUCCESS)
   	{
		log_debug("processRpcSysZdoIeeeAddrRsp nwkAddr: %x, IEEE Addr: ",epInfo.nwkAddr);
		log_debug_array(epInfo.IEEEAddr,8,":");
	    log_debug("epInfo.capbility=%02x\n",epInfo.capbility);

    	//添加到数据库中
    	processRpcSysZdoNewDevIndicationCb(&epInfo);	

    	//发送端点请求命令
		mt_Zdo_Active_ep_req(epInfo.nwkAddr);
   	}
}


void processRpcUtilGetDevInfoRsp(hostCmd *cmd)
{
	uint8_t mIndex = 0;
	uint8_t mStatus = 0;
	uint8_t ieeeAddr[8] = {0};
	uint16_t mShortAddr = 0;
	uint8_t mDeviceType = 0;
	uint8_t mDeviceState = 0;
	uint8_t mNumAssocDevices = 0;
	uint16_t mAssocDeviceList[64]={0};

	if(NULL != cmd)
	{
		cmdGet8bitVal(cmd,&mStatus);
		cmdGetStringVal_lh(cmd,ieeeAddr,8);
		cmdGet16bitVal_lh(cmd,&mShortAddr);
		cmdGet8bitVal(cmd,&mDeviceType);
		cmdGet8bitVal(cmd,&mDeviceState);
		cmdGet8bitVal(cmd,&mNumAssocDevices);
		for(mIndex = 0; mIndex < mNumAssocDevices; mIndex++)
		{
			cmdGet16bitVal_lh(cmd,&mAssocDeviceList[mIndex]);
		}

		SRPC_Util_Get_Device_Info_Cfm(mStatus,ieeeAddr,mShortAddr,mDeviceType,mDeviceState,mNumAssocDevices,mAssocDeviceList);
	}
}



/*************************************************************************************************
 * @fn      processRpcSysApp()
 *
 * @brief   read and process the RPC App message from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
static void processRpcSysApp(hostCmd *cmd)
{
    uint8_t cmd1 = 0;
    cmdGet8bitVal(cmd, &cmd1);

	log_debug("processRpcSysApp++\n");

	switch(cmd1)
	{
		case MT_APP_ZLL_TL_IND:
			log_debug("MT_APP_ZLL_TL_IND\n");
		break;
		/*用户命令反馈*/
		case MT_APP_RSP:
			log_debug("MT_APP_RSP\n");
        	processRpcSysAppZcl(cmd);
		break;
		case MT_APP_COORD_VERSION_RSP://获取设备固件版本号
			log_debug("MT_APP_COORD_VERSION_RSP\n");
       		processRpcSysAppCoordVersionRsp(cmd);
		break;
		case MT_APP_DEVICE_OFF_LINE_RSP://设备未在线
			log_debug("MT_APP_DEVICE_OFF_LINE_RSP\n");
    		//processRpcSysAppDeviceOffLineRsp(cmd);
		break;
		case MT_APP_SERIAL_RECEIVED_RSP:
			log_debug("MT_APP_SERIAL_RECEIVED_RSP\n");
    		processRpcSysAppSerialReceivedRsp(cmd);
		break;
		default:
			log_debug("processRpcSysApp: Unsupported MT App Msg CMD=%d\n",cmd1);
		break;
	}
	
	log_debug("processRpcSysApp--\n");
    return;
}

/*************************************************************************************************
 * @fn      processRpcSysZdo()
 *
 * @brief   read and process the RPC ZDO messages from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
void processRpcSysZdo(hostCmd *cmd)
{
    uint8_t cmd1;
    cmdGet8bitVal(cmd, &cmd1);
    log_debug("cmd1 = %x \n",cmd1);
    switch(cmd1)
    {
        case MT_ZDO_END_DEVICE_ANNCE_IND:	//节点入网返回以下三个命令
            processRpcSysZdoEndDeviceAnnceInd(cmd);	
            break;
        case MT_ZDO_ACTIVE_EP_RSP:			//发送端点数量
            processRpcSysZdoActiveEPRsp(cmd);
            break;
        case MT_ZDO_SIMPLE_DESC_RSP:		//发送端点的描述
            processRpcSysZdoSimpleDescRsp(cmd);
            break;
        case MT_ZDO_LEAVE_IND:				//恢复出厂设置反馈
            processRpcSysZdoLeaveInd(cmd);
            break;
		case MT_ZDO_STATE_CHANGE_IND:		//网络状态改变广播包
			log_debug("This callback message indicates the ZDO state change\n");
			break;
		case MT_ZDO_EXT_NWK_INFO:			//获取网络PANID 网络信道
			mt_Zdo_Ext_Nwk_Info_srsp(cmd);
			break;
		case MT_ZDO_IEEE_ADDR_RSP:			//通过NWK获取IEEE地址
			processRpcSysZdoIeeeAddrRsp(cmd);
			break;
		case MT_ZDO_NWK_CONFLICT_IND:
			SRPC_Mt_Network_Conflict_Ind(0);
        default:
            log_debug("processRpcSysZdo: Unsupported MT ZDO Msg\n");
            break;
    }

}

/*************************************************************************************************
 * @fn      processRpcSysUtil()
 *
 * @brief   read and process the RPC UTIL messages from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/

void processRpcSysUtil(hostCmd *cmd)
{
    uint8_t cmd1;
    cmdGet8bitVal(cmd, &cmd1);

	switch(cmd1)
	{
		case MT_UTIL_GET_DEVICE_INFO:
		 	processRpcUtilGetDevInfoRsp(cmd);
		break;
		case MT_UTIL_SET_CHANNELS:
			mt_Util_Set_Channels_srsp(cmd);
		break;
		default:
		 log_debug("processRpcSysUtil: Unsupported MT UTIL Msg\n");
		break;
	}
}

void processRpcSys(hostCmd *cmd)
{
	uint8_t cmd1;
	cmdGet8bitVal(cmd, &cmd1);
	switch(cmd1)
	{
		case MT_SYS_RESET_IND:
			mt_Sys_Reset_srsp(cmd);
		break;
		case MT_SYS_OSAL_NV_WRITE:
			mt_Sys_Osal_Nv_Write_srsp(cmd);
		break;
		default:
		 log_debug("processRpcSys: Unsupported MT SYS Msg\n");
		break;
	}
}

/*************************************************************************************************
 * @fn      processRpcSysDbg()
 *
 * @brief   read and process the RPC debug message from the ZLL controller
 *
 * @param   none
 *
 * @return  length of current Rx Buffer
 *************************************************************************************************/
static void processRpcSysDbg(hostCmd *cmd)
{
   
}
/************************************************************************
* 函数名 :zbSocMsg_ProcessIncoming(hostCmd *cmd)
* 描述   :   处理解析出的命令包的指令
* 输入   ：
* 输出   ：无
* 返回   ：无
************************************************************************/
void zbSocMsg_ProcessIncoming(hostCmd *cmd)
{
    uint8_t cmd0;
	
    log_debug("zbSocMsg_ProcessIncoming++\n");

    cmdGet8bitVal(cmd, &cmd0);

    switch (cmd0 & MT_RPC_SUBSYSTEM_MASK)
    {
        case MT_RPC_SYS_DBG:
        {
            log_debug("MT_RPC_SYS_DBG \n");
			processRpcSysDbg(cmd);
            break;
        }
        case MT_RPC_SYS_APP:
        {
            log_debug("MT_RPC_SYS_APP \n");
            processRpcSysApp(cmd);
            break;
        }
        case MT_RPC_SYS_ZDO:
        {
            log_debug("MT_RPC_SYS_ZDO \n");
            processRpcSysZdo(cmd);
            break;
        }
        case MT_RPC_SYS_UTIL:
        {
            log_debug("MT_RPC_SYS_UTIL \n");
            processRpcSysUtil(cmd);
            break;
        }
		case MT_RPC_SYS_SYS:
		{
			log_debug("MT_RPC_SYS \n");
			processRpcSys(cmd);
			break;
		}
        default:
        {
            break;
        }
    }

    log_debug("zbSocMsg_ProcessIncoming--\n");

}
