/**************************************************************************************************
 * Filename:       zbSocCmd.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-01,11:22)    :   Create the file.
 *
 *
 *************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>

#include "zbSocUart.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "zbSocCmdSend.h"
#include "LogUtils.h"
/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * CONSTANTS
 */


/************************************************************
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
uint8 zbMakeMsgEnder(hostCmd *cmd)
{
    cmd->data[SOC_MSG_LEN_POS] = ((cmd->idx-4)&0xff);
    cmd->data[SOC_MSG_DATALEN_POS] = ((cmd->idx-SOC_MSG_DATALEN_POS-1)&0xff);
    cmdSetFCS(cmd);
    return true;
}

/************************************************************************
* 函数名 :zbSoc_RevertFactorySettingCmd(uint8 status)
* 描述   :   恢复协调器出厂设置命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_RevertFactorySettingCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;

    printf("zbSoc_RevertFactorySettingCmd !\n");

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

    //wait for message to be consumed
    usleep(30);
}


/************************************************************************
* 函数名 :zbSoc_RevertOneDevFactorySettingCmd(uint8 status)
* 描述   :   恢复单个设备至出厂设置命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_RevertOneDevFactorySettingCmd(uint16 dstAddr,uint8 endpoint)
{
    hostCmd cmd;
    cmd.idx=0;

    printf("zbSoc_RevertOneDevFactorySettingCmd +++++++++++++++!\n");

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

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);

    printf("zbSoc_RevertOneDevFactorySettingCmd ----------------!\n");

}


/************************************************************************
* 函数名 :zbSoc_GetDevIDCmd(void)
* 描述   :   读取设备ID 命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_GetState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
	hostCmd cmd;
    cmd.idx=0;
	LOG_PRINT("zbSoc_GetState++\n");
	
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
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_GET_STATE);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type
    cmdSet8bitVal(&cmd, 0x2);
    cmdSet16bitVal(&cmd, 0xffff);//红外学习模式命令

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);
	
}

/************************************************************************
* 函数名 :zbSoc_GetDevIDCmd(void)
* 描述   :   读取设备ID 命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_GetDevIDCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;
	
    printf("zbSoc_GetDevIDCmd. \n");

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
* 函数名 :zbSoc_GetDevMACCmd(void)
* 描述   :   读取设备ID 命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_GetDevMACCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;

    LOG_DEBUG("zbSoc_GetDevMACCmd. \n");

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
* 函数名 :zbSoc_OpenNwkCmd(uint8 status)
* 描述   :   开放网络命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_OpenNwkCmd(uint8 duration)
{
    hostCmd cmd;
    cmd.idx=0;

    LOG_PRINT("zbSoc_OpenNwkCmd: duration %ds\n", duration);

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len 预留位
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //发送给所有的路由设备,都开放网络
    cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER);	//可以Reserved
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint	//可以Reserved
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, afAddrBroadcast);//Addr mode			//可以Reserved
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control					//可以Reserved
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number//可以Reserved
    cmdSet8bitVal(&cmd, APP_ZCL_CMD_OPEN_NETWORK);//cmd ID
    cmdSet16bitVal_lh(&cmd, duration);//ZCL CLUSTER ID			

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);
}

/************************************************************************
* 函数名 :zbSoc_CoorSystemResetCmd(uint8 status)
* 描述   :   协调器复位命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zbSoc_CoorSystemResetCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;

    printf("zbSoc_CoorSystemResetCmd\n");

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

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);
}


/************************************************************************
* 函数名 :zbSoc_SetGenOnOffState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* 描述   :   发送On/Off命令请求
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_SetGenOnOffState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
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

/************************************************************************
* 函数名 :zbSoc_QueryLightValueState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* 描述   :   查询开关状态值
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_QueryLightValueState(uint16 dstAddr, uint8 endpoint, uint8 addrMode)
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
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_ON_OFF);//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}



/************************************************************************
* 函数名 :zbSoc_QueryDoorLockPowerValueState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* 描述   :   查询门锁电量值
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_QueryDoorLockPowerValueState(uint16 dstAddr, uint8 endpoint, uint8 addrMode)
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

/************************************************************************
* 函数名 :zbSoc_SetTempIntervalReportReq(uint16 intervalTime, uint16 dstAddr,uint8 endpoint, uint8 addrMode)
* 描述   :   配置温度上报的间隔时间
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_SetTempIntervalReportReq(uint16 intervalTime, uint16 dstAddr,uint8 endpoint, uint8 addrMode)
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
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen 预留位
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_CONFIG_REPORT);//ZCL COMMAND ID

    cmdSet8bitVal(&cmd, 0x00);//Direction
    cmdSet16bitVal_lh(&cmd, ATTRID_MS_REPORT_INTERVAL_VALUE);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_UINT8);//Data Type
    cmdSet16bitVal_lh(&cmd, intervalTime);//minReportIn
    cmdSet16bitVal_lh(&cmd, 0xffff);//maxReportIn

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :zbSoc_SetDevValidReq(bool onoff, uint16 dstAddr,uint8 endpoint, uint8 addrMode)
* 描述   :   配置通用节点设备启用/禁用的接口
* 输入   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_SetDevValidReq(bool onoff, uint16 dstAddr,uint8 endpoint, uint8 addrMode)
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

    cmdSet8bitVal(&cmd, 0x00);//Direction
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_DEVICE_ENABLED);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_BOOLEAN);//Data Type
    cmdSet8bitVal(&cmd, onoff);

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_IRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 ctrlcmd)
* 描述   :   控制红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_IRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ctrlcmd)
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
* 函数名 :zbSoc_IRCDevCtrlSetLearnAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr)
* 描述   :   控制红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_IRCDevCtrlSetLearnAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr)
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
* 函数名 :zbSoc_IRCDevCtrlSendAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr)
* 描述   :   控制红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_IRCDevCtrlSendAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr)
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
* 函数名 :zbSoc_RemoteIRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 ctrlcmd)
* 描述   :   控制远程红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_RemoteIRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ctrlcmd)
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
* 函数名 :zbSoc_RemoteIRCDevCtrlCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 ctrlcmd)
* 描述   :   控制远程红外转发器节点进入学习模式
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_RemoteIRCDevCtrlCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ircDataLen,uint8 *ircData)
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

    cmdSet16bitVal_lh(&cmd, ircDataLen);//红外学习模式命令
    cmdSetStringVal(&cmd, ircData,ircDataLen);//红外学习模式命令

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zbSoc_CurtainDevCtrlCmdReq(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdvalue)
* 描述   :   发送窗帘控制器控制命令
* 输入   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_CurtainDevCtrlCmdReq(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdvalue)
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
* 函数名 :zbSoc_RemoveGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* 描述   :   添加组成员命令
* 输入   :  groupId - Group ID of the Scene.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_AddGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint,uint8 addrMode)
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
* 函数名 :zbSoc_RemoveGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* 描述   :   移除组成员命令
* 输入   :  groupId - Group Id
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_RemoveGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
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
* 函数名 :zbSoc_AddSceneMemberCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr,
                                            uint8 endpoint, uint8 addrMode,uint16 deviceid,
                                            uint8 data1,uint8 data2,uint8 data3,uint8 data4)
* 描述   :   Store Scene.
* 输入   :  groupId - Group Id
                    sceneId - Scene ID of the Scene.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_AddSceneMemberCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr,
                             uint8 endpoint, uint8 addrMode,uint16 deviceid,
                             uint8 data1,uint8 data2,uint8 data3,uint8 data4)
{
    uint8 datalen;
    uint16 clusterid;
    hostCmd cmd;
    cmd.idx =0;

    switch(deviceid)
    {
        case ZB_DEV_ONOFF_PLUG:
        case ZB_DEV_ONOFF_SWITCH:
        {
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
            cmdSet8bitVal(&cmd, 0x00);//cmd ID
            cmdSet16bitVal_lh(&cmd, groupId);//ZCL Group ID
            cmdSet8bitVal(&cmd, sceneId++);//cmd ID
            cmdSet8bitVal(&cmd, 0x00);//cmd ID
            cmdSet8bitVal(&cmd, 0x00);//cmd ID
            cmdSet8bitVal(&cmd, 0x00);//cmd ID
            cmdSet16bitVal_lh(&cmd, clusterid);//ZCL cluster id
            cmdSet8bitVal(&cmd, 0x04);//data Length
            cmdSet8bitVal(&cmd, data1);//data1
            cmdSet8bitVal(&cmd, data2);//data2
            cmdSet8bitVal(&cmd, data3);//data3
            cmdSet8bitVal(&cmd, data4);//data4

            zbMakeMsgEnder(&cmd);

            zbSocCmdSend(cmd.data,cmd.idx);
        }
        break;
        case ZB_DEV_WIN_CURTAIN:
        {

        }
        break;
        case ZB_DEV_IRC_LEARN_CTRL:
        {

        }
        break;
        default:
            break;
    }
#if 0
    if ( deviceid == 0x0102 )
    {
        clusterid=0x0300;//彩灯
        datalen=4;

        uint8_t cmd[] =
        {
            0xFE, 0x18, /*RPC payload Len */
            0x29, /*MT_RPC_CMD_AREQ + MT_RPC_SYS_APP */
            0x00, /*MT_APP_MSG  */
            0x08, /*Application Endpoint */
            (dstAddr & 0x00ff), (dstAddr & 0xff00) >> 8, endpoint, /*Dst EP */
            (ZCL_CLUSTER_ID_GEN_SCENES & 0x00ff), (ZCL_CLUSTER_ID_GEN_SCENES & 0xff00)
            >> 8, 13+datalen, //Data Len
            addrMode, 0x01, //0x01 ZCL frame control field.  (send to the light cluster only)
            zbTransSeqNumber++, 0x00, (groupId & 0x00ff), (groupId
            & 0xff00) >> 8, sceneId++, 0x00,0x00,0x00,
            (clusterid & 0x00ff), (clusterid & 0xff00)>> 8,datalen,data1,data2,data3,data4,
            0x00 //FCS - fill in later
        };
        //  int i;
        //  for(i=0;i<sizeof(cmd);++i)
        //      printf("Cheney: Serial cmd[%d]=%x\n", i,cmd[i]);


    }
    else if ( deviceid == 0x0101 )
    {
        clusterid=0x0008;
        datalen=2;

        uint8_t cmd[] =
        {
            0xFE, 0x16, /*RPC payload Len */
            0x29, /*MT_RPC_CMD_AREQ + MT_RPC_SYS_APP */
            0x00, /*MT_APP_MSG  */
            0x08, /*Application Endpoint */
            (dstAddr & 0x00ff), (dstAddr & 0xff00) >> 8, endpoint, /*Dst EP */
            (ZCL_CLUSTER_ID_GEN_SCENES & 0x00ff), (ZCL_CLUSTER_ID_GEN_SCENES & 0xff00)
            >> 8, 13+datalen, //Data Len
            addrMode, 0x01, //0x01 ZCL frame control field.  (send to the light cluster only)
            zbTransSeqNumber++, 0x00, (groupId & 0x00ff), (groupId
            & 0xff00) >> 8, sceneId++, 0x00,0x00,0x00,
            (clusterid & 0x00ff), (clusterid & 0xff00)>> 8,datalen,data1,data2,
            0x00 //FCS - fill in later
        };
        //  int i;
        //  for(i=0;i<sizeof(cmd);++i)
        //      printf("Cheney: Serial cmd[%d]=%x\n", i,cmd[i]);

    }
    else
    {
        clusterid=0x0006;
        datalen=1;

        uint8_t cmd[] =
        {
            0xFE, 0x15, /*RPC payload Len */
            0x29, /*MT_RPC_CMD_AREQ + MT_RPC_SYS_APP */
            0x00, /*MT_APP_MSG  */
            0x08, /*Application Endpoint */
            (dstAddr & 0x00ff), (dstAddr & 0xff00) >> 8, endpoint, /*Dst EP */
            (ZCL_CLUSTER_ID_GEN_SCENES & 0x00ff), (ZCL_CLUSTER_ID_GEN_SCENES & 0xff00)
            >> 8, 13+datalen, //Data Len
            addrMode, 0x01, //0x01 ZCL frame control field.  (send to the light cluster only)
            zbTransSeqNumber++, 0x00, (groupId & 0x00ff), (groupId
            & 0xff00) >> 8, sceneId++, 0x00,0x00,0x00,
            (clusterid & 0x00ff), (clusterid & 0xff00)>> 8,datalen,data1,
            0x00 //FCS - fill in later
        };

        //  int i;
        //  for(i=0;i<sizeof(cmd);++i)
        //      printf("Cheney: Serial cmd[%d]=%x\n", i,cmd[i]);

    }
#endif
}


/************************************************************************
* 函数名 :zbSoc_RemoveSceneMemberCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr,
                                                                uint8 endpoint, uint8 addrMode)
* 描述   :   移除场景成员
* 输入   :  groupId - Group Id
                    sceneId - Scene ID of the Scene.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* 输出   :  无
* 返回   :  0:处理成功
************************************************************************/
void zbSoc_RemoveSceneMemberCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
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
void zbSoc_RecallSceneCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    printf("zbSoc_RecallSceneCmd++\n");

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
    cmdSet8bitVal(&cmd, COMMAND_SCENE_RECALL);//cmd ID
    cmdSet16bitVal_lh(&cmd, groupId);//ZCL Group ID
    cmdSet8bitVal(&cmd, sceneId++);//Scene ID

    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}


