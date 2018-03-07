/***********************************************************************************
 * 文 件 名   : interface_srpcserver.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年11月18日
 * 文件描述   : 
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#include "globalVal.h"

#include "interface_scenelist.h"
#include "interface_deviceStatelist.h"
#include "interface_eventlist.h"
#include "interface_timetasklist.h"
#include "interface_devicelist.h"
#include "interface_vDeviceList.h"
#include "interface_timetasklist.h"
#include "interface_srpcserver.h"
#include "interface_srpcserver_defs.h"

#include "Zigbee_device_Heartbeat_Manager.h"

#include "zbSocCmd.h"
#include "errorCode.h"

#include "comParse.h"
#include "logUtils.h"
#include "fileMng.h"
#include "zbSocCmd.h"
#include "SimpleDBTxt.h"
#include "scene_manager.h"
#include "One_key_match.h"
#include "zbSocMasterControl.h"
#include "updateHW.h"
#include "mt_zbSocCmd.h"
#include "Tcp_client.h"
#include "timetask_manager.h"
#include "zbSocPrivate.h"
#include "PermissionManage.h"
#include "wifi_iwinfo.h"
#include "config_operate.h"

/************************************************************************************************
 * GLOBAL VALUE
 ************************************************************************************************/
bool g_getCoordVersion = false;

/************************************************************************************************
 * LOCAL FUNCTIONS
 ************************************************************************************************/
void SRPC_WinCurtainPercentageCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);
void SRPC_ComAlarmGetPowerCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);
void SRPC_SwitchQueryValueCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status);

/***************************************************************************************************
 * @fn      cmdMsgSend
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void cmdMsgSend(uint8_t* cmdMsg,uint16_t cmdMsgLen)
{   
 	tcp_client_send_msg(cmdMsg,cmdMsgLen);
    return;
}

/************************************************************************
* 函数名 :makeMsgHeader(hostCmd *cmd,uint8_t dir)
* 描述   :  设置数据包的包头数据
* 输入   ：*cmd:数据包指针,dir 为数据包的方向
* 输出   ：无
* 返回   ：uint8_t  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
bool makeMsgHeader(hostCmd *cmd,uint8_t dir)
{
    cmdSet8bitVal(cmd,CMD_MSG_FLAG);
    cmdSet16bitVal(cmd,0);//数据包长度先置为0
    cmdSet16bitVal(cmd,CMD_MSG_TP);
    cmdSetStringVal(cmd,roomfairy_WifiMac,6);
    cmdSet8bitVal(cmd,dir);
    return true;
}

/*****************************************************************************
 * 函 数 名  : makeMsgEnder
 * 负 责 人  : Edward
 * 创建日期  : 2016年11月28日
 * 函数功能  :  填充数据的长度及FCS校验
 * 输入参数  : hostCmd *cmd 
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
bool makeMsgEnder(hostCmd *cmd)
{
    cmd->data[CMD_MSG_LEN0_POS] = (((cmd->idx-3)>>8)&0xff);
    cmd->data[CMD_MSG_LEN1_POS] = ((cmd->idx-3)&0xff);
    cmdSetFCS(cmd);
    return true;
}

/************************************************************************
* 函数名 :SRPC_ConfigSsidPasswd(hostCmd *cmd)
* 描述   :   配置wifi的ssid和passwd
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/

static uint8_t SRPC_ConfigSsidPasswd(hostCmd *cmd)
{
    uint8_t ssidLen = 0,pwdLen = 0,authMod = 0,serverIPLen = 0;
    uint8_t ssid[32]= {0};
    uint8_t pwd[32]= {0};
    uint8_t serverIPAddr[128]= {0};
    uint8_t setwifi[128]={0};
    uint16_t serverPort = 0;
    uint8_t sPort[8] = {0};
	int8_t ret = -1;
	int cnt = 5;
	
	char *uciServer = NULL;
	char *uciPort = NULL;
	wfInfo_t *wfInfo = NULL;

    log_debug("SRPC_ConfigSsidPasswd--\n");

    cmdGet8bitVal(cmd, &ssidLen);
    cmdGetStringVal(cmd,ssid,ssidLen);
  
    cmdGet8bitVal(cmd, &pwdLen);
    cmdGetStringVal(cmd,pwd,pwdLen);

	cmdGet8bitVal(cmd,&authMod);

	//read servier address
	cmdGet8bitVal(cmd,&serverIPLen);
	cmdGetStringVal(cmd,serverIPAddr,serverIPLen);

	//read port
	cmdGet16bitVal(cmd,&serverPort);
	sprintf(sPort,"%d",serverPort);
    
#ifdef OPENWRT_TEST 

#if 0
	//扫描周围是否存在SSID
	while(cnt--)
	{
		wfInfo =  wifi_iwinfoScan("ra0",ssid,pwd);
		if(wfInfo != NULL)
			break;
	}

	//未扫描到SSID
	if(wfInfo == NULL)
	{
		while((cnt++)<5)
		{
			SRPC_ConfigSsidPasswdCfm(YY_STATUS_FAIL);
		}
		return 0;
	}
	//扫描到SSID
	else 
	{
		//备份配置文件
		wifiConfigbackups();

		while((cnt++)<5)
		{
			SRPC_ConfigSsidPasswdCfm(YY_STATUS_SUCCESS);
		}

		wireless_sta_config(wfInfo->ssid,wfInfo->encry,wfInfo->pwd,&wfInfo->channel);
		wireless_ap_disable(true);
		
		//定时重启网卡
		zbSoc_WifiRestart(4000);
		gateway_wireless_set(true);
	}

	log_debug("Device:%s\n",wfInfo->device);
	log_debug("ESSID:%s\n",wfInfo->ssid);
	log_debug("PASSWD:%s\n",wfInfo->pwd);
	log_debug("Encry:%s\n",wfInfo->encry);
	log_debug("Ch:%d\n",wfInfo->channel);
	
	wifi_iwinfoDestroy(wfInfo);

#else
	char *encry = NULL;
	char *pwds = NULL;
	
	if ((authMod != AUTH_OPEN || authMod != AUTH_SHARED)&&(pwdLen < 6))
	{
		log_err("password length failed\n");
		SRPC_ConfigSsidPasswdCfm(YY_STATUS_FAIL);
		return 1;
	}

	if(authMod == AUTH_OPEN || authMod == AUTH_SHARED)
	{
		encry = NULL;
		pwds = NULL;
	}
	else if(authMod == AUTH_PSK)
	{
		encry = strdup("psk");
		pwds = pwd;
	}
	else if(authMod == AUTH_PSK2)
	{
		encry = strdup("psk2");
		pwds = pwd;	
	}
	else
	{
		log_err("authmod failed\n");
		SRPC_ConfigSsidPasswdCfm(YY_STATUS_FAIL);
		return 1;
	}
	
	log_debug("SSID:%s PWD:%s encry:%s\n",ssid,pwds==NULL?"none":pwds,encry==NULL?"none":encry);
	log_debug("Address:%s Port:%s\n",serverIPAddr,sPort);
	
	//config seriver ip and port
	if(gateway_server_config(serverIPAddr,sPort)!=0)
	{
		log_err("gateway_server_config failed\n");
		SRPC_ConfigSsidPasswdCfm(YY_STATUS_FAIL);
		return 1;
	}

	//back wireless config
	wifiConfigbackups();
	
	SRPC_ConfigSsidPasswdCfm(YY_STATUS_SUCCESS);
	
	wireless_sta_config(ssid,encry,pwd,NULL);
	wireless_ap_disable(true);

	zbSoc_WifiRestart(4000);
	
	gateway_wireless_set(true);
	gateway_Binding_set(true);

	if(pwds != NULL)
		free(pwds);
	if(encry != NULL)
		free(encry);

#endif
#endif

	log_debug("SRPC_ConfigSsidPasswd++\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_ConfigEthernetCmdReq(hostCmd *cmd)
* 描述   :   配置有线路由方式
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_ConfigEthernetCmdReq(hostCmd *cmd)
{

    uint8_t serverIPLen =0 ;
    uint8_t serverIPAddr[128]= {0};
    uint16_t serverPort  =0 ;
    uint8_t sPort[8] = {0};
    int cnt = 0;
	
	cmdGet8bitVal(cmd,&serverIPLen);
	
	if(serverIPLen > 128)
	{
		SRPC_ConfigEthernetCmdCfm(YY_STATUS_FAIL);
		return -1;
	}
	
	cmdGetStringVal(cmd,serverIPAddr,serverIPLen);
	cmdGet16bitVal(cmd,&serverPort);
    sprintf(sPort,"%d",serverPort);
    
	log_debug("SRPC_ConfigEthernetCmdReq++\n");
    
#ifdef OPENWRT_TEST 

	if(gateway_server_config(serverIPAddr,sPort)!=0)
	{
		SRPC_ConfigEthernetCmdCfm(YY_STATUS_FAIL);
		return 1;
	}
		
	//back wireless config
	wifiConfigbackups();
	wireless_ap_disable(true);
	gateway_Binding_set(true);

	while((cnt++)<5)
	{
		SRPC_ConfigEthernetCmdCfm(YY_STATUS_SUCCESS);
	}
	
	zbSoc_WifiRestart(5000);//3

#endif

    log_debug("SRPC_ConfigEthernetCmdReq--\n");
	
}

/************************************************************************
* 函数名 :SRPC_ConfigApSSidCmdReq(hostCmd *cmd)
* 描述   :   配置主机无线热点
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_ConfigApSSidCmdReq(hostCmd *cmd)
{
	int ret = 0;
    uint8_t ApSSidFlag = 0;

    log_debug("SRPC_ConfigApSSidCmdReq++\n");

    cmdGet8bitVal(cmd, &ApSSidFlag);

#ifdef OPENWRT_TEST

    if(ApSSidFlag==0)
    {
		//关闭热点
    	ret = wireless_ap_disable(true);
    }
    else
    {
		//启动热点
    	ret = wireless_ap_disable(false);
    }

	if(!ret)
	{
		SRPC_ConfigApSSidCmdCfm(YY_STATUS_SUCCESS);
		zbSoc_WifiRestart(2000);
	}
	else
	{
		SRPC_ConfigApSSidCmdCfm(YY_STATUS_FAIL);
	}
    
#endif
 
    log_debug("SRPC_ConfigApSSidCmdReq--\n");

    return 0;
}


static uint8_t SRPC_ConfigGetVersionCmdCfm(char *version,uint8_t len)
{
	hostCmd cmd = {{0},0,0};
    cmd.idx = 0;
    
    log_debug("SRPC_ConfigGetVersionCmdCfm++\n");
	
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_CONFIG_GET_VERSION_CMD_CFM);
	//D2 版本字段长度
    cmdSet8bitVal(&cmd,len);
	//副版本号
	cmdSetStringVal(&cmd,version,len);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
	
    log_debug("SRPC_ConfigGetVersionCmdCfm--\n");

    return 0;
}

static uint8_t SRPC_ConfigGetVersionCmdReq(hostCmd *cmd)
{
    log_debug("SRPC_ConfigGetVersionCmdReq++\n");
	SRPC_ConfigGetVersionCmdCfm(VERSION,strlen(VERSION));
    log_debug("SRPC_ConfigGetVersionCmdReq--\n");
    return 0;
}

static uint8_t SRPC_ConfigModifySSIDCmdCfm(uint8_t status)
{
	hostCmd cmd;
	cmd.idx = 0;

	log_debug("SRPC_ConfigModifySSIDCmdCfm++\n");
	
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_MODIFY_SSID_CFM);
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
	
    log_debug("SRPC_ConfigModifySSIDCmdCfm--\n");

    return 0;
}

static uint8_t SRPC_ConfigModifySSIDCmdReq(hostCmd *cmd)
{
	uint8_t ssidlength = 0; 
	uint8_t ssidName[30] = {0};
	int ret = 0;
    log_debug("SRPC_ConfigModifySSIDCmdReq++\n");
	
	cmdGet8bitVal(cmd,&ssidlength);

	//判断数据是否符合长度
	if ((ssidlength==0) && (ssidlength >30))
	{
		SRPC_ConfigModifySSIDCmdCfm(YY_STATUS_FAIL);
		return 0;
	}
		
	cmdGetStringVal(cmd,ssidName,ssidlength);
		
#ifdef OPENWRT_TEST
	ret = wireless_ap_setSSID(ssidName);
	if(!ret)
	{
		SRPC_ConfigModifySSIDCmdCfm(YY_STATUS_SUCCESS);
		zbSoc_WifiRestart(2000);
	}
	else
	{
		SRPC_ConfigModifySSIDCmdCfm(YY_STATUS_FAIL);
	}
	
#endif

	log_debug("SRPC_ConfigModifySSIDCmdReq--\n");
		
    return 0;
}


static uint8_t SRPC_ConfigModifyPassWordCmdCfm(uint8_t status)
{
	hostCmd cmd;
    cmd.idx = 0;
	log_debug("SRPC_ConfigModifyPassWordCmdCfm++\n");
	
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_MODIFY_PASSWORD_CFM);
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
	
    log_debug("SRPC_ConfigModifyPassWordCmdCfm--\n");

    return 0;
}

static uint8_t SRPC_ConfigModifyPassWordCmdReq(hostCmd *cmd)
{
	int ret = -1;
	uint8_t passauth = 0;
	uint8_t passlength = 0; 
	uint8_t password[30] = {0};
	uint8_t uciCmd[128]={0};
    log_debug("SRPC_ConfigModifyPassWordCmdReq++\n");
	
	cmdGet8bitVal(cmd,&passauth);
	
	if (passauth==AUTH_OPEN||passauth==AUTH_SHARED)
	{
	
#ifdef OPENWRT_TEST
		ret = wireless_ap_setPasswd("none",NULL);
#endif

	}
	else
	{
		cmdGet8bitVal(cmd,&passlength);

		//判断数据是否符合长度
		if((passlength < 8)||(passlength > 30))
		{
			SRPC_ConfigModifyPassWordCmdCfm(YY_STATUS_FAIL);
			return 0;
		}
		else
		{
			cmdGetStringVal(cmd,password,passlength);
			
#ifdef OPENWRT_TEST
			if(passauth==AUTH_PSK)
			{
				ret = wireless_ap_setPasswd("psk",password);
			}
			else if(passauth==AUTH_PSK2)
			{
				ret = wireless_ap_setPasswd("psk2",password);
			}
#endif	
		}
	}
	
#if OPENWRT_TEST

	if(!ret)
	{
		SRPC_ConfigModifyPassWordCmdCfm(YY_STATUS_SUCCESS);
		zbSoc_WifiRestart(2000);
	}
	else
	{
		SRPC_ConfigModifyPassWordCmdCfm(YY_STATUS_FAIL);
	}
	
#endif
	log_debug("SRPC_ConfigModifyPassWordCmdReq--\n");
	
    return 0;
}


void SRPC_SystemRebootCfm(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_SYSTEM_REBOOT_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static uint8_t  SRPC_SystemRebootReq(hostCmd *cmd)
{
    log_debug("SRPC_SystemRebootReq--\n");
	SRPC_SystemRebootCfm(YY_STATUS_SUCCESS);
	//重启协调器
//	zbSoc_SetCoordResetCmd();
	mt_Sys_Reset_sreq();
//	sleep(10);

#if OPENWRT_TEST
	zbSoc_SystemReboot(2000);
#endif

	return 0;
}

/************************************************************************
* 函数名 :SRPC_GetMacAddrCmdReq(hostCmd *cmd)
* 描述   :   获取路由器的Mac地址
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_GetMacAddrCmdReq(hostCmd *cmd)
{
    log_debug("SRPC_GetMacAddrCmdReq++\n");


    SRPC_GetMacAddrCfm(&roomfairy_WifiMac[0]);


    log_debug("SRPC_GetMacAddrCmdReq--\n");
	return 0;
}

uint8_t SRPC_GetCoorVersionCmdInd(uint8_t version)
{
	hostCmd cmd;
    cmd.idx = 0;
    log_debug("SRPC_GetCoorVersionCmdInd++\n");
	
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_GET_COORD_VERSION_CMD_IND);
    cmdSet8bitVal(&cmd,version);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
    log_debug("SRPC_GetCoorVersionCmdInd--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_GetCoordVersionCmdReq(hostCmd *cmd)
* 描述   :   获取设备的地址
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_GetCoordVersionCmdReq(hostCmd *cmd)
{
    log_debug("SRPC_GetCoordVersionCmdReq++\n");

	g_getCoordVersion = true;

    zbSoc_GetCoordVersionCmd();

    log_debug("SRPC_GetCoordVersionCmdReq--\n");
	return 0;
}

/************************************************************************
* 函数名 :SRPC_permitJoin(hostCmd *cmd)
* 描述   :   开放zigbee 网络，允许节点设备入网
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
//开放zigbe网络
static uint8_t SRPC_permitJoin(hostCmd *cmd)
{
    uint8_t openTimes;
    log_debug("SRPC_permitJoin++\n");

    cmdGet8bitVal(cmd, &openTimes);

//    Open the network for joining
//    zbSoc_OpenNwkCmd(openTimes);

	zbSoc_Permit_Join_Req(openTimes);

    SRPC_OpenNwkCfm(YY_STATUS_SUCCESS);

    log_debug("SRPC_permitJoin--\n");

    return 0;
}

/************************************************************************
* 函数名 :SRPC_OpenNwkCfm(uint8 status)
* 描述   :   开放网络命令
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
//关闭网络确认
void SRPC_CloseNwkCfm(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_CLOSE_NETWORK_CMD_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_permitJoin(hostCmd *cmd)
* 描述   :   开放zigbee 网络，允许节点设备入网
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
//关闭zigbee网络
static uint8_t SRPC_CloseNetWork(hostCmd *cmd)
{
    uint8_t openTimes = 0;
    log_debug("SRPC_CloseNetWork++\n");

//  cmdGet8bitVal(cmd, &openTimes);

//	Open the network for joining
//  zbSoc_OpenNwkCmd(openTimes);

  	zbSoc_Permit_Join_Req(openTimes);

    SRPC_CloseNwkCfm(YY_STATUS_SUCCESS);

    log_debug("SRPC_CloseNetWork--\n");

    return 0;
}

/************************************************************************
* 函数名 :SRPC_RevertFactorySettingReq(hostCmd *cmd)
* 描述   :   恢复所有设备至出厂设置
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
//zigbee恢复出厂设置
static uint8_t SRPC_RevertFactorySettingReq(hostCmd *cmd)
{
    log_debug("SRPC_RevertFactorySettingReq++\n");

    //Revert One Device Factory Sertting
    zbSoc_RevertFactorySettingCmd();

    SRPC_RevertFactorySettingCfm(YY_STATUS_SUCCESS);

    log_debug("SRPC_RevertFactorySettingReq--\n");

    return 0;
}

//判断是否是门锁/主控设备复位
int8_t SRPC_FactorySettingReq(epInfo_t *epInfo)
{
	epInfo_t srcepInfo;

	if(epInfo== NULL)
		return -1;
		
	memcpy(&srcepInfo,epInfo,sizeof(epInfo_t));
	log_debug("SRPC_FactorySettingReq++\n");
	//epInfo = devListGetDeviceByIeeeDeviceID(mEpInfo.IEEEAddr,ZB_DEV_LEVEL_DOORLOCK);

	//判断当前的设备是0X010A门锁设备
	epInfo = devListGetDeviceByIeeeEp(srcepInfo.IEEEAddr,0x08);
	
	if(epInfo != NULL) //是0X010A门锁或主控
	{
//		if((epInfo->deviceID == ZB_DEV_LEVEL_DOORLOCK)||(epInfo->deviceID == ZB_DEV_MASTER_CONTROL))
		if(epInfo->deviceID == ZB_DEV_MASTER_CONTROL)
		{
			zbSoc_RevertOneDevFactorySettingCmd(epInfo->nwkAddr,epInfo->endpoint);
		}
		else
		{
			zbSoc_RevertOneDevFactorySettingCmd(srcepInfo.nwkAddr,srcepInfo.endpoint);
		}
	}
	else	//没找到0X010A门锁
	{
		zbSoc_RevertOneDevFactorySettingCmd(srcepInfo.nwkAddr,srcepInfo.endpoint);
	}
	log_debug("SRPC_FactorySettingReq--\n");
	
	return 0;
}

/************************************************************************
* 函数名 :SRPC_RevertOneFactorySettingReq(hostCmd *cmd)
* 描述   :   恢复单个设备至出厂设置
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
//指定设备恢复出厂设置
static uint8_t SRPC_RevertOneFactorySettingReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    epInfo_t *epInfo;

    log_debug("SRPC_RevertOneFactorySettingReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo != NULL)
	{
		if(epInfo->onlineflag == true)
		{
			//Revert One Device Factory Sertting
		    //zbSoc_RevertOneDevFactorySettingCmd(epInfo->nwkAddr,epInfo->endpoint);
		    SRPC_RevertOneFactorySettingCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_RevertOneFactorySettingCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
#if ( USE_MASTER_CONTROL)
		SRPC_FactorySettingReq(epInfo);
#else
		zbSoc_RevertOneDevFactorySettingCmd(epInfo->nwkAddr,epInfo->endpoint);
#endif
	}
	else
	{
		SRPC_RevertOneFactorySettingCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_RevertOneFactorySettingReq--\n");

    return 0;
}

void SRPC_SetCoordResetInd(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_SET_COORD_RESET_IND);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_SetCoordResetCfm(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_SET_COORD_RESET_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


static uint8_t  SRPC_SetCoordResetReq(hostCmd *cmd)
{
    log_debug("SRPC_SetCoordResetReq--\n");

	//重启协调器
	//zbSoc_SetCoordResetCmd();
	mt_Sys_Reset_sreq();
	//sleep(10);
	SRPC_SetCoordResetCfm(YY_STATUS_SUCCESS);
	
	log_debug("SRPC_SetCoordResetReq--\n");
	return 0;
}


uint8_t SRPC_UpDateMasterHostFWCfm(uint8_t result,uint8_t actionOfprogress,uint8_t progress )
{
	hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_SET_MASTER_HOST_UPDATE_CFM);
	//D2 result
	cmdSet8bitVal(&cmd,result);

	if(result == YY_STATUS_UP_PROGRESS)
	{
		cmdSet8bitVal(&cmd,actionOfprogress);
		cmdSet8bitVal(&cmd,progress);
	}

	makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/*****************************************************************************
 * 函 数 名  : SRPC_UpDateMasterHostFW
 * 负 责 人  : Edward
 * 创建日期  : 2016年5月31日
 * 函数功能  : 远程升级主机功能
 * 输入参数  : hostCmd *cmd  接收到的数据
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
//更新主机固件
uint8_t SRPC_UpDateMasterHostFWReq(hostCmd *cmd)
{
	static uint8_t vflag = 0;


	static pthread_t thread;
	log_debug("SRPC_UpDateMasterHostFWReq++\n");
	if(NULL == cmd)
	{
		SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
		return 1;
	}
	
	log_debug("SRPC_UpDateMasterHostFWReq++1\n");
	
	cmdGet8bitVal(cmd,&vflag);
	
	//防止连续点击，重复下载
	if(start_download == false)
	{
		//创建线程开始下载文件
		if((1 == pthread_create(&thread,NULL,&updateHW_Update_Pthread,&vflag)))
		{
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
		}
	}
	log_debug("SRPC_UpDateMasterHostFWReq--\n");
	
	return 0;
}


uint8_t SRPC_MtUtilSetChannelsCfm(uint8_t result)
{
	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_SET_COORD_CHANNELS_CFM);
	//D2 result
	cmdSet8bitVal(&cmd,result);

	makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_MtUtilSetChannelsTimeOutProcess(tu_timer_t *mtUtilSetChannelsTimers)
{
	if( NULL==mtUtilSetChannelsTimers)
		return;
	log_debug("SRPC_MtUtilSetChannelsTimeOutProcess++\n");
	if(false != mt_getChangeCoordinatorChannels())
	{
		SRPC_MtUtilSetChannelsCfm(YY_STATUS_FAIL);
		mt_setChangeCoordinatorChannels(false);
	}
	log_debug("SRPC_MtUtilSetChannelsTimeOutProcess--\n");
}

uint8_t SRPC_MtUtilSetChannelsReq(hostCmd *cmd)
{
	log_debug("SRPC_MtUtilSetChannelsReq++\n");
	
	static tu_evtimer_t mtUtilSetChannelsTimers = {NULL,NULL,NULL,NULL,-1,false,false};
	
	uint8_t channel = 0;
	if(NULL != cmd)
	{
		cmdGet8bitVal(cmd,&channel);
		if((true == mt_getChangeCoordinatorChannels())||(mtUtilSetChannelsTimers.in_use == true))
			return 0;
		log_debug("SRPC_MtUtilSetChannelsReq++2\n");
		if(0 == mt_Util_Set_Channels_sreq(channel))
		{
			log_debug("SRPC_MtUtilSetChannelsReq++3\n");

			//启动定时器，超时5s钟处理
			tu_set_evtimer(&mtUtilSetChannelsTimers,SERIAL_CMD_TIMEOUT,false,(timer_handler_cb_t)SRPC_MtUtilSetChannelsTimeOutProcess,(void*)(&mtUtilSetChannelsTimers));
			
			return 0;
		}
	}
	log_debug("SRPC_MtUtilSetChannelsReq--\n");
	SRPC_MtUtilSetChannelsCfm(YY_STATUS_FAIL);
}

uint8_t SRPC_GetCoordPanidAndChannelsReq(hostCmd *cmd)
{
	mt_Zdo_Ext_Nwk_Info_sreq();
}

uint8_t SRPC_GetCoordPanidAndChannelsInd(uint8_t IEEEAddrs[8],uint16_t Panid,uint16_t channels,uint8_t dev_state)
{
	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_SET_COORD_CHANNELS_IND);

	//IEEE Address
	cmdSetStringVal(&cmd,IEEEAddrs,8);
	//Pandid
	cmdSet16bitVal(&cmd,Panid);
	//channels
	cmdSet8bitVal(&cmd,channels);
	//Dev_State
	cmdSet8bitVal(&cmd,dev_state);

	makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_RoomfairyRegisterResp(hostCmd *cmd)
* 描述   :   roomfairy主机注册的反馈结果
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_RoomfairyRegisterResp(hostCmd *cmd)
{
    uint8_t status;
    epInfo_t* epInfo = NULL;
    
    uint8_t endPoint=0;
    uint8_t ieeeAddr[8];
    
    log_debug("SRPC_RoomfairyRegisterResp++\n");
    
    cmdGet8bitVal(cmd, &status);
    memset(ieeeAddr,0,8*sizeof(uint8_t));

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo != NULL)
	{
		if (status == 0x00)
		{
			if(epInfo->registerflag != true)
			{
				epInfo->registerflag = true; ////设备允许入网
				vdevListModifyByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint,epInfo);
	            devListModifyRecordByIeeeEp(epInfo->IEEEAddr,epInfo->endpoint,epInfo);
			}
			roomfairy_registerFlag = true;
			log_debug("[RoomfairyRegister] Success 1.\n");
			ZbSocHeartbeat_HeartPacketSend();
		}
		else
		{
			log_err("[RoomfairyRegister] Failed 2.\n");
		}		
	}
	else
	{
		log_err("[RoomfairyRegister] Failed 3.\n");
	}
	
    log_debug("SRPC_RoomfairyRegisterResp--\n");
    return 0;
}

/***************************************************************************************************
 * @fn      SRPC_RoomfairyRegisterInd
 *
 * @brief   发送zigbee插座的注册信息到服务器

 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
static void SRPC_RoomfairyRegisterInd(int binding)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ROOMFAIRY_REGISTER_IND);
    cmdSet8bitVal(&cmd,binding);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
    log_debug("SRPC_RoomfairyRegisterInd++\n");
}

/***************************************************************************************************
 * @fn      SRPC_RoomfairyRegistered
 *
 * @brief   发送zigbee插座的注册信息到服务器

 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_RoomfairyRegistered(void)
{
	epInfo_t epInfo;
	int binding = 0;
	char *sbinding = NULL;
	log_debug("SRPC_RoomfairyRegistered++\n");
	
	if(roomfairy_registerFlag == false)
	{
		sbinding = gateway_Binding_get();
		if((sbinding != NULL)&&(strstr(sbinding,"true")==0))
		{
			//未绑定
			binding = 0;
		}
		else
		{
			//已绑定用户
			binding = 1;
		}
		
		memset(&epInfo,0,sizeof(epInfo));

		//查找是否注册
		if(devListGetDeviceByNaEp(epInfo.nwkAddr,epInfo.endpoint) == NULL)
		{
		    log_debug("zigbeeDev_RoomfairyRegisterInd.\n");                
		    memset(&epInfo,0,sizeof(epInfo));
		    devListAddDevice(&epInfo);
		}

		//上报注册信息
		SRPC_RoomfairyRegisterInd(binding);
		//sleep(3);//发送间隔开
	}
	
	log_debug("SRPC_RoomfairyRegistered--\n");
}


/************************************************************************
* 函数名 :SRPC_heartPacketResp(hostCmd *cmd)
* 描述   :   心跳包反馈(socket保持长连接机制)
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_heartPacketResp(hostCmd *cmd)
{
    log_debug("=======SRPC_heartPacketResp======\n"); 
    return 0;
}

/************************************************************************
* 函数名 :SRPC_QueryDevOnlineStateReq(hostCmd *cmd)
* 描述   :   roomfairy主机注册的反馈结果
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_QueryDevOnlineStateReq(hostCmd *cmd)
{
    log_debug("SRPC_QueryDevOnlineStateReq++\n");
//  HeartPacketSend();
	ZbSocHeartbeat_HeartPacketSend();

    log_debug("SRPC_QueryDevOnlineStateReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_AllowDevConnectNetworkReq(hostCmd *cmd)
* 描述   :   服务器允许节点设备入网的命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_AllowDevConnectNetworkReq(hostCmd *cmd)
{
    uint8_t devCount;
    uint8_t devIEEEAddr[8],devEndPoint;
    uint8_t cnt = 0;
    
    epInfo_t  *epInfo = NULL;
	
    hostCmd sendCmd;
    
    sendCmd.idx = 0;

    log_debug("SRPC_AllowDevConnectNetworkReq++\n");
    
    cmdGet8bitVal(cmd, &devCount);

    // 作成应答数据包
    makeMsgHeader(&sendCmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&sendCmd,WIFI_ALLOW_DEV_CONN_NET_CFM);
    cmdSet8bitVal(&sendCmd, devCount);

    for(cnt=0; cnt<devCount; cnt++)
    {
        cmdGetStringVal(cmd, devIEEEAddr,8);
		log_debug("IEEE:");
		log_debug_array(devIEEEAddr,8,":");
		
        cmdGet8bitVal(cmd, &devEndPoint);

        epInfo = devListGetDeviceByIeeeEp(devIEEEAddr,devEndPoint);

        //作成数据包
        cmdSetStringVal(&sendCmd, devIEEEAddr,8);
        cmdSet8bitVal(&sendCmd, devEndPoint);

        if(epInfo != NULL)
        {
            epInfo->registerflag = true; //设备允许入网
			vdevListModifyByIeeeEp(devIEEEAddr,devEndPoint,epInfo);
			devListModifyRecordByIeeeEp(devIEEEAddr,devEndPoint,epInfo);
            //注册状态
            cmdSet16bitVal(&sendCmd, epInfo->deviceID);
            cmdSet8bitVal(&sendCmd, YY_STATUS_SUCCESS);
        }
        else
        {
            //注册状态,节点不存在
            cmdSet16bitVal(&sendCmd,0xffff);
            cmdSet8bitVal(&sendCmd, YY_STATUS_NODE_NO_EXIST);
        }
    }

    //作成数据包
    makeMsgEnder(&sendCmd);
    cmdMsgSend(sendCmd.data,sendCmd.idx);

    //允许设备入网后，立即更新发送下心跳包
	ZbSocHeartbeat_HeartPacketSend();

    log_debug("SRPC_AllowDevConnectNetworkReq--\n");

    return 0;
}

/************************************************************************
* 函数名 :SRPC_QueryAllUnregisterDevReq(hostCmd *cmd)
* 描述   :   查询所有未注册设备的命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_QueryAllUnregisterDevReq(hostCmd *cmd)
{
    uint8_t devCount;
    uint16_t bufLen;
    uint8_t buf[MaxPacketLength] = {0};

    log_debug("SRPC_QueryAllUnregisterDevReq++\n");

    devCount = vdevListGetAllUnRegOnlineDev(buf,&bufLen);

    SRPC_QueryAllUnregisterDevCfm(devCount,buf,bufLen);

    log_debug("SRPC_QueryAllUnregisterDevReq--\n");
	return 0;
}

/************************************************************************
* 函数名 :SRPC_DeleteDevConnectNetworkReq(hostCmd *cmd)
* 描述   :   服务器删除节点设备入网的命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_DeleteDevConnectNetworkReq(hostCmd *cmd)
{
    uint8_t devCount;
    uint8_t devIEEEAddr[8],devEndPoint;
    uint8_t cnt;
    epInfo_t *epInfo;

    hostCmd sendCmd;
    sendCmd.idx = 0;

    log_debug("SRPC_DeleteDevConnectNetworkReq++\n");
    cmdGet8bitVal(cmd, &devCount);

    // 作成应答数据包
    makeMsgHeader(&sendCmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&sendCmd,WIFI_DELETE_DEV_CONN_NET_CFM);

	//设置节点个数
	cmdSet8bitVal(&sendCmd, devCount);

    for(cnt=0; cnt<devCount; cnt++)
    {
        cmdGetStringVal(cmd, devIEEEAddr,8);
		
		log_debug("\nIEEE:");
		log_debug_array(devIEEEAddr,8,":");
		
        cmdGet8bitVal(cmd, &devEndPoint);

        //epInfo = devListGetDeviceByIeeeEp(devIEEEAddr,devEndPoint);
		epInfo = vdevListGetDeviceByIeeeEp(devIEEEAddr,devEndPoint);
			
        //作成数据包
        cmdSetStringVal(&sendCmd, devIEEEAddr,8);
        cmdSet8bitVal(&sendCmd, devEndPoint);

        if(epInfo != NULL)
        {
            epInfo->registerflag = false; //设备删除入网
            vdevListModifyByIeeeEp(devIEEEAddr,devEndPoint,epInfo);
            devListModifyRecordByIeeeEp(devIEEEAddr,devEndPoint,epInfo);
            //注册状态
            cmdSet8bitVal(&sendCmd, YY_STATUS_SUCCESS);
        }
        else
        {
            //注册状态,节点不存在
            cmdSet8bitVal(&sendCmd, YY_STATUS_NODE_NO_EXIST);
        }

    }
	
    //作成数据包
    makeMsgEnder(&sendCmd);
    cmdMsgSend(sendCmd.data,sendCmd.idx);

    log_debug("SRPC_DeleteDevConnectNetworkReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_ComDevRegisterResp(hostCmd *cmd)
* 描述   :   通用节点注册的反馈结果
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_ComDevRegisterResp(hostCmd *cmd)
{
    log_debug("WIFI_ZIGBEE_COM_DEV_REGISTER_RESP.\n");
	return 0;
}

void SRPC_GetDevVersionCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_VERSION_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_GetDevVersionInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t version)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_VERSION_IND);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,version);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static int8_t SRPC_GetSpecialDevVersionReq(epInfo_t *epInfo)
{
	epInfo_t *epinfo = NULL;
//	vepInfo_t *vepInfo = NULL;

	ASSERT(epInfo != NULL);
		
	//memcpy(&srcepInfo,epInfo,sizeof(vepInfo_t));
	
	log_debug("SRPC_GetSpecialDevVersionReq++\n");

	//判断当前的设备是0X010A门锁设备
	//epInfo = devListGetDeviceByIeeeDeviceID(mEpInfo.IEEEAddr,ZB_DEV_LEVEL_DOORLOCK);
	epinfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,0x08);
	
	if(epinfo != NULL) //是0X010A门锁或主控
	{
//		mepInfo = &(vepInfo->epInfo);
//		if((epInfo->deviceID == ZB_DEV_LEVEL_DOORLOCK)||(epInfo->deviceID == ZB_DEV_MASTER_CONTROL))
		if(epinfo->deviceID == ZB_DEV_MASTER_CONTROL)
		{
			zbSoc_QueryDeviceVerion(epinfo->nwkAddr,epinfo->endpoint,afAddr16Bit);
		}
		else
		{
			zbSoc_QueryDeviceVerion(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
	}
	else
	{
		zbSoc_QueryDeviceVerion(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
	}
	
	log_debug("SRPC_GetSpecialDevVersionReq--\n");
}

static uint8_t SRPC_GetDevVersionReq(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
	uint8_t endPoint;
//	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo  = NULL;

	log_debug("SRPC_GetDevVersionReq.\n");
   
    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if(epInfo != NULL)
	{
//		epInfo = &(vepInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			//Revert One Device Factory Sertting
			SRPC_GetDevVersionCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_GetDevVersionCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		
#if ( USE_MASTER_CONTROL)
		SRPC_GetSpecialDevVersionReq(epInfo);
#else
		zbSoc_QueryDeviceVerion(epInfo->nwkAddr,epInfo->endPoint,afAddr16Bit);
#endif

	}
	else
	{
		SRPC_GetDevVersionCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}
	
	return 0;
}

/************************************************************************
* 函数名 :SRPC_PeriodTimerSwitchCtrlReq(hostCmd *cmd)
* 描述   :   周期定时器节点控制
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static int8_t SRPC_addPeriodTimerSwitchCtrlReq(hostCmd *cmd)
{
	uint8_t i = 0;

	timeTaskRecord_t timerTaskInfo;
	nodeMembersRecord_t member[MAX_SUPPORTED_NODE_MEMBERS] = {0};
	nodeMembersRecord_t ** nextMemberPtr;

	memset(&timerTaskInfo,0,sizeof(timeTaskRecord_t));
	nextMemberPtr =  &timerTaskInfo.members;
	
    log_debug("SRPC_addPeriodTimerSwitchCtrlReq++\n");

	cmdGet8bitVal(cmd, &timerTaskInfo.timeTaskId);
	cmdGet8bitVal(cmd, &timerTaskInfo.timeTaskState);
	cmdGet8bitVal(cmd, &timerTaskInfo.timerType);//定时类型
	cmdGet8bitVal(cmd, &timerTaskInfo.hours);
	cmdGet8bitVal(cmd, &timerTaskInfo.minute);
	cmdGet8bitVal(cmd, &timerTaskInfo.second);
	cmdGet8bitVal(cmd, &timerTaskInfo.timeTaskType);//事件类型
	
	if(timerTaskInfo.timeTaskType!=EVENT_TYPE_SCENEID_ACTION && timerTaskInfo.timeTaskType!=EVENT_TYPE_NODE_ACTION )
	{
		log_err("add error\n");
		SRPC_PeriodTimerSwitchCtrlCfm(YY_STATUS_FAIL,0);
		return -1;
	}
	else if(timerTaskInfo.timeTaskType ==EVENT_TYPE_SCENEID_ACTION) //场景任务
	{
		timerTaskInfo.memberscount = 0x01;
		cmdGet8bitVal(cmd, &timerTaskInfo.sceneid);
	}
	else
	{
		//任务数
		cmdGet8bitVal(cmd,&timerTaskInfo.memberscount);
		
		for (i = 0;(i < timerTaskInfo.memberscount) && (i < MAX_SUPPORTED_NODE_MEMBERS); i++)
		{
			*nextMemberPtr = &(member[i]);
			cmdGetStringVal(cmd,&member[i].IEEEAddr[0],8);
			cmdGet8bitVal(cmd, &member[i].endpoint);
			cmdGet8bitVal(cmd, &member[i].length);//获取数据段长度
			cmdGetStringVal(cmd,&member[i].dataSegment[0],member[i].length);
			
			nextMemberPtr = &(member[i].next);
		}
		
		*nextMemberPtr = NULL;
	}

	//添加新的任务
	if(timerTaskInfo.timeTaskId == NEW_TIMER_TASK)
	{
		timerTask_AddTask(&timerTaskInfo);
	}
	else //修改
	{
		timerTask_ModifyRecordByID(&timerTaskInfo);
	}

	log_debug("SRPC_addPeriodTimerSwitchCtrlReq--\n");
	
    return 0;
}

/************************************************************************
* 函数名 :SRPC_DeletePeriodTimerSwitchCtrlReq(hostCmd *cmd)
* 描述   :   删除周期定时器任务
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_deletePeriodTimerSwitchCtrlReq(hostCmd *cmd)
{

    uint8_t timeTaskId;
    log_debug("SRPC_deletePeriodTimerSwitchCtrlReq++\n");

    cmdGet8bitVal(cmd, &timeTaskId);
	timerTask_DeleteTask(timeTaskId);
	
    log_debug("SRPC_deletePeriodTimerSwitchCtrlReq--\n");

    return 0;
}


void SRPC_PeriodTimerSwitchCtrlInd(uint8_t devCount,uint8_t* pBuf,uint16_t pBufLen)
{
	hostCmd cmd;
	cmd.idx = 0;
	makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
	//D0 D1 Opcode
	cmdSet16bitVal(&cmd,WIFI_ZIGBEE_PERIOD_TIMER_SWITCH_INFO_IND);
	//D2 devCount
	cmdSet8bitVal(&cmd,devCount);
	//发送所有的IEEE 值,IEEE len + Endpoint len=8+1=9
	cmdSetStringVal(&cmd,pBuf,pBufLen);
	makeMsgEnder(&cmd);
	cmdMsgSend(cmd.data,cmd.idx);
}


static uint8_t SRPC_getPeriodTimerSwitchCtrlReq(hostCmd *cmd)
{
//	uint8_t devNum = 0;
	//uint16_t mlength = 0;
//	uint16_t count = 0;

	//char rec[MAX_SUPPORTED_RECORD_SIZE];

	timeTaskListGetAllTask();
	//SRPC_PeriodTimerSwitchCtrlInd(devNum,rec,mlength);
	return 0;
}

static uint8_t SRPC_CtlPeriodTimerSwitchCtrlReq(hostCmd *cmd)
{
	uint8_t timeTaskId = 0;
	uint8_t timeTaskState = 0;

	cmdGet8bitVal(cmd,&timeTaskId);
	cmdGet8bitVal(cmd,&timeTaskState);
	timerTask_ControlTask(timeTaskId,timeTaskState);
}

void SRPC_DevSceneCfm(uint16_t opcode,uint8_t SceneID,uint8_t state)
{
	hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,opcode);
	cmdSet8bitVal(&cmd,SceneID);

    cmdSet8bitVal(&cmd,state);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_AddDevSceneReq(hostCmd *cmd)
* 描述   :   设备添加建组命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
#if 0
static uint8_t SRPC_AddDevSceneReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    epInfo_t *epInfo;
//   SPeriodTimers perTimer;
    log_debug("SRPC_AddDevSceneReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    //判断该IEEE地址设备是否存在
    log_debug("SRPC_AddDevSceneReq Success.\n");
    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

    //查询看下当前节点是否已经注册
    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_AddDevGroupCfm(YY_STATUS_UNREGISTER);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
        //点播添加设备到组
        zbSoc_AddGroupMemberCmd(0x01,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
        SRPC_AddDevGroupCfm(YY_STATUS_SUCCESS);
    }
    else
    {
        SRPC_AddDevGroupCfm(YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_AddDevSceneReq--\n");
    return 0;
}
#else
static uint8_t SRPC_AddDevSceneReq(hostCmd *cmd)
{
	int i;

	//场景描述结构
	sceneRecord_t newScene;
	//场景节点的描述，最大节点数量为100个
	sceneMembersRecord_t member[MAX_SUPPORTED_NODE_MEMBERS] = {0};
	sceneMembersRecord_t ** nextMemberPtr;

	memset(&newScene,0,sizeof(newScene));

	cmdGet8bitVal(cmd,&newScene.sceneId);//获取场景ID
	cmdGet8bitVal(cmd,&newScene.submbers);//获取任务节点数量
	
	nextMemberPtr =  &newScene.members;

	//任务节点数量大于0的时候才会继续操作
	if(newScene.submbers > 0)
	{
		for (i = 0;(i < newScene.submbers) && (i < MAX_SUPPORTED_NODE_MEMBERS); i++)
		{
			*nextMemberPtr = &(member[i]);
			cmdGetStringVal(cmd,&member[i].IEEEAddr[0],8);
			cmdGet8bitVal(cmd, &member[i].endpoint);
			cmdGet8bitVal(cmd, &member[i].length);//获取数据段长度
			cmdGetStringVal(cmd,&member[i].dataSegment[0],member[i].length);

			nextMemberPtr = &(member[i].next);
		}
		
		*nextMemberPtr = NULL; 

		//添加新的任务
		if(newScene.sceneId== NEW_TIMER_SCENE)
		{
			scene_addScene(&newScene);
		}
		//修改任务
		else 
		{
			scene_ModifyRecordByID(&newScene);
		}
	}
	else
	{
		SRPC_DevSceneCfm(WIFI_ZIGBEE_ADD_DEV_GROUP_CFM,newScene.sceneId,YY_STATUS_FAIL);
	}
	
	return 0;
}
#endif

static uint8_t SRPC_DelDevSceneReq(hostCmd *cmd)
{
	uint8_t sceneid = 0;
	cmdGet8bitVal(cmd, &sceneid);
	scene_DeleteScene(sceneid);
	return 0;
}

static void SRPC_DevSceneInd(uint8_t devCount,uint8_t* pBuf,uint16_t pBufLen)
{
	hostCmd cmd;
	cmd.idx = 0;
	makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
	//D0 D1 Opcode
	cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DEV_GROUP_IND);
	//D2 devCount
	cmdSet8bitVal(&cmd,devCount);
	//发送所有的IEEE 值,IEEE len + Endpoint len=8+1=9
	cmdSetStringVal(&cmd,pBuf,pBufLen);
	makeMsgEnder(&cmd);
	cmdMsgSend(cmd.data,cmd.idx);
}

static uint8_t SRPC_GetDevSceneReq(hostCmd *cmd)
{
	uint8_t devNum = 0;
	uint16_t mlength = 0;
	char rec[MAX_SUPPORTED_RECORD_SIZE];
	mlength = sceneListGetAllScene(&devNum,rec,MAX_SUPPORTED_RECORD_SIZE);
	SRPC_DevSceneInd(devNum,rec,mlength);
}

static uint8_t SRPC_CtlDevSceneReq(hostCmd *cmd)
{
	uint8_t sceneid = 0;
	cmdGet8bitVal(cmd,&sceneid);
	scene_Controlscene(sceneid);
	return 0;
}

void SRPC_setSwitchDefaultStateCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SET_SWITCH_DEFAULT_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_getSwitchDefaultStateCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_SWITCH_DEFAULT_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_SwitchDefaultStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SWITCH_DEFAULT_IND);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :SRPC_SetSwitchDefaultStateReq(hostCmd *cmd)
* 描述   :   设置开关的默认状态
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_SetSwitchDefaultStateReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,switchstate;
   
//    vepInfo_t *vepInfo = NULL;
    epInfo_t *epInfo = NULL;
    log_debug("SRPC_SwitchCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &switchstate);

    //断该IEEE地址设备是否存在
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
    
	if(epInfo!=NULL)
	{
		
//		pvepInfo = &(vepInfo->epInfo);
		//过滤功能
		if(epInfo->deviceID != ZB_DEV_ONOFF_SWITCH)
		{
			SRPC_setSwitchDefaultStateCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return -1;
		}
		
		if(epInfo->onlineflag == true)
		{
			SRPC_setSwitchDefaultStateCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_setSwitchDefaultStateCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
	 	log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
/*
#if USE_MASTER_CONTROL
		zbSoc_MasterControlSetOnOffState(epInfo,switchstate);
#else
		zbSoc_SetGenOnOffState(switchstate,epInfo->nwkAddr,endPoint,afAddr16Bit);
#endif
*/		zbSoc_SetGenOnOffDefaultState(switchstate,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		
	}
	else
	{
		SRPC_setSwitchDefaultStateCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_SwitchCtrlReq--\n");
    return 0;
}

static uint8_t SRPC_GetSwitchDefaultStateReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,switchstate;
//    vepInfo_t *vepInfo = NULL;
    epInfo_t *epInfo = NULL;
    
    log_debug("SRPC_SwitchCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &switchstate);

    //判断该IEEE地址设备是否存在
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
    
	if(epInfo!=NULL)
	{
//		pvepInfo = &(vepInfo->epInfo);

		if(epInfo->deviceID != ZB_DEV_ONOFF_SWITCH)
		{
			SRPC_setSwitchDefaultStateCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return -1;
		}
		
		if(epInfo->onlineflag == true)
		{
			SRPC_getSwitchDefaultStateCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_getSwitchDefaultStateCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
	 	log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
//#if USE_MASTER_CONTROL
//		zbSoc_MasterControlSetOnOffState(epInfo,switchstate);
//#else
//		zbSoc_SetGenOnOffState(switchstate,epInfo->nwkAddr,endPoint,afAddr16Bit);
//#endif
		zbSoc_getGenOnOffDefaultState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);

	}
	else
	{
		SRPC_getSwitchDefaultStateCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_SwitchCtrlReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_SwitchCtrlReq(hostCmd *cmd)
* 描述   :   开关节点控制
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_SwitchCtrlReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,switchstate;
//    vepInfo_t *vepInfo = NULL;
  	epInfo_t *epInfo = NULL;
    log_debug("SRPC_SwitchCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &switchstate);

    //断该IEEE地址设备是否存在
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
    
	if(epInfo == NULL)
	{
		log_err("vdevListGetDeviceByIeeeEp device no fonud \n");
		SRPC_SwitchCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
		return YY_STATUS_NODE_NO_EXIST;
	}

//	epInfo = &(vepInfo->epInfo);

	if(epInfo->onlineflag == true)
	{
		SRPC_SwitchCtrlCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
	}
	else
	{
		SRPC_SwitchCtrlCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
	}
	
	log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);

    zblist_add(epInfo,&switchstate,0x01);
		
#if USE_MASTER_CONTROL
	zbSoc_MasterControlSetOnOffState(epInfo,switchstate);
#else
	zbSoc_SetGenOnOffState(switchstate,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
#endif
		
    log_debug("SRPC_SwitchCtrlReq--\n");
    
    return YY_STATUS_SUCCESS;
}


/************************************************************************
* 函数名 :SRPC_QuerySwitchStatusReq(hostCmd *cmd)
* 描述   :   查询开关节点状态
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_QuerySwitchStatusReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
//    vepInfo_t *vepInfo = NULL;
    epInfo_t *epInfo = NULL;
    
    log_debug("SRPC_QuerySwitchStatusReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if(epInfo!=NULL)
	{
//		pvepInfo = &(vepInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			SRPC_SwitchQueryCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_SwitchQueryCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		

		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		
#if USE_MASTER_CONTROL
		zbSoc_MasterControlQueryOnOffState(epInfo);
#else
		zbSoc_QueryLightValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
#endif	

	}
	else
	{
		SRPC_SwitchQueryCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_QuerySwitchStatusReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_QueryAllSwitchStatusReq(hostCmd *cmd)
* 描述   :   查询所有开关节点状态
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
uint8_t SRPC_QueryAllSwitchStatusReq(hostCmd *cmd)
{
    log_debug("SRPC_QueryAllSwitchStatusReq++\n");

	zbSoc_QueryLightValueState(0xffff,0xff,afAddrBroadcast);
	
    return 0;
}


static uint8_t SRPC_QuerySwitchSocketValueReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    uint8_t srchmode = 0;

//    vepInfo_t *vepInfo = NULL;
    epInfo_t *epInfo = NULL;
    
    log_debug("SRPC_QuerySwitchSocketValueReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
	cmdGet8bitVal(cmd, &srchmode);

    
    log_debug("SRPC_QuerySwitchStatusReq Success.\n");

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if(epInfo!=NULL)
	{
//		pvepInfo = &(vepInfo->epInfo);

		////设备过滤，如果不是插座则返回
		if(epInfo->deviceID != ZB_DEV_ONOFF_PLUG)
		{
			log_debug("device is not ZB_DEV_ONOFF_PLUG");
			SRPC_SwitchQueryValueCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return -1;
		}
		
		if(epInfo->onlineflag == true)
		{
			SRPC_SwitchQueryValueCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_SwitchQueryValueCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}

		//将命令添加到重发机制列表中
//		zblist_add(epInfo,&srchmode,0x01);
		
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);

		zbSoc_QuerySwitchSocketValueState(epInfo->nwkAddr,endPoint,afAddr16Bit,srchmode);
	}
	else
	{
		SRPC_SwitchQueryValueCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_QuerySwitchSocketValueReq--\n");
    return 0;
}

void SRPC_SwitchSocketValueInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t type,uint32_t values)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_SIWTCH_VALUE_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,type);
    cmdSet32bitVal(&cmd,values);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_ExitIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_IRCLearnRetInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t addr)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_LEARN_STATUS_IND);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,addr);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_RemoteIRCLearnDataRetInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t dataLen,uint8_t *data)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_LEARN_DATA_IND);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet16bitVal(&cmd,dataLen);
    cmdSetStringVal(&cmd,data,dataLen);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_RemoteIrcCodeLibDataRetInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t type,uint16_t dataLen,uint8_t *data)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_CODE_LIB_IND);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,type);
    cmdSet16bitVal(&cmd,dataLen);
    cmdSetStringVal(&cmd,data,dataLen);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_RemoteIRCLearnRetInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_RemoteIRCLearnRetInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t dataLen,uint8_t *data)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_LEARN_STATUS_IND);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet16bitVal(&cmd,dataLen);
    cmdSetStringVal(&cmd,data,dataLen);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_SendIRCCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_SendIRCCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_SEND_CTRL_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_EnterRemoteIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_EnterRemoteIRCLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_ENTER_LEARN_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_ExitRemoteIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_ExitRemoteIRCLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_EXIT_LEARN_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_SendRemoteIRCCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_SendRemoteIRCCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_SEND_CTRL_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_SendRemoteIRCCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_RemoteIRCCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
	hostCmd cmd;
    cmd.idx = 0;
    
	log_debug("SRPC_RemoteIRCCfm++\n");

    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_DEVICE_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
   
}


/***************************************************************************************************
 * @fn      SRPC_EnterIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_EnterIRCLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_ENTER_LEARN_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_ExitIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_ExitIRCLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_EXIT_LEARN_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_ExitIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_SetIRCLearnAddrCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_SET_LEARN_ADDR_CFM);
    //D2-D9 
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 
    cmdSet8bitVal(&cmd,endpoint);
    //D11 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :SRPC_EnterIRCLearnModeReq(hostCmd *cmd)
* 描述   :   进入红外学习模式请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_EnterIRCLearnModeReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    epInfo_t *epInfo;
    log_debug("SRPC_EnterIRCLearnModeReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);


    log_debug("SRPC_EnterIRCLearnModeReq Success.\n");

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_EnterIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
        return 0;
    }

 
    if((epInfo != NULL)&&(epInfo->onlineflag != true))
    {
        SRPC_EnterIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
        zbSoc_IRCDevCtrlLearnModeCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,IRC_CTRL_ENTER_LEARN_MODE);
    }
    else
    {
        SRPC_EnterIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_EnterIRCLearnModeReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_ExitIRCLearnModeReq(hostCmd *cmd)
* 描述   :   退出红外学习模式请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_ExitIRCLearnModeReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    epInfo_t *epInfo;
    log_debug("SRPC_ExitIRCLearnModeReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

   
    log_debug("SRPC_ExitIRCLearnModeCfm Success.\n");

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

   
    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_ExitIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
        return 0;
    }

    
    if((epInfo != NULL)&&(epInfo->onlineflag != true))
    {
        SRPC_ExitIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
        zbSoc_IRCDevCtrlLearnModeCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,IRC_CTRL_EXIT_LEARN_MODE);
    }
    else
    {
        SRPC_ExitIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_ExitIRCLearnModeReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_SetIRCLearnAddrReq(hostCmd *cmd)
* 描述   :   设置红外学习地址请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_SetIRCLearnAddrReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,cmdAddr;
    epInfo_t *epInfo;
    log_debug("SRPC_SetIRCLearnAddrReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &cmdAddr);

   
    log_debug("SRPC_ExitIRCLearnModeCfm Success.\n");

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

   
    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_SetIRCLearnAddrCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
        return 0;
    }

    
    if((epInfo != NULL)&&(epInfo->onlineflag != true))
    {
        SRPC_SetIRCLearnAddrCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
        zbSoc_IRCDevCtrlSetLearnAddrCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,cmdAddr);
    }
    else
    {
        SRPC_SetIRCLearnAddrCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_SetIRCLearnAddrReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_SendIRCCtrlReq(hostCmd *cmd)
* 描述   :   发送红外控制命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_SendIRCCtrlReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,cmdAddr;
    epInfo_t *epInfo;
    log_debug("SRPC_SendIRCCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &cmdAddr);

    
    log_debug("SRPC_SendIRCCtrlReq Success.\n");

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

   
    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_SendIRCCtrlCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
        return 0;
    }

    
    if((epInfo != NULL)&&(epInfo->onlineflag != true))
    {
        SRPC_SendIRCCtrlCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
        zbSoc_IRCDevCtrlSendAddrCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,cmdAddr);
    }
    else
    {
        SRPC_SendIRCCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_SendIRCCtrlReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_EnterRemoteIRCLearnModeReq(hostCmd *cmd)
* 描述   :   进入远程红外学习模式请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_EnterRemoteIRCLearnModeReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    epInfo_t *epInfo;
    log_debug("SRPC_EnterRemoteIRCLearnModeReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    
    log_debug("SRPC_EnterRemoteIRCLearnModeReq Success.\n");

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

    
    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_EnterRemoteIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
        return 0;
    }

   
    if((epInfo != NULL)&&(epInfo->onlineflag != true))
    {
        SRPC_EnterRemoteIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
        zbSoc_RemoteIRCDevCtrlLearnModeCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,IRC_REMOTE_CTRL_ENTER_LEARN_MODE);
    }
    else
    {
        SRPC_EnterRemoteIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_EnterRemoteIRCLearnModeReq--\n");
    return 0;
}

void SRPC_IRC_RemoteLearnModeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_ENTER_LEARN_MODE_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_IRC_RemotelLearnModeReq(hostCmd *cmd)
* 描述   :   进入远程红外学习模式请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_IRC_RemotelLearnModeReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;

//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
    //pepInfo = &(vepInfo->epInfo);

    log_debug("SRPC_EnterRemoteIRCLearnModeReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
	//获取要学习设备的类型
	cmdGet8bitVal(cmd,&IRC_Remote_Learn_Device_Type);
	cmdGet8bitVal(cmd,&IRC_Remote_Learn_Device_Data_Type);
    
    log_debug("SRPC_EnterRemoteIRCLearnModeReq Success.\n");
	//判断该IEEE地址设备是否存在
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
#if 0		
		if(epInfo->onlineflag == true)
		{
//				//SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
//				log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
//				//清除上次学习的数据
//				IRC_Remote_Learn_Count = 0;
//				//发送学习命令
//		        zbSoc_IRC_RemoteIModeCtrlCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,0x01);
		}
		else
		{
			SRPC_RemoteIRCCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
#endif	
//		pepInfo = &(vepInfo->epInfo);
		//SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		//清除上次学习的数据
		IRC_Remote_Learn_Count = 0;
		//发送学习命令
		zbSoc_IRC_RemoteIModeCtrlCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,0x01);
	}
#if 0
	else
	{
		SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}
#endif

    log_debug("SRPC_EnterRemoteIRCLearnModeReq--\n");
    return 0;
}


/************************************************************************
* 函数名 :SRPC_ExitRemoteIRCLearnModeReq(hostCmd *cmd)
* 描述   :   退出远程红外学习模式请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_ExitRemoteIRCLearnModeReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    epInfo_t *epInfo;
    log_debug("SRPC_ExitRemoteIRCLearnModeReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    
    log_debug("SRPC_ExitRemoteIRCLearnModeReq Success.\n");

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

    
    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_ExitRemoteIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
        return 0;
    }

   
    if((epInfo != NULL)&&(epInfo->onlineflag != true))
    {
        SRPC_ExitRemoteIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
        zbSoc_RemoteIRCDevCtrlLearnModeCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,IRC_REMOTE_CTRL_EXIT_LEARN_MODE);
    }
    else
    {
        SRPC_ExitRemoteIRCLearnModeCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_ExitRemoteIRCLearnModeReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_SendRemoteIRCCtrlReq(hostCmd *cmd)
* 描述   :   发送远程红外控制命令请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_SendRemoteIRCCtrlReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    epInfo_t *epInfo;
    uint8_t irDataLen;
//	uint8_t data = 0;
    uint8_t irData[512]= {0};
    log_debug("SRPC_SendRemoteIRCCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &irDataLen);
	cmdGetStringVal(cmd,irData,irDataLen);
	
	

    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

    
    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_SendRemoteIRCCtrlCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
        return 0;
    }

   
    if((epInfo != NULL)&&(epInfo->onlineflag != true))
    {
        SRPC_SendRemoteIRCCtrlCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
		zbSoc_RemoteIRCDevCtrlCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,irDataLen,irData);
    }
    else
    {
        SRPC_SendRemoteIRCCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_SendRemoteIRCCtrlReq--\n");
    return 0;
}

//发送原始红外码数据
static uint8_t SRPC_SendRemoteIRCRawDataReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    hostCmd data;
    uint8_t irDataLen;

//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
    //pepInfo = &(vepInfo->epInfo);
	
    uint8_t irData[512]= {0};
    log_debug("SRPC_SendRemoteIRCRawDataReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &irDataLen);
	cmdGetStringVal(cmd,irData,irDataLen);
	
	
	//判断该IEEE地址设备是否存在
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_RemoteIRCCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
	
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		Creat_Irc_LearnCode_Packege(irData,irDataLen,&data);
		zbSoc_RemoteIRCDevCtrlCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,data.idx,data.data);
    }
	else
	{
		SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_SendRemoteIRCRawDataReq--\n");
    return 0;
}

void SRPC_QueryRCCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t deviceType,uint16_t deviceNameID,uint16_t deviceModel,uint16_t tableIndex)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_QUERY_DEVICE_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,deviceType);
	cmdSet16bitVal(&cmd,deviceNameID);
	cmdSet16bitVal(&cmd,deviceModel);
	cmdSet16bitVal(&cmd,tableIndex);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_QueryIRCDeviceWithLibReq(hostCmd *cmd)
* 描述   :   搜索匹配红外设备
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_QueryIRCDeviceWithLibReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
	uint8_t DeviceType ;//产品类型
	uint16_t DevcieNameId;//设备名称ID
	uint16_t DeviceModel;//产品的型号
	uint16_t TableIndex =0;
//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
    //pepInfo = &(vepInfo->epInfo);
	hostCmd data;
	
	log_debug("SRPC_QueryIRCDeviceWithLibReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
	cmdGet8bitVal(cmd, &DeviceType);
	cmdGet16bitVal(cmd, &DevcieNameId);
	cmdGet16bitVal(cmd, &DeviceModel);
   
    
	
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
    if(NULL != epInfo)
    {
//    	pepInfo = &(vepInfo->epInfo);
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
		log_debug("\n DeviceType: %d \nDevcieNameId: %d \nDeviceModel: %d\n",DeviceType,DevcieNameId,DeviceModel);
		//组包
		if((TableIndex = Create_Irc_Query_Package(DeviceType,DevcieNameId,DeviceModel,&data)) >= 0)
		{
			SRPC_QueryRCCtrlCfm(epInfo->IEEEAddr,epInfo->endpoint,DeviceType,DevcieNameId,DeviceModel,TableIndex);
			zbSoc_IrcRemoteDevCtrlCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,data.idx,data.data);
		}
    }
#if 0
	if(epInfo!=NULL)
	{
		
		if(epInfo->onlineflag == true)
		{
			SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_RemoteIRCCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
		log_debug("\n DeviceType: %d \nDevcieNameId: %d \nDeviceModel: %d\n",DeviceType,DevcieNameId,DeviceModel);
		//组包
		if((TableIndex = Create_Irc_Query_Package(DeviceType,DevcieNameId,DeviceModel,&data)) >= 0)
		{
			SRPC_QueryRCCtrlCfm(epInfo->IEEEAddr,endPoint,DeviceType,DevcieNameId,DeviceModel,TableIndex);
			zbSoc_IrcRemoteDevCtrlCmd(epInfo->nwkAddr,endPoint,afAddr16Bit,data.idx,data.data);
		}
		else
		{
			SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
		}
	}
	else
	{
		SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}
#endif

    log_debug("SRPC_SendRemoteIRCCtrlReq--\n");
    return 0;
}

void SRPC_QueryIRCDeviceIDCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t deviceType,uint16_t deviceNameID,uint16_t deviceNum)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_QUERY_DEVICE_ID_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,deviceType);
	cmdSet16bitVal(&cmd,deviceNameID);
	cmdSet16bitVal(&cmd,deviceNum);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static uint8_t SRPC_QueryIRCDeviceIDWithLibReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
	uint8_t DeviceType ;//产品类型
	uint16_t DevcieNameId;//设备名称ID 
	uint16_t DeviceNumber = 0;
	
//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
    //pepInfo = &(vepInfo->epInfo);
    
    log_debug("SRPC_QueryIRCDeviceWithLibReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
	cmdGet8bitVal(cmd, &DeviceType);
	cmdGet16bitVal(cmd, &DevcieNameId);
   
    

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

    
    if(epInfo != NULL)
    {
//    	epInfo = &(vepInfo->epInfo);
		DeviceNumber =  Create_Irc_Query_Device_Number_Package(DeviceType,DevcieNameId);
		SRPC_QueryIRCDeviceIDCfm(epInfo->IEEEAddr,epInfo->endpoint,DeviceType,DevcieNameId,DeviceNumber);
    }

    log_debug("SRPC_SendRemoteIRCCtrlReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_SendIRCDeviceWithLibReq(hostCmd *cmd)
* 描述   :   控制红外设备
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_SendIRCDeviceWithLibReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
	uint8_t DeviceType;
	uint8_t DeviceKey[255]={0};
	uint8_t DeviceKeyLength;
	uint16_t TableIndex = 0;

//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
    //pepInfo = &(vepInfo->epInfo);
    
    hostCmd data;
	uint8_t count = 0;
    log_debug("SRPC_SendIRCDeviceWithLibReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
	cmdGet8bitVal(cmd, &DeviceType);
	cmdGet16bitVal(cmd,&TableIndex);
   	cmdGet8bitVal(cmd, &DeviceKeyLength);
	cmdGetStringVal(cmd,DeviceKey,DeviceKeyLength);

	log_debug("DeviceType = %d\n",DeviceType);
	log_debug("TableIndex = %d\n",TableIndex);
	log_debug("DeviceKeyLength = %d\n",DeviceKeyLength);

#ifndef NDEBUG
	for(count = 0;count<DeviceKeyLength;count++)
		log_debug("DeviceKey = %d\n",DeviceKey[count]);
#endif

    

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);
		
		if(epInfo->onlineflag == true)
		{
			SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_RemoteIRCCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
	
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		
		//
		Create_Irc_Ctrl_Package(DeviceType,TableIndex,DeviceKeyLength,DeviceKey,&data);
		zbSoc_IrcRemoteDevCtrlCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,data.idx,data.data);
	}
	else
	{
		SRPC_RemoteIRCCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_SendRemoteIRCCtrlReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_WinCurtainCtrlReq(hostCmd *cmd)
* 描述   :   窗帘控制器节点控制
* 输入   :
* 输出   :   无
* 返回   :   0:处理成功
************************************************************************/
static uint8_t SRPC_WinCurtainCtrlReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,ctrlCmd;
//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;

    log_debug("SRPC_WinCurtainCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &ctrlCmd);

    
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			SRPC_WinCurtainCtrlCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_WinCurtainCtrlCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		//将命令添加到重发机制列表中
		zblist_add(epInfo,&ctrlCmd,0x01);
		
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
		zbSoc_CurtainDevCtrlCmdReq(ctrlCmd,epInfo->nwkAddr,endPoint,afAddr16Bit);
	}
	else
	{
		SRPC_WinCurtainCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_WinCurtainCtrlReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_WinCurtainPercentageReq(hostCmd *cmd)
* 描述   :   控制窗帘运动到百分比的位置 0%-100% 关-开
* 输入   :
* 输出   :   无
* 返回   :   0:处理成功
************************************************************************/
static uint8_t SRPC_WinCurtainPercentageReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,percentage;
   
//   	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
    //pepInfo = &(vepInfo->epInfo);

    log_debug("SRPC_WinCurtainPercentageReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &percentage);//???

    
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			SRPC_WinCurtainPercentageCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_WinCurtainPercentageCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		//??б
		zblist_add(epInfo,&percentage,0x01);
		
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,endPoint);
		zbSoc_CurtainDevPercentageCmdReq(percentage,epInfo->nwkAddr,endPoint,afAddr16Bit);
	}
	else
	{
		SRPC_WinCurtainPercentageCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_WinCurtainPercentageReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_HumitureLightCtrlReq(hostCmd *cmd)
* 描述   :   温湿度光照节点控制
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
//static uint8_t SRPC_HumitureLightCtrlReq(hostCmd *cmd)
//{
//    uint8_t ieeeAddr[8];
//    uint8_t endPoint;
//    uint16_t timeInterval;

//    vepInfo_t *vepInfo = NULL;
//	epInfo_t *epInfo = NULL;
//	//pepInfo = &(vepInfo->epInfo);

//    log_debug("SRPC_HumitureLightCtrl++\n");

//    cmdGetStringVal(cmd, &ieeeAddr[0],8);
//    cmdGet8bitVal(cmd, &endPoint);
//    cmdGet16bitVal(cmd, &timeInterval);

//    
//    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

//	if(epInfo == NULL)
//	{
//		 SRPC_HumitureLightCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
//		 return 1;
//	}

//	pepInfo = &(vepInfo->epInfo);

//    
//    if(epInfo->registerflag != true)
//    {
//        SRPC_HumitureLightCtrlCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
//        return 0;
//    }

//   
//    if(epInfo->onlineflag != true)
//    {
//        SRPC_HumitureLightCtrlCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
//        return 0;
//    }

//    log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
//    zbSoc_SetTempIntervalReportReq(timeInterval,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
//    SRPC_HumitureLightCtrlCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);


//    log_debug("SRPC_HumitureLightCtrl--\n");
//    return 0;
//}
/************************************************************************
* 函数名 :SRPC_QueryHumitureLightReq(hostCmd *cmd)
* 描述   :   查询温湿度光照节点
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
/*
static uint8_t SRPC_QueryHumitureLightReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endpoint;
    uint16_t tempVal;
    uint16_t humVal;
    uint16_t lightVal;
    
//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);

    log_debug("SRPC_QueryHumitureLightReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endpoint);//???

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endpoint);

	if(epInfo == NULL)
	{
		SRPC_QueryHumitureLightStateCfm(ieeeAddr,endpoint,YY_STATUS_NODE_NO_EXIST,0xffff,0xffff,0xffff);
		return 1;
	}

//	pepInfo = &(vepInfo->epInfo);

  	
   
    if(epInfo->onlineflag != true)
    {
        SRPC_QueryHumitureLightStateCfm(ieeeAddr,endpoint,YY_STATUS_OUTLINE,0xffff,0xffff,0xffff);
        return 0;
    }

    if(devState_getHumitureLightVal(ieeeAddr,endpoint,&tempVal,&humVal,&lightVal))
    {
        SRPC_QueryHumitureLightStateCfm(ieeeAddr,endpoint,YY_STATUS_SUCCESS,tempVal,humVal,lightVal);
    }
    else
    {
        SRPC_QueryHumitureLightStateCfm(ieeeAddr,endpoint,YY_STATUS_FAIL,0xffff,0xffff,0xffff);
    }

    log_debug("SRPC_QueryHumitureLightReq--\n");

    return 0;
}
*/


/************************************************************************
* 函数名 :SRPC_HumitureValidCtrlReq(hostCmd *cmd)
* 描述   :  温湿度节点是否启用的节点控制(启用或禁用)
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_HumitureValidCtrlReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t useFlag;
    uint8_t endPoint;
    epInfo_t *epInfo;

    log_debug("SRPC_HumitureValidCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &useFlag);

    if(useFlag>1)
    {
        SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_ILLEGAL_PARAM);
        return 0;
    }

    
    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

    
    if((epInfo != NULL)&&(epInfo->registerflag != true))
    {
        SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
        return 0;
    }

   
    if((epInfo != NULL)&&(epInfo->onlineflag != true))
    {
        SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
        return 0;
    }

    if (epInfo != NULL)
    {
        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
        zbSoc_SetDevValidReq(useFlag,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
        SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
    }
    else
    {
        SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
    }

    log_debug("SRPC_HumitureValidCtrlReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_HumitureIntervalCtrlReq(hostCmd *cmd)
* 描述   :   温湿度节点上报间隔时间控制
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
//static uint8_t SRPC_HumitureIntervalCtrlReq(hostCmd *cmd)
//{
//    uint8_t ieeeAddr[8];
//    uint16_t timeInterval;
//    uint8_t endPoint;
//    epInfo_t *epInfo;

//    log_debug("SRPC_HumitureIntervalCtrlReq++\n");

//    cmdGetStringVal(cmd, &ieeeAddr[0],8);
//    cmdGet8bitVal(cmd, &endPoint);
//    cmdGet16bitVal(cmd, &timeInterval);

//    
//    log_debug("SRPC_HumitureIntervalCtrlReq Success.\n");

//    epInfo = devListGetDeviceByIeeeEp(ieeeAddr,endPoint);

//    
//    if((epInfo != NULL)&&(epInfo->registerflag != true))
//    {
//        SRPC_HumitureIntervalCtrlCfm(ieeeAddr,endPoint,YY_STATUS_UNREGISTER);
//        return 0;
//    }

//   
//    if((epInfo != NULL)&&(epInfo->onlineflag != true))
//    {
//        SRPC_HumitureIntervalCtrlCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
//        return 0;
//    }

//    if (epInfo != NULL)
//    {
//        log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
//        zbSoc_SetTempIntervalReportReq(timeInterval,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
//        SRPC_HumitureIntervalCtrlCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
//    }
//    else
//    {
//        SRPC_HumitureIntervalCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
//    }

//    log_debug("SRPC_HumitureIntervalCtrlReq--\n");
//    return 0;
//}

/************************************************************************
* 函数名 :SRPC_QueryHumitureReq(hostCmd *cmd)
* 描述   :   查询温湿度节点
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_QueryHumitureReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    uint16_t tempVal;
    uint16_t humVal;
    
//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);

    log_debug("SRPC_QueryHumitureReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);//???

    log_debug("SRPC_QueryHumitureReq++,endpoint=%02x\n",endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			SRPC_QueryHumitureStateCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_QueryHumitureStateCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
        //zbSoc_QueryDoorLockPowerValueState(epInfo->nwkAddr,endPoint,afAddr16Bit);
        zbSoc_getHumitureState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
    }
	else
	{
		SRPC_QueryHumitureStateCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_QueryHumitureReq--\n");

    return 0;
}

/************************************************************************
* 函数名 :SRPC_ComAlarmCtrlReq(hostCmd *cmd)
* 描述   :  通用报警器节点控制(启用或禁用)
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_ComAlarmCtrlReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t useFlag;
    uint8_t endPoint;
    
//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);
    
    log_debug("SRPC_ComAlarmCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &useFlag);

    if(useFlag > 1)
    {
        SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_ILLEGAL_PARAM);
        return 0 ;
    }
	
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo !=NULL)
	{
		if(epInfo->onlineflag == true)
		{
    		SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		zblist_add(epInfo,&useFlag,0x01);
		
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
   	 	zbSoc_SetDevValidReq(useFlag,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
	}
	else
	{
		SRPC_ComAlarmCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}
	
    log_debug("SRPC_ComAlarmCtrlReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_ComAlarmGetPowerReq(hostCmd *cmd)
* 描述   : 传感器电量查询
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_ComAlarmGetPowerReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    
	//vepInfo_t *vepInfo = NULL;
	
	  // epInfo_t *pepInfo = NULL;
	   //pepInfo = &(vepInfo->epInfo);
	epInfo_t *epinfo = NULL;

    log_debug("SRPC_ComAlarmGetPowerReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
  
    //获取设设备信息
    epinfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epinfo !=NULL)
	{
		if(epinfo->onlineflag == true)
		{
			SRPC_ComAlarmGetPowerCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_ComAlarmGetPowerCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		if((epinfo->deviceID == ZB_DEV_DOOR_SENSOR)||(epinfo->deviceID == ZB_DEV_INFRARED_BODY_SENSOR))
		{

			if(epinfo->onlineflag == true)
			{
				uint8_t Battery = vdevListGetAlarmBattery(epinfo);
				//if(vepInfo->dataSegment[1]!=0)
				if (Battery != 0)
				{
					SRPC_ComAlarmPowerValueInd(epinfo->IEEEAddr,epinfo->endpoint,epinfo->deviceID,Battery);
				}
			}
			else
			{
				SRPC_ComAlarmPowerValueInd(epinfo->IEEEAddr,epinfo->endpoint,epinfo->deviceID,0);
			}
		}
		else
		{
			SRPC_ComAlarmPowerValueInd(epinfo->IEEEAddr,epinfo->endpoint,epinfo->deviceID,0);
		}
	}
	else
	{
		SRPC_ComAlarmGetPowerCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

	
    log_debug("SRPC_ComAlarmGetPowerReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_DoorLockCtrlReq(hostCmd *cmd)
* 描述   :   门锁开关节点控制
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_DoorLockCtrlReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,switchCmd;
    
	epInfo_t *epInfo = NULL;

    log_debug("SRPC_DoorLockCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &switchCmd);
    
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
    
	if(epInfo != NULL)
	{
		if(epInfo->onlineflag == true)
		{
			SRPC_DoorLockCtrlCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_DoorLockCtrlCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}

		zblist_add(epInfo,&switchCmd,0x01);
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);

		switch(epInfo->deviceID)
		{
		
#if DEVICE_LIWEI_DOOR_SUPPERT
			//Level 门锁
			case ZB_DEV_LEVEL_DOORLOCK:
				doorLevel_setOnOffReq(epInfo,switchCmd);
			break;
#endif
		
			//普通门锁
			case ZB_DEV_ONOFF_DOORLOCK:
				zbSoc_SetGenOnOffState(switchCmd,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
			break;
		}
    }
	else
	{
		SRPC_DoorLockCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_DoorLockCtrlReq--\n");
    return 0;
}

/************************************************************************
* 函数名 :SRPC_QueryDoorLockPowerValueReq(hostCmd *cmd)
* 描述   :   查询门锁电量值请求
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_QueryDoorLockPowerValueReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
    
//	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);

    log_debug("SRPC_QueryDoorLockPowerValueReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);
		if(epInfo->deviceID != ZB_DEV_ONOFF_DOORLOCK)
		{
			SRPC_QueryPowerValueCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return 1;
		}
	
		if(epInfo->onlineflag == true)
		{
			SRPC_QueryPowerValueCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_QueryPowerValueCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("DoorLock nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
        zbSoc_QueryDoorLockPowerValueState(epInfo->nwkAddr,endPoint,afAddr16Bit);
    }
	else
	{
		SRPC_QueryPowerValueCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_QueryDoorLockPowerValueReq--\n");
    return 0;
}

void SRPC_QueryDoorLockSpeedCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint16_t mSpeed)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_DOORLOCK_SPEED_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);

	if(status == YY_STATUS_SUCCESS)
	{
		 cmdSet16bitVal(&cmd,mSpeed);
	}
	
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static uint8_t SRPC_QueryDoorLockSpeedValueReq(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
    uint8_t endPoint;
    
//	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);

    log_debug("SRPC_QueryDoorLockSpeedValueReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);
		
		if(epInfo->deviceID != ZB_DEV_ONOFF_DOORLOCK)
		{
			SRPC_QueryDoorLockSpeedCfm(ieeeAddr,endPoint,YY_STATUS_FAIL,0);
			return 1;
		}
		
		if(epInfo->onlineflag != true)
		{
			SRPC_QueryDoorLockSpeedCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE,0);
		}
		
		log_debug("DoorLock nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
        zbSoc_QueryDoorLockSpeedValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
    }
	else
	{
		SRPC_QueryDoorLockSpeedCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST,0);
	}

    log_debug("SRPC_QueryDoorLockSpeedValueReq--\n");
    return 0;
}


void SRPC_SetDoorLockSpeedCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint16_t mSpeed)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SET_DOORLOCK_SPEED_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);

	if(status == YY_STATUS_SUCCESS)
	{
		 cmdSet16bitVal(&cmd,mSpeed);
	}
	
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static uint8_t SRPC_SetDoorLockSpeedValueReq(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
    uint8_t endPoint;
	uint16_t mTimes = 0;
    
//	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);
    
    log_debug("SRPC_QueryDoorLockSpeedValueReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
	cmdGet16bitVal(cmd,&mTimes);
	
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);

		if(epInfo->deviceID != ZB_DEV_ONOFF_DOORLOCK)
		{
			SRPC_SetDoorLockSpeedCfm(ieeeAddr,endPoint,YY_STATUS_FAIL,0);
			return 1;
		}
		
		if(epInfo->onlineflag != true)
		{
			SRPC_SetDoorLockSpeedCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE,0);
		}
		
		log_debug("DoorLock nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
        zbSoc_SetDoorLockSpeedValueState(epInfo->nwkAddr,epInfo->endpoint,mTimes,afAddr16Bit);
    }
	else
	{
		SRPC_SetDoorLockSpeedCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST,0);
	}

    log_debug("SRPC_QueryDoorLockSpeedValueReq--\n");
    return 0;
}


void SRPC_GetLevelDoorLockInfoCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_LEVEL_DOOR_INFO_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static uint8_t SRPC_GetLevelDoorLockInfoReq(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
    uint8_t endPoint;

//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);

    log_debug("SRPC_GetLevelDoorLockInfoReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
	
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);

		if(epInfo->deviceID != ZB_DEV_LEVEL_DOORLOCK)
		{
			
			SRPC_GetLevelDoorLockInfoCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return 1;
		}
		
		if(epInfo->onlineflag != true)
		{
			SRPC_GetLevelDoorLockInfoCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("DoorLock nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		doorLevel_getInfoReq(epInfo);
        //zbSoc_SetDoorLockSpeedValueState(epInfo->nwkAddr,endPoint,mTimes,afAddr16Bit);
    }
	else
	{
		SRPC_GetLevelDoorLockInfoCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_GetLevelDoorLockInfoReq--\n");

    return 0;
}

void SRPC_SetLevelDoorLockTimeCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SET_LEVEL_DOOR_TIME_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static uint8_t SRPC_SetLevelDoorLockTimeReq(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
    uint8_t endPoint;

	uint8_t year = 0;
	uint8_t month = 0;
	uint8_t day = 0;
	uint8_t hours =0 ;
	uint8_t min = 0;
	uint8_t sec = 0;
	uint8_t wday = 0;
    
//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);
	
    log_debug("SRPC_SetLevelDoorLockTimeReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

	cmdGet8bitVal(cmd, &year);
	cmdGet8bitVal(cmd, &month);
	cmdGet8bitVal(cmd, &day);
	cmdGet8bitVal(cmd, &hours);
	cmdGet8bitVal(cmd, &min);
	cmdGet8bitVal(cmd, &sec);
	cmdGet8bitVal(cmd, &wday);
	
    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
//		epInfo = &(vepInfo->epInfo);
		
		if(epInfo->deviceID != ZB_DEV_LEVEL_DOORLOCK)
		{
			
			SRPC_SetLevelDoorLockTimeCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return 1;
		}
		
		if(epInfo->onlineflag != true)
		{
			SRPC_SetLevelDoorLockTimeCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("DoorLock nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		
		doorLevel_settimeReq(epInfo,year,month,day,hours,min,sec,wday);
    }
	else
	{
		SRPC_SetLevelDoorLockTimeCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_SetLevelDoorLockTimeReq--\n");

    return 0;
}


void SRPC_SetLevelDoorLockRegCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_LEVEL_DOOR_REG_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


static uint8_t SRPC_GetLevelDoorLockRegReq(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
	uint8_t endPoint;
	uint8_t regType = 0;
	
//	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);

	log_debug("SRPC_GetLevelDoorLockRegReq++\n");

	cmdGetStringVal(cmd, &ieeeAddr[0],8);
	cmdGet8bitVal(cmd, &endPoint);

	cmdGet8bitVal(cmd, &regType);
	
	epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{
//		epInfo = &(epInfo->epInfo);
		
		if(epInfo->deviceID != ZB_DEV_LEVEL_DOORLOCK)
		{
			
			SRPC_SetLevelDoorLockRegCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return 1;
		}
		
		if(epInfo->onlineflag != true)
		{
			SRPC_SetLevelDoorLockRegCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("DoorLock nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		
		doorLevel_getRegisterInfoReq(epInfo,regType);
	}
	else
	{
		SRPC_SetLevelDoorLockRegCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

	log_debug("SRPC_GetLevelDoorLockRegReq--\n");

	return 0;
}

void SRPC_LevelDoorLockUsrCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ADD_LEVEL_DOOR_USR_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static uint8_t SRPC_LevelDoorLockUsrReq(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
	uint8_t endPoint;
	uint8_t usrType = 0;
	uint8_t optType = 0;
	uint16_t usrid = 0;
	uint8_t passwd[8] = {0};

//	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
//	pepInfo = &(vepInfo->epInfo);

	log_debug("SRPC_LevelDoorLockUsrReq++\n");

	cmdGetStringVal(cmd, &ieeeAddr[0],8);
	cmdGet8bitVal(cmd, &endPoint);

	cmdGet8bitVal(cmd, &usrType);
	cmdGet8bitVal(cmd, &optType);
	cmdGet16bitVal(cmd, &usrid);
	cmdGetStringVal(cmd,passwd,8);
	
	epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo!=NULL)
	{	
	
//		pepInfo = &(vepInfo->epInfo);

		if(epInfo->deviceID != ZB_DEV_LEVEL_DOORLOCK)
		{
			SRPC_LevelDoorLockUsrCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return 1;
		}
		
		if(epInfo->onlineflag != true)
		{
			SRPC_LevelDoorLockUsrCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("DoorLock nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		
		doorLevel_usrMngReq(epInfo,usrType,optType,usrid,passwd);
	}
	else
	{
		SRPC_LevelDoorLockUsrCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

	log_debug("SRPC_LevelDoorLockUsrReq--\n");

	return 0;
}

#if DEVICE_LIWEI_DOOR_OPEN_CNT

void SRPC_LevelDoorLockOpenCntCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_LEVEL_DOOR_OPEN_CNT_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_LevelDoorLockOpenCntInd(uint8_t* ieeeAddr,uint8_t endpoint,uint32_t value)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_LEVEL_DOOR_OPEN_CNT_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet32bitVal(&cmd,value);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


static uint8_t SRPC_LevelDoorLockOpenCntReq(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
	uint8_t endPoint;
	
	epInfo_t *epInfo = NULL;

	log_debug("SRPC_LevelDoorLockOpenCntReq++\n");

	cmdGetStringVal(cmd, &ieeeAddr[0],8);
	cmdGet8bitVal(cmd, &endPoint);

	epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);

	if(epInfo != NULL)
	{	
		if(epInfo->deviceID != ZB_DEV_LEVEL_DOORLOCK)
		{
			SRPC_LevelDoorLockOpenCntCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
			return 1;
		}
		
		if(epInfo->onlineflag != true)
		{
			SRPC_LevelDoorLockOpenCntCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("DoorLock nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);

		SRPC_LevelDoorLockOpenCntCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		
		doorLevel_getOpenCntReq(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		
	}
	else
	{
		SRPC_LevelDoorLockOpenCntCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

	log_debug("SRPC_LevelDoorLockOpenCntReq--\n");

	return 0;
}

#endif
/************************************************************************
* 函数名 :SRPC_PowerSwitchCtrlReq(hostCmd *cmd)
* 描述   :   取电开关节点控制
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_PowerSwitchCtrlReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint,switchCmd;
    
//	vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);
    
    log_debug("SRPC_PowerSwitchCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);
    cmdGet8bitVal(cmd, &switchCmd);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
    
	if(epInfo!=NULL)
	{
//		pepInfo = &(vepInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			SRPC_PowerSwitchCtrlCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_PowerSwitchCtrlCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);

		 //将命令添加到重发机制列表中
		zblist_add(epInfo,&switchCmd,0x01);
		
#if USE_MASTER_CONTROL
		zbSoc_MasterControlSetOnOffState(epInfo,switchCmd);
#else
		zbSoc_SetGenOnOffState(switchCmd,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
#endif
		
    }
	else
	{
		SRPC_PowerSwitchCtrlCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}
	
    log_debug("SRPC_PowerSwitchCtrlReq--\n");
    return 0;
}

/***************************************************************************************************
 * @fn      SRPC_PowerSwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_QueryPowerSwitchValueCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_POWERSWITCH_POWER_VALUE_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_QueryPowerSwitchValueReq(hostCmd *cmd)
* 描述   :   智能取电开关电量查询
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_QueryPowerSwitchValueReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;
	uint16_t nwkAddr; 

//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);
    
    log_debug("SRPC_PowerSwitchCtrlReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if(epInfo!=NULL)
	{
//		pepInfo = &(epInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			SRPC_QueryPowerSwitchValueCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_QueryPowerSwitchValueCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		

		log_debug("nwkAddr %04x,endpoint %x.\n",epInfo->nwkAddr,epInfo->endpoint);
		nwkAddr  = epInfo->nwkAddr; 
		
#if USE_MASTER_CONTROL
		if(zbSoc_MasterControlCheck(ieeeAddr,endPoint)==true)//豸
		{
//			SRPC_QueryPowerSwitchValueCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);
		}
		else
		{
			zbSoc_QueryPowerSwitchValueState(nwkAddr,endPoint,afAddr16Bit);
//			SRPC_QueryPowerSwitchValueCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
#else
		zbSoc_QueryPowerSwitchValueState(nwkAddr,endPoint,afAddr16Bit);
#endif

    }
	else
	{
		SRPC_QueryPowerSwitchValueCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_PowerSwitchCtrlReq--\n");
    return 0;
}

/***************************************************************************************************
 * @fn      SRPC_DoorLockPowerValueInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_QueryPowerSwitchValueInd(uint8_t* ieeeAddr,uint8_t endpoint,uint32_t status,uint8_t rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_POWERSWITCH_POWER_VALUE_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet32bitVal(&cmd,status);
    //D12 
    cmdSet8bitVal(&cmd,rssi);
	
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_PowerSwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_QueryPowerSwitchStateCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_POWERSWITCH_STATUS_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_QueryPowerSwitchValueReq(hostCmd *cmd)
* 描述   :   智能取电开关电量查询
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static uint8_t SRPC_QueryPowerSwitchStateReq(hostCmd *cmd)
{
    uint8_t ieeeAddr[8];
    uint8_t endPoint;

//    vepInfo_t *vepInfo = NULL;
	epInfo_t *epInfo = NULL;
	//pepInfo = &(vepInfo->epInfo);

    log_debug("SRPC_QueryPowerSwitchStateReq++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if(epInfo!=NULL)
	{
//		epInfo = &(vepInfo->epInfo);
		if(epInfo->onlineflag == true)
		{
			SRPC_QueryPowerSwitchStateCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else
		{
			SRPC_QueryPowerSwitchStateCfm( ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
	
#if USE_MASTER_CONTROL
		zbSoc_MasterControlQueryOnOffState(epInfo);
#else
		zbSoc_QueryLightValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
#endif

    }
	else
	{
		SRPC_QueryPowerSwitchStateCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

    log_debug("SRPC_QueryPowerSwitchStateReq--\n");
    return 0;
}


/************************************************************************
* 函数名 :SRPC_ConfigSsidPasswdCfm(uint8 status)
* 描述   :   配置路由器的ssid和密码命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_ConfigSsidPasswdCfm(uint8_t status)
{
	log_debug("SRPC_ConfigSsidPasswdCfm++\n");
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_CONFIG_SSID_PASSWD_CMD_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_ConfigEthernetCmdCfm(uint8 status)
* 描述   :   配置路由器的ssid和密码命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_ConfigEthernetCmdCfm(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_CONFIG_LAN_ETHERNET_CMD_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_ConfigApSSidCmdCfm(uint8 status)
* 描述   :   配置路由器热点命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_ConfigApSSidCmdCfm(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_CONFIG_AP_SSID_CMD_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_GetMacAddrCfm(uint8 status)
* 描述   :   获取Mac地址应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_GetMacAddrCfm(uint8_t* macAddr)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_GET_MAC_ADDR_CMD_CFM);
    //D2 Mac Addr
    cmdSetStringVal(&cmd,macAddr,6);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_GetDevAddrCfm(uint8 status)
* 描述   :   获取Dev地址应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_GetDevAddrCfm(uint8_t* devAddr)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_GET_COORD_VERSION_CMD_CFM);
    //D2 Mac Addr
    cmdSetStringVal(&cmd,devAddr,6);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_OpenNwkCfm(uint8 status)
* 描述   :   开放网络命令
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_OpenNwkCfm(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_OPEN_NETWORK_CMD_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_RevertFactorySettingCfm(uint8 status)
* 描述   :   恢复协调器出厂设置命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_RevertFactorySettingCfm(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_REVERT_ALL_DEV_FACTORY_SETTING_CMD_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_GatewayFactoryInd(void)
* 描述   :   网关恢复出厂设置信息上报
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_GatewayFactoryInd(void)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_GATEWAY_FACTORY_IND);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/************************************************************************
* 函数名 :SRPC_RevertOneFactorySettingCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
* 描述   :   恢复单个设备至出厂设置命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_RevertOneFactorySettingCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_REVERT_ONE_DEV_FACTORY_SETTING_CMD_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_ComDevRegisterInd
 *
 * @brief   发送zigbee节点通用的注册信息到服务器

 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_ComDevRegisterInd(uint8_t* ieeeAddr,uint16_t deviceid,uint8_t portVal)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_DEV_REGISTER_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
    cmdSet8bitVal(&cmd,portVal);
    cmdSet16bitVal(&cmd,deviceid);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :SRPC_heartPacketInd(uint8 devCount)
* 描述   :   设备心跳包消息上报
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void SRPC_heartPacketInd(uint8_t devCount,uint8_t* pBuf,uint16_t pBufLen)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ONLINE_DEVICE_HEART_IND);
    //D2 devCount
    cmdSet8bitVal(&cmd,devCount);
    //еIEEE ?,IEEE len + Endpoint len=8+1=9
    cmdSetStringVal(&cmd,pBuf,pBufLen);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_QueryAllUnregisterDevCfm
 *
 * @brief   发送查询到的所有未注册的节点应答

 * @param
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_QueryAllUnregisterDevCfm(uint8_t devCount,uint8_t* buf,uint16_t bufLen)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_QUERY_ALL_UNREGISTER_DEV_CFM);
    //D2 devCount
    cmdSet8bitVal(&cmd,devCount);
    //发送所有的尚未注册的节点
    cmdSetStringVal(&cmd,buf,bufLen);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_PeriodTimerSwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_PeriodTimerSwitchCtrlCfm(uint8_t status,uint8_t taskId)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_PERIOD_TIMER_SWITCH_SET_CFM);
    //D2  ID 
    cmdSet8bitVal(&cmd,taskId);
    //D3 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_DeletePeriodTimerSwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_DeletePeriodTimerSwitchCtrlCfm(uint8_t status,uint8_t timeTaskId)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DELETE_PERIOD_TIMER_SWITCH_CFM);
	cmdSet8bitVal(&cmd,timeTaskId);
    //D2 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_CtlPeriodTimerSwitchCtrlCfm(uint8_t status,uint8_t timeTaskId)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_CTL_PERIOD_TIMER_SWITCH_CFM);
	cmdSet8bitVal(&cmd,timeTaskId);
    //D2 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_AddDevGroupCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_AddDevGroupCfm(uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ADD_DEV_GROUP_CFM);
    //D2 
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_SwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_SwitchCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SWITCH_CTRL_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


void SRPC_SwitchQueryCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_SWITCH_STATUS_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_SwitchQueryValueCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_SIWTCH_VALUE_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_SwitchStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_SwitchStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint8_t rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SWITCH_STATUS_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
    cmdSet8bitVal(&cmd,endpoint);
    cmdSet8bitVal(&cmd,status);
    //D12 
    cmdSet8bitVal(&cmd,rssi);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}



/***************************************************************************************************
 * @fn      SRPC_WinCurtainCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_WinCurtainCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_WIN_CURTAIN_CTRL_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_WinCurtainStatusInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_WIN_CURTAIN_STATUS_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_WinCurtainPercentageCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_WIN_CURTAIN_PERCENTAGE_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_WinCurtainPercentageStatusInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_WIN_CURTAIN_PERCENTAGE_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}



/***************************************************************************************************
 * @fn      SRPC_HumitureLightCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
//void SRPC_HumitureLightCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
//{
//    hostCmd cmd;
//    cmd.idx = 0;
//    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
//    //D0 D1 Opcode
//    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_HUMITURE_LIGHT_CTRL_CFM);
//    //D2-D9 IEEE
//    cmdSetStringVal(&cmd,ieeeAddr,8);
//   
//    cmdSet8bitVal(&cmd,endpoint);
//   
//    cmdSet8bitVal(&cmd,status);
//    makeMsgEnder(&cmd);
//    cmdMsgSend(cmd.data,cmd.idx);
//}


/***************************************************************************************************
 * @fn      SRPC_HumitureLightStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
/*
void SRPC_HumitureLightStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t temp,uint16_t hum,uint16_t ilum)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_HUMITURE_LIGHT_STATUS_IND);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
   ?
    cmdSet8bitVal(&cmd,endpoint);
    //D11-D12 
    cmdSet16bitVal(&cmd,temp);
    //D13-D14 
    cmdSet16bitVal(&cmd,hum);
    //D15-D16 
    cmdSet16bitVal(&cmd,ilum);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}
*/
/***************************************************************************************************
 * @fn      SRPC_QueryHumitureLightStateCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
//void SRPC_QueryHumitureLightStateCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint16_t temp,uint16_t hum,uint16_t light)
//{
//    hostCmd cmd;
//    cmd.idx = 0;
//    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
//    //D0 D1 Opcode
//    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_HUMITURE_LIGHT_STATUS_CFM);
//    //D2-D9 IEEE
//    cmdSetStringVal(&cmd,ieeeAddr,8);
//    //D10
//    cmdSet8bitVal(&cmd,endpoint);
//    //D11-D12 
//    cmdSet16bitVal(&cmd,temp);
//    //D13-D14 
//    cmdSet16bitVal(&cmd,hum);
//    //D15-D16
//    cmdSet16bitVal(&cmd,light);
//    makeMsgEnder(&cmd);
//    cmdMsgSend(cmd.data,cmd.idx);
//}

/***************************************************************************************************
 * @fn      SRPC_HumitureIntervalCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
//void SRPC_HumitureIntervalCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
//{
//    hostCmd cmd;
//    cmd.idx = 0;
//    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
//    //D0 D1 Opcode
//    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_HUMITURE_CTRL_CFM);
//    //D2-D9 IEEE?
//    cmdSetStringVal(&cmd,ieeeAddr,8);
//   
//    cmdSet8bitVal(&cmd,endpoint);
//   
//    cmdSet8bitVal(&cmd,status);
//    makeMsgEnder(&cmd);
//    cmdMsgSend(cmd.data,cmd.idx);
//}

/***************************************************************************************************
 * @fn      SRPC_HumitureStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_HumitureStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t data)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_HUMITURE_STATUS_IND);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
    //D11-D12 
    cmdSet16bitVal(&cmd,data);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_QueryHumitureStateCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_QueryHumitureStateCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_HUMITURE_STATUS_CFM);
    //D2-D9 IEEE?
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10
    cmdSet8bitVal(&cmd,endpoint);
    //D11
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_EnvironmentTempHumStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_EnvironmentTempHumStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t temp,uint16_t hum)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ENVIRONMENT_TEMPHUM_STATUS_IND);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
    //D11-D12 温度值
    cmdSet16bitVal(&cmd,temp);
    //D13-D14 湿度值
    cmdSet16bitVal(&cmd,hum);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_EnvironmentLightStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_EnvironmentLightStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t lightState)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ENVIRONMENT_LIGHT_STATUS_IND);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,lightState);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_EnvironmentPM25StateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_EnvironmentPM25StateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t pmState)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ENVIRONMENT_PM25_STATUS_IND);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
    //D11 PM2.5 
    cmdSet8bitVal(&cmd,pmState);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_EnvironmentNoiceStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_EnvironmentNoiceStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t noiceState)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ENVIRONMENT_NOICE_STATUS_IND);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,noiceState);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_ComAlarmCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_ComAlarmCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_ALARM_CTRL_CFM);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_ComAlarmCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_ComAlarmGetPowerCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_ALARM_GET_POWERS_CFM);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_ComAlarmStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_ComAlarmStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t deviceID,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_ALARM_STATUS_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet16bitVal(&cmd,deviceID);
  
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_ComAlarmPowerValueInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t deviceID,uint8_t value)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_ALARM_POWER_VALUE_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
    
    cmdSet16bitVal(&cmd,deviceID);
    
    cmdSet8bitVal(&cmd,value);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_ComAlarmDeviceStateInd(uint8_t* ieeeAddr,uint8_t endpoint,uint16_t deviceID,uint8_t value)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_ALARM_ENABLE_IND);
    //D2-D9 IEEE
    cmdSetStringVal(&cmd,ieeeAddr,8);
	//端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11~D12状态值
    cmdSet16bitVal(&cmd,deviceID);
    //D13状态值
    cmdSet8bitVal(&cmd,value);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      SRPC_DoorLockCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_DoorLockCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_CTRL_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_DoorLockCtrlInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_DoorLockCtrlInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint8_t rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_STATUS_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    //D12 ?
    cmdSet8bitVal(&cmd,rssi);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_DoorLockPowerValueInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_DoorLockPowerValueInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint8_t rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_POWER_VALUE_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    //D12 
    cmdSet8bitVal(&cmd,rssi);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_DoorLockPowerValueInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_DoorLockPowerValueResp(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_DOORLOCK_POWER_VALUE_RESP);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_QueryPowerValueCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_QueryPowerValueCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_DOORLOCK_POWER_VALUE_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_PowerSwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_PowerSwitchCtrlCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_POWERSWITCH_CTRL_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      SRPC_PowerSwitchCtrlInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void SRPC_PowerSwitchCtrlInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status,uint8_t rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_POWERSWITCH_STATUS_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    //D12 
    cmdSet8bitVal(&cmd,rssi);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/*********************************************************************
				中央空调控制器
**********************************************************************/
void SPRC_SetCentralAirReqCmdCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
	hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SET_CENTRAL_AIR_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
 
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SPRC_QueryCentralAirReqCmdCfm(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t status)
{
	 hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_CENTRAL_AIR_CFM);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);
   
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_CentralAirInd(uint8_t* ieeeAddr,uint8_t endpoint,uint8_t airLength,uint8_t *airData)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_CENTRAL_AIR_STATE_IND);
    
    cmdSetStringVal(&cmd,ieeeAddr,8);
   
    cmdSet8bitVal(&cmd,endpoint);

	
	cmdSetStringVal(&cmd,airData,airLength);
	
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/*****************************************************************************
 * 函 数 名  : SPRC_QueryCentralAirReqCmd
 * 负 责 人  : Edward
 * 创建日期  : 2016年3月28日
 * 函数功能  : 中央空调状态查询
 * 输入参数  : hostCmd *cmd  服务器传输过来的数据
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static int8_t SPRC_QueryCentralAirReqCmd(hostCmd *cmd)
{

#define CENTRAL_AIR_REAL_DATA_SIZE	9

	uint8_t ieeeAddr[8];
    uint8_t endPoint;
	int8_t ret = 0;
	epInfo_t *epInfo = NULL;
	
    log_debug("SPRC_QueryCentralAirReqCmd++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if (epInfo != NULL)
	{
		if(epInfo->onlineflag == true)	
		{
			uint8_t value[CENTRAL_AIR_REAL_DATA_SIZE] ={0};

			ret = vdevListGetCentralAirState(epInfo,value,CENTRAL_AIR_REAL_DATA_SIZE);
			
			if(ret > 0)
			{
				SPRC_QueryCentralAirReqCmdCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
				usleep(2000);
				//返回中央空调的当前状态
				SRPC_CentralAirInd(ieeeAddr,endPoint,CENTRAL_AIR_REAL_DATA_SIZE,value);
			}
			else
			{
				SPRC_QueryCentralAirReqCmdCfm(ieeeAddr,endPoint,YY_STATUS_FAIL);	
			}
		}
		else //设备不存在
		{
			SPRC_QueryCentralAirReqCmdCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
	}
	else //设备不存在
	{
		SPRC_QueryCentralAirReqCmdCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

	return 0;
}

/*****************************************************************************
 * 函 数 名  : SPRC_SetCentralAirReqCmd
 * 负 责 人  : Edward
 * 创建日期  : 2016年3月28日
 * 函数功能  : 中央空调控制
 * 输入参数  : hostCmd *cmd  服务器传输过来的数据
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static int8_t SPRC_SetCentralAirReqCmd(hostCmd *cmd)
{
	uint8_t ieeeAddr[8];
    uint8_t endPoint;

	epInfo_t *epInfo = NULL;

	uint8_t AirAttribute[9] = {0};
	
    log_debug("SPRC_SetCentralAirReqCmd++\n");

    cmdGetStringVal(cmd, &ieeeAddr[0],8);
    cmdGet8bitVal(cmd, &endPoint);

    epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,endPoint);
	
	if (epInfo != NULL)//设备存在
	{
		if(epInfo->onlineflag == true)	//设备在线
		{	
			SPRC_SetCentralAirReqCmdCfm(ieeeAddr,endPoint,YY_STATUS_SUCCESS);
		}
		else //设备不在线
		{
			SPRC_SetCentralAirReqCmdCfm(ieeeAddr,endPoint,YY_STATUS_OUTLINE);
		}
		
		cmdGetStringVal(cmd,AirAttribute,9);//获取数据
		//控制设备
		zbSoc_SetCentralAirCmdReq(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,9,AirAttribute);
	}
	else //设备不存在
	{
		SPRC_SetCentralAirReqCmdCfm(ieeeAddr,endPoint,YY_STATUS_NODE_NO_EXIST);
	}

	log_debug("SPRC_SetCentralAirReqCmd--\n");
	return 0;
}

/********************************************************************/

void SRPC_ActionEventCfm(uint16_t opcode,uint8_t eventid,uint8_t* ieeeAddr,uint8_t endpoint, uint8_t status)
{
	hostCmd cmd;
	cmd.idx = 0;
	makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
	//D0 D1 Opcode
	cmdSet16bitVal(&cmd,opcode);
	cmdSet8bitVal(&cmd,eventid);
	
	cmdSetStringVal(&cmd,ieeeAddr,8);
	//D10 
	cmdSet8bitVal(&cmd,endpoint);
	//D11 
	cmdSet8bitVal(&cmd,status);
	makeMsgEnder(&cmd);
	cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_ActionEventAddReq(hostCmd *cmd)
{
	uint8_t mLenght = 0;
	
	ActionEvent_t mAction;
	Event_t *pCondition = &mAction.Condition; //条件
	Event_t member[MAX_SUPPORTED_NODE_MEMBERS] = {0};
	Event_t ** nextMemberPtr;
	
	log_debug("SRPC_ActionEventAddReq++\n");

	memset(&mAction,0,sizeof(ActionEvent_t));
	nextMemberPtr =  &mAction.members;
	
	//Event ID
	cmdGet8bitVal(cmd,&mAction.ActionEventID);
	cmdGet8bitVal(cmd,&mAction.EventState);
	cmdGetStringVal(cmd,&pCondition->IEEEAddr[0],8);
    cmdGet8bitVal(cmd, &pCondition->endpoint);
	cmdGet8bitVal(cmd, &pCondition->length);//获取数据段长度
	cmdGetStringVal(cmd,&pCondition->dataSegment[0],pCondition->length);

	cmdGet8bitVal(cmd, &mAction.type);//事件类型
	
	log_debug("type = %d\n",mAction.type);
	
	if((mAction.type != EVENT_TYPE_SCENEID_ACTION) && (mAction.type != EVENT_TYPE_NODE_ACTION) )
	{
		log_debug("Event Format error\n");
		SRPC_ActionEventCfm(WIFI_ZIGBEE_ACTION_EVENT_ADD_CFM,mAction.ActionEventID,pCondition->IEEEAddr,pCondition->endpoint,YY_STATUS_FAIL);
		return;
	}
	else if(mAction.type == EVENT_TYPE_SCENEID_ACTION) //场景
	{
		cmdGet8bitVal(cmd, &mAction.sceneid);
	}
	else	//节点
	{
		//节点数量
		cmdGet8bitVal(cmd,&mAction.membersCount);

		//获取每个节点数据
		for (mLenght= 0;(mLenght < mAction.membersCount) && (mLenght < MAX_SUPPORTED_NODE_MEMBERS); mLenght++)
		{
			*nextMemberPtr = &(member[mLenght]);
			cmdGetStringVal(cmd,&member[mLenght].IEEEAddr[0],8);
			cmdGet8bitVal(cmd, &member[mLenght].endpoint);
			cmdGet8bitVal(cmd, &member[mLenght].length);//获取数据段长度
			cmdGetStringVal(cmd,&member[mLenght].dataSegment[0],member[mLenght].length);

			nextMemberPtr = &(member[mLenght].next);
		}
		
		*nextMemberPtr = NULL;
	}

	//增加事件
	if(mAction.ActionEventID == NEW_TIMER_EVENT)
	{
		event_addEvent(&mAction);
	}
	else //修改事件
	{
		event_ModifyRecordByEventID(&mAction);
	}
	log_debug("SRPC_ActionEventAddReq--\n");
}

void SRPC_ActionEventDelReq(hostCmd *cmd)
{
//	uint8_t mLenght = 0;
	Event_t node;
	uint8_t eventID;
	log_debug("SRPC_ActionEventDelReq++\n");
	//获取事件任务ID
	cmdGet8bitVal(cmd,&eventID);
	cmdGetStringVal(cmd,&node.IEEEAddr[0],8);
    cmdGet8bitVal(cmd, &node.endpoint);
	event_DeleteEvent(node.IEEEAddr,node.endpoint,eventID);
	log_debug("SRPC_ActionEventDelReq++\n");
}

void SRPC_ActionEventCtlReq(hostCmd *cmd)
{
//	uint8_t mLenght = 0;
	Event_t node;
	uint8_t eventID;
	uint8_t eventState;
	//获取事件任务ID
	cmdGet8bitVal(cmd,&eventID);
	cmdGet8bitVal(cmd,&eventState);
	cmdGetStringVal(cmd,&node.IEEEAddr[0],8);
    cmdGet8bitVal(cmd, &node.endpoint);
    //控制事件触发启动或者关闭
	event_ControlTask(node.IEEEAddr,node.endpoint,eventID,eventState);
}

void SRPC_ActionEventGetALLIND(uint8_t devCount,uint8_t* pBuf,uint16_t pBufLen)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ACTION_EVENT_STATE_IND);
    //D2 devCount
    cmdSet8bitVal(&cmd,devCount);
    //发送所有的IEEE 值,IEEE len + Endpoint len=8+1=9
    cmdSetStringVal(&cmd,pBuf,pBufLen);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

void SRPC_ActionEventGetAllReq(hostCmd *cmd)
{
	uint8_t rec[MAX_SUPPORTED_RECORD_SIZE]  ={0};
	uint8_t devNum = 0;
	uint16_t dataLength = 0;
	dataLength = eventListGetAllDevice(&devNum,rec,MAX_SUPPORTED_RECORD_SIZE);

	SRPC_ActionEventGetALLIND(devNum,rec,dataLength);
}

void SRPC_Mt_Network_Conflict_Ind(uint8_t mStatus)
{
	hostCmd cmd;
	cmd.idx = 0;
	
	makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,ZIGBEE_DEVICE_MT_NETWORK_CONFLICT_IND);
    //D2 状态
    cmdSet8bitVal(&cmd,mStatus);
 
	makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


void SRPC_Mt_Network_SerialState_Ind(void)
{
	hostCmd cmd;
	cmd.idx = 0;
	
	makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,ZIGBEE_DEVICE_MT_SERIAL_STATE_IND);
    //D2 状态
    //cmdSet8bitVal(&cmd,mStatus);
 
	makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}




void SRPC_Util_Get_Device_Info_Cfm(uint8_t status,uint8_t ieeeaddr[8],uint16_t shortAddr,uint8_t devcietype,uint8_t devciestate,uint8_t NumassocDevcies,uint16_t *AssocDeviceList)
{
	uint8_t mIndex = 0;
	hostCmd cmd;
    cmd.idx = 0;

	makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,ZIGBEE_DEVICE_UTIL_GET_DEVICE_INFO_CFM);
    //D2 devCount
    cmdSet8bitVal(&cmd,status);
    //IEEE
    cmdSetStringVal(&cmd,ieeeaddr,8);
	cmdSet16bitVal(&cmd,shortAddr);
	cmdSet8bitVal(&cmd,devcietype);
	cmdSet8bitVal(&cmd,devciestate);
	cmdSet8bitVal(&cmd,NumassocDevcies);

	for(mIndex = 0;mIndex<NumassocDevcies;mIndex++)
	{
		cmdSet16bitVal(&cmd,AssocDeviceList[mIndex]);
	}

	makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


#if PERMIMNG
static uint8_t SRPC_PermissionRequest_Resp(hostCmd *cmd)
{
	log_debug("SRPC_PermissionRequest_Resp++");

	ASSERT(cmd != NULL);

	uint8_t PermiType = 0;
	uint8_t PermiData[PERMI_BUF_SIZE] = {0};
	struct tm tp;;
	time_t timep;
	
	if((cmd->size-1) <= PERMI_BUF_SIZE)
	{
		cmdGet8bitVal(cmd,&PermiType);
		cmdGetStringVal(cmd,PermiData,10);

		sprintf(PermiData+strlen(PermiData)," 00:00:00");
		
		if(NULL==strptime(PermiData,"%Y-%m-%d %H:%M:%S",&tp))
		{
			return 0;
		}
		
		tp.tm_isdst = -1; 
		timep = mktime(&tp);
//		log_debug("times:%ld",timep);
//		log_debug("times:%s",asctime(localtime(&timep)));
		PermiMng_WriteConfig(PermiType,timep);
		g_PermiMngRequestScuess = true;
		PermiMng_Config();
		zbSoc_Heartbeat_Permimng_Report_stop();
		PermiMng_close();
	}

	return 0;
}


uint8_t SRPC_PermissionRequest_Ind(void)
{
	log_debug("SRPC_PermissionRequest_Ind++");
	
	hostCmd cmd;
	cmd.idx = 0;
	
	makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_HOST_PERMISSION_RESP);
   
	makeMsgEnder(&cmd);

	PermiMng_SendMsg(cmd.data,cmd.idx);
  
	return 0;
}

void SRPC_PermissionMng(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
		case WIFI_HOST_PERMISSION_RESP:

		SRPC_PermissionRequest_Resp(cmd);
		break;
		default:break;
	}
}
#endif

/************************************************
*				系统设置(00H)					*
************************************************/

void SRPC_SystemProcess(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
		 
        case WIFI_CONFIG_SSID_PASSWD_CMD_REQ:	//设置路由无线模式
            SRPC_ConfigSsidPasswd(cmd);
            break;
        case WIFI_CONFIG_LAN_ETHERNET_CMD_REQ:	//设置路由有线模式
            SRPC_ConfigEthernetCmdReq(cmd);
            break;
        case WIFI_GET_MAC_ADDR_CMD_REQ:			//获取路由器的 MAC地址
            SRPC_GetMacAddrCmdReq(cmd);
            break;
        case WIFI_GET_COORD_VERSION_CMD_REQ:	//获取协调器固件版本
            SRPC_GetCoordVersionCmdReq(cmd);
            break;
        case WIFI_CONFIG_AP_SSID_CMD_REQ:		//控制无线AP
            SRPC_ConfigApSSidCmdReq(cmd);
            break;
		case WIFI_CONFIG_GET_VERSION_CMD_REQ:	//获取固件版本
			SRPC_ConfigGetVersionCmdReq(cmd);
			break;
		case WIFI_MODIFY_SSID_REQ:				//修改SSID命令
			SRPC_ConfigModifySSIDCmdReq(cmd);
			break;
		case WIFI_MODIFY_PASSWORD_REQ:
			SRPC_ConfigModifyPassWordCmdReq(cmd);	//修改主机密码
			break;
		case WIFI_SYSTEM_REBOOT_REQ:			//系统重启命令
			SRPC_SystemRebootReq(cmd);
			break;
		case WIFI_CLOSE_NETWORK_CMD_REQ:		//关闭网络命令
			SRPC_CloseNetWork(cmd);
			break;
        case WIFI_OPEN_NETWORK_CMD_REQ:			//开放网络命令
            SRPC_permitJoin(cmd);
            break;
        case WIFI_REVERT_ALL_DEV_FACTORY_SETTING_CMD_REQ:	//所有设备恢复出厂设置
            SRPC_RevertFactorySettingReq(cmd);
            break;
        case WIFI_REVERT_ONE_DEV_FACTORY_SETTING_CMD_REQ:	//恢复指定设备至出厂设置
            SRPC_RevertOneFactorySettingReq(cmd);
            break;
        case WIFI_SET_COORD_RESET_REQ:				//重启协调器
			SRPC_SetCoordResetReq(cmd);
        	break;
		case WIFI_SET_MASTER_HOST_UPDATE_REQ:		//远程更新主机固件
			SRPC_UpDateMasterHostFWReq(cmd);
			break;
		case WIFI_SET_COORD_CHANNELS_REQ:
			SRPC_MtUtilSetChannelsReq(cmd);			//切换信道
			break;
		case WIFI_GET_PANID_AND_CHANNELS_REQ:		//读取协调器的网络PANID和CHANNEL信息
			SRPC_GetCoordPanidAndChannelsReq(cmd);
			break;
		case WIFI_GATEWAY_FACTORY_RESP:
			log_debug("system to fasctory\n");
			break;
		default:
			break;
	}
}

/************************************************
*				所有设备产品管理(01H)			*
************************************************/

void SRPC_DeviceManager(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
        case WIFI_ROOMFAIRY_REGISTER_RESP:		//主机注册反馈
            SRPC_RoomfairyRegisterResp(cmd);
            break;
        case WIFI_ONLINE_DEVICE_HEART_RESP:		//设备心跳反馈
            SRPC_heartPacketResp(cmd);
            break;
        case WIFI_ALLOW_DEV_CONN_NET_REQ:		//允许节点入网请求
            SRPC_AllowDevConnectNetworkReq(cmd);
            break;
        case WIFI_QUERY_ALL_UNREGISTER_DEV_REQ:	//查询当前已经入网单未批准的设备
            SRPC_QueryAllUnregisterDevReq(cmd);
            break;
        case WIFI_DELETE_DEV_CONN_NET_REQ:		//删除已经入网的节点命令请求
            SRPC_DeleteDevConnectNetworkReq(cmd);
            break;
        case WIFI_QUERY_ONLINE_DEVICE_REQ:		//查询当前在线设备命令
            SRPC_QueryDevOnlineStateReq(cmd);
            break;
        case WIFI_ZIGBEE_COM_DEV_REGISTER_RESP:	//通用节点的注册上报通知反馈
            SRPC_ComDevRegisterResp(cmd);
            break;
		case WIFI_ZIGBEE_GET_VERSION_REQ:		//获取节点固件版本
			SRPC_GetDevVersionReq(cmd);
			break;
		default:
			break;
	}
}


/************************************************
*				协调器管理(02H)			*
************************************************/

void SRPC_CoordinatorManager(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
		case ZIGBEE_DEVICE_UTIL_GET_DEVICE_INFO_REQ:	//获取协调器信息
			mt_Util_Get_Device_Info_sreq();	
		break;
		default:
			log_debug("Unsupported SRPC_CoordinatorManager CMD \n");
		break;
	}	
}

/************************************************
*				通用开关量节点(10H)				*
************************************************/

void SRPC_SwitchDevice(uint16_t opcode,hostCmd *cmd)
{
	log_debug("SRPC_SwitchDevice++\n");
	switch(opcode)
	{
		
        case WIFI_ZIGBEE_SWITCH_CTRL_REQ:
            SRPC_SwitchCtrlReq(cmd);
            break;
        case WIFI_ZIGBEE_SWITCH_STATUS_RESP:
            break;
        case WIFI_ZIGBEE_QUERY_SWITCH_STATUS_REQ:
            SRPC_QuerySwitchStatusReq(cmd);
            break;
		case WIFI_ZIGBEE_QUERY_ALL_SWITCH_STATUS_REQ:
			//SRPC_QueryAllSwitchStatusReq(cmd);
			break;
		case WIFI_ZIGBEE_SET_SWITCH_DEFAULT_REQ:
			SRPC_SetSwitchDefaultStateReq(cmd);
			break;
		case WIFI_ZIGBEE_GET_SWITCH_DEFAULT_REQ:
			SRPC_GetSwitchDefaultStateReq(cmd);
			break;
		case WIFI_ZIGBEE_QUERY_SIWTCH_VALUE_REQ:
			SRPC_QuerySwitchSocketValueReq(cmd);
			break;
		case WIFI_ZIGBEE_QUERY_SIWTCH_VALUE_RESP:
			
			break;
		default:
			break;
	}
	log_debug("SRPC_SwitchDevice--\n");
}


/************************************************
*					窗帘设备(11H)				*
************************************************/

void SRPC_CurtainDevice(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
	 
        case WIFI_ZIGBEE_WIN_CURTAIN_CTRL_REQ:
            SRPC_WinCurtainCtrlReq(cmd);
            break;
         case WIFI_ZIGBEE_WIN_CURTAIN_STATUS_RESP:
         	break;
         case WIFI_ZIGBEE_WIN_CURTAIN_PERCENTAGE_REQ:
			SRPC_WinCurtainPercentageReq(cmd);
         	break;
         case WIFI_ZIGBEE_WIN_CURTAIN_PERCENTAGE_RESP:
         	break;
		default:
			break;
	}
}

/************************************************
*				外转发器(12H)					*
************************************************/

void SRPC_InfraredDevice(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
	
        case WIFI_ZIGBEE_IRC_ENTER_LEARN_REQ:
            log_debug("WIFI_ZIGBEE_IRC_ENTER_LEARN_REQ.\n");
            //SRPC_EnterIRCLearnModeReq(cmd);
            break;
        case WIFI_ZIGBEE_IRC_EXIT_LEARN_REQ:
            log_debug("WIFI_ZIGBEE_IRC_EXIT_LEARN_REQ.\n");
           // SRPC_ExitIRCLearnModeReq(cmd);
            break;
        case WIFI_ZIGBEE_IRC_SET_LEARN_ADDR_REQ:
            log_debug("WIFI_ZIGBEE_IRC_SET_LEARN_ADDR_REQ.\n");
            //SRPC_SetIRCLearnAddrReq(cmd);
            break;
        case WIFI_ZIGBEE_IRC_SEND_CTRL_REQ:
            log_debug("WIFI_ZIGBEE_IRC_SEND_CTRL_REQ.\n");
            //SRPC_SendIRCCtrlReq(cmd);
            break;
        case WIFI_ZIGBEE_REMOTE_IRC_ENTER_LEARN_REQ:
            log_debug("WIFI_ZIGBEE_REMOTE_IRC_ENTER_LEARN_REQ.\n");
            //SRPC_EnterRemoteIRCLearnModeReq(cmd);
            break;
        case WIFI_ZIGBEE_REMOTE_IRC_EXIT_LEARN_REQ:
            log_debug("WIFI_ZIGBEE_REMOTE_IRC_EXIT_LEARN_REQ.\n");
            //SRPC_ExitRemoteIRCLearnModeReq(cmd);
            break;
        case WIFI_ZIGBEE_REMOTE_IRC_SEND_CTRL_REQ:
            log_debug("WIFI_ZIGBEE_REMOTE_IRC_SEND_CTRL_REQ.\n");
            //SRPC_SendRemoteIRCCtrlReq(cmd);
            break;
		//===================??=====================//	
		case WIFI_ZIGBEE_REMOTE_IRC_QUERY_DEVICE_REQ: //红外搜寻设备
			SRPC_QueryIRCDeviceWithLibReq(cmd);
			break;
			
		case WIFI_ZIGBEE_REMOTE_IRC_CTL_DEVICE_REQ: //红外控制设备
			SRPC_SendIRCDeviceWithLibReq(cmd);
			break;
			
		case WIFI_ZIGBEE_REMOTE_IRC_QUERY_DEVICE_ID_REQ://设备数量查询
			SRPC_QueryIRCDeviceIDWithLibReq(cmd);
			break;

		case WIFI_ZIGBEE_REMOTE_IRC_ENTER_LEARN_MODE_REQ: //远程红外学习模式
			SRPC_IRC_RemotelLearnModeReq(cmd);
			break;
			
		case WIFI_ZIGBEE_REMOTE_IRC_SEND_RAW_DATA_REQ: //发送原始数据
			SRPC_SendRemoteIRCRawDataReq(cmd);
			break;

//		case WIFI_ZIGBEE_REMOTE_IRC_EXIT_LEARN_MODE_REQ_TEST:
//			SRPC_SendIRCDeviceWithLibReqTest(cmd);

//			break;
		//===================??=====================//
		default:
			break;
	}
}

/************************************************
*				智能门锁(40H)						*
************************************************/

void SRPC_DoorLockDevice(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
	    case WIFI_ZIGBEE_DOORLOCK_CTRL_REQ:
	        SRPC_DoorLockCtrlReq(cmd);
	        break;
	    case WIFI_ZIGBEE_QUERY_DOORLOCK_POWER_VALUE_REQ:
	        SRPC_QueryDoorLockPowerValueReq(cmd);
	        break;
		case WIFI_ZIGBEE_QUERY_DOORLOCK_SPEED_REQ:
			SRPC_QueryDoorLockSpeedValueReq(cmd);
			break;
		case WIFI_ZIGBEE_SET_DOORLOCK_SPEED_REQ:
			SRPC_SetDoorLockSpeedValueReq(cmd);
			break;
			
#if DEVICE_LIWEI_DOOR_SUPPERT	
		//Level门锁
		case WIFI_ZIGBEE_GET_LEVEL_DOOR_INFO_REQ:
			SRPC_GetLevelDoorLockInfoReq(cmd);
		break;
		case WIFI_ZIGBEE_SET_LEVEL_DOOR_TIME_REQ:
			SRPC_SetLevelDoorLockTimeReq(cmd);
		break;
		case WIFI_ZIGBEE_GET_LEVEL_DOOR_REG_REQ:
			SRPC_GetLevelDoorLockRegReq(cmd);
		break;
		case WIFI_ZIGBEE_ADD_LEVEL_DOOR_USR_REQ:
			SRPC_LevelDoorLockUsrReq(cmd);
		break;
#if DEVICE_LIWEI_DOOR_OPEN_CNT
		case WIFI_ZIGBEE_GET_LEVEL_DOOR_OPEN_CNT_REQ:
			SRPC_LevelDoorLockOpenCntReq(cmd);
		break;
#endif
#endif
		default:
			break;
	}
}

/************************************************
*				智能取电开关(41H)				*
************************************************/

void SRPC_PowerSwitchDevice(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
		case WIFI_ZIGBEE_POWERSWITCH_CTRL_REQ:
	        SRPC_PowerSwitchCtrlReq(cmd);
	        break;
		case WIFI_ZIGBEE_POWERSWITCH_STATUS_RESP:
			break;
		case WIFI_ZIGBEE_POWERSWITCH_POWER_VALUE_RESP:
			break;
		case WIFI_ZIGBEE_QUERY_POWERSWITCH_POWER_VALUE_REQ:
			SRPC_QueryPowerSwitchValueReq(cmd);
			break;
		case WIFI_ZIGBEE_QUERY_POWERSWITCH_STATUS_REQ:
			SRPC_QueryPowerSwitchStateReq(cmd);
			break;
	    default:
	        break;
	}
}


/************************************************
*				温湿度光照节点控制(20H)			*
************************************************/
/*
void SRPC_HumiTureDevice(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
	    case WIFI_ZIGBEE_HUMITURE_LIGHT_CTRL_REQ:
	        SRPC_HumitureLightCtrlReq(cmd);
	        break;
	    case WIFI_ZIGBEE_HUMITURE_LIGHT_STATUS_RESP:
	        log_debug("WIFI_ZIGBEE_HUMITURE_LIGHT_STATUS_RESP.\n");
	        break;
	    case WIFI_ZIGBEE_QUERY_HUMITURE_LIGHT_STATUS_REQ:
	        log_debug("WIFI_ZIGBEE_HUMITURE_LIGHT_STATUS_REQ.\n");
	        SRPC_QueryHumitureLightReq(cmd);
	        break;
	    default:
	        break;
	}
}
*/
/************************************************
*				温湿度节点控制(20H)				*
************************************************/
void SRPC_HumiTureCtrlDevice(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
		case WIFI_ZIGBEE_HUMITURE_CTRL_REQ:
	        // SRPC_HumitureIntervalCtrlReq(cmd);
	        break;
	    case WIFI_ZIGBEE_HUMITURE_STATUS_RESP:
	        break;
	    case WIFI_ZIGBEE_QUERY_HUMITURE_STATUS_REQ:
	        SRPC_QueryHumitureReq(cmd);
	        break;
	    case WIFI_ZIGBEE_HUMITURE_ENABLE_REQ:
			//SRPC_HumitureValidCtrlReq(cmd);
	        break;
	    default:
	        break;
	}
}

/************************************************
*				环境节点控制(22H)				*
************************************************/

void SRPC_EnvironmentDevice(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
	    case WIFI_ZIGBEE_ENVIRONMENT_LIGHT_STATUS_RESP:
	        break;
	    case WIFI_ZIGBEE_ENVIRONMENT_TEMPHUM_STATUS_RESP:
	        break;
	    case WIFI_ZIGBEE_ENVIRONMENT_PM25_STATUS_RESP:
	        break;
	    case WIFI_ZIGBEE_ENVIRONMENT_NOICE_STATUS_RESP:
	        break;
	    default:
	        break;
	}
}

/************************************************
*				中央控制器(42H)						*
************************************************/

void SRPC_CentralAirConditiconingSystem(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
		//控制中央空调
		case WIFI_ZIGBEE_SET_CENTRAL_AIR_REQ:
			SPRC_SetCentralAirReqCmd(cmd);
		break;
		//中央空调状态上报反馈
		case WIFI_ZIGBEE_CENTRAL_AIR_STATE_RESP:
		//pass
		break;
		//查询中央空调状态
		case WIFI_ZIGBEE_QUERY_CENTRAL_AIR_REQ:
			SPRC_QueryCentralAirReqCmd(cmd);
		break;
		default:
        	break;
	}
}


/************************************************
*				报警器节点控制(30H)				*
************************************************/

void SRPC_AlarmDevice(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
	case WIFI_ZIGBEE_COM_ALARM_CTRL_REQ:
	    SRPC_ComAlarmCtrlReq(cmd);
	    break;
	case WIFI_ZIGBEE_COM_ALARM_GET_POWERS_REQ:
		SRPC_ComAlarmGetPowerReq(cmd);
		break;
    default:
        break;
	}
}

/************************************************
*				定时任务(03H)					*
************************************************/

void SRPC_TimerManager(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
	    case WIFI_ZIGBEE_PERIOD_TIMER_SWITCH_SET_REQ:
	        SRPC_addPeriodTimerSwitchCtrlReq(cmd);
	        break;
	    case WIFI_ZIGBEE_DELETE_PERIOD_TIMER_SWITCH_REQ:
	        SRPC_deletePeriodTimerSwitchCtrlReq(cmd);
	        break;
		case WIFI_ZIGBEE_GET_PERIOD_TIMER_SWITCH_REQ:
			SRPC_getPeriodTimerSwitchCtrlReq(cmd);
			break;
		case WIFI_ZIGBEE_CTL_PERIOD_TIMER_SWITCH_REQ:
			SRPC_CtlPeriodTimerSwitchCtrlReq(cmd);
			break;
		case WIFI_ZIGBEE_PERIOD_TIMER_SWITCH_INFO_RESP:
			//任务上报反馈
			break;
	    default:
	        break;
	}
}


/************************************************
			组管理及场景管理(04H)	
************************************************/

void SRPC_GroupSceneManager(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
		case WIFI_ZIGBEE_ADD_DEV_GROUP_REQ:
			SRPC_AddDevSceneReq(cmd);
			break;
		case WIFI_ZIGBEE_DEL_DEV_GROUP_REQ:
			SRPC_DelDevSceneReq(cmd);
			break;
		case WIFI_ZIGBEE_GET_DEV_GROUP_REQ:
			SRPC_GetDevSceneReq(cmd);
			break;
		case WIFI_ZIGBEE_CTL_DEV_GROUP_REQ:
			SRPC_CtlDevSceneReq(cmd);
			break;
		default:
			break;
	}
}

/************************************************
				事件管理(05H)
************************************************/

void SRPC_ActionEventManager(uint16_t opcode,hostCmd *cmd)
{
	switch(opcode)
	{
	  case WIFI_ZIGBEE_ACTION_EVENT_ADD_REQ:
		  SRPC_ActionEventAddReq(cmd);
		  break;
	  case WIFI_ZIGBEE_ACTION_EVENT_DEL_REQ:
	  	  SRPC_ActionEventDelReq(cmd);
	  	break;
	  case WIFI_ZIGBEE_ACTION_EVENT_GET_REQ:
	  	  SRPC_ActionEventGetAllReq(cmd);
	  	break;
	  case WIFI_ZIGBEE_ACTION_EVENT_CTL_REQ:
		   SRPC_ActionEventCtlReq(cmd);
	  	break;
		case WIFI_ZIGBEE_ACTION_EVENT_STATE_RESP:
			
			break;
	  default:
	      break;
	}
}

/************************************************************************
* 函数名 :SRPC_ProcessIncoming(hostCmd *cmd,uint32_t clientFd)
* 描述   :   处理接收到的消息包
* 输入   ：
* 输出   ：无
* 返回   ：无
************************************************************************/
void SRPC_ProcessIncoming(hostCmd *cmd)
{
    uint16_t tp=0;
    uint8_t  mac[7]= {0};
    uint8_t  dir=0;
    uint16_t opcode =0;
	uint8_t MainCmd =0;
	uint8_t subCmd  =0;

    log_debug("SRPC_ProcessIncoming++\n");

    cmdGet16bitVal(cmd, &tp);
    cmdGetStringVal(cmd, &mac[0],6);
    cmdGet8bitVal(cmd, &dir);
    cmdGet16bitVal(cmd, &opcode);//D0 D1
	
    log_debug("[TP:0x%x][MAC:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x][DIR:0x%x][OPCODE:0x%x]\n",tp,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],dir,opcode);

	if((dir != CMD_MSG_DIR_REQ) && (dir != CMD_MSG_DIR_RESP))
	{
		log_err("Msg Dir error %d\n",dir);
		return;
	}

    switch(opcode & 0xFF00)
    {
		case WIFI_SYSMTEM_PROCESS_ID:
			SRPC_SystemProcess(opcode,cmd);
			break;
		case WIFI_DEVICE_MANAGER_ID	:
			SRPC_DeviceManager(opcode,cmd);
			break;	
#if PERMIMNG
		case WIFI_PERMISSION_ID:
			SRPC_PermissionMng(opcode,cmd);
			break;
#endif
		case WIFI_COORD_DEVICE_ID:
			SRPC_CoordinatorManager(opcode,cmd);
			break;
		case WIFI_TIMER_MANAGER_ID:
			SRPC_TimerManager(opcode,cmd);
			break;							
		case WIFI_GROUP_SCENE_ID:
			SRPC_GroupSceneManager(opcode, cmd);
			break;
		case WIFI_ACTION_EVENT_ID:
			SRPC_ActionEventManager(opcode,cmd);
			break;
		case WIFI_SWITCH_DEVICE_ID:
			SRPC_SwitchDevice(opcode, cmd);
			break;							
		case WIFI_CURTAIN_DEVICE_ID:
			SRPC_CurtainDevice( opcode, cmd);
			break;							
		case WIFI_INFRARED_DEVICE_ID:
			SRPC_InfraredDevice( opcode, cmd);
			break;														
		case WIFI_HUMITURE_CTL_DEVICE_ID:
			SRPC_HumiTureCtrlDevice(opcode, cmd);
			break;						
		case WIFI_ENVIRONMENT_DEVICE_ID:
			SRPC_EnvironmentDevice(opcode, cmd);
			break;						
		case WIFI_ALARM_DEVICE_ID:
			SRPC_AlarmDevice( opcode, cmd);
			break;							
		case WIFI_DOORLOCk_DEVICE_ID:
			SRPC_DoorLockDevice( opcode, cmd);
			break;							
		case WIFI_POWERSWITCH_DEVICE_ID:
			SRPC_PowerSwitchDevice( opcode, cmd);
			break;
		case WIFI_CENTRALAIR_DEVICE_ID:
			SRPC_CentralAirConditiconingSystem(opcode,cmd);
			break;
		default:
			break; 
    }
 
    log_debug("SRPC_ProcessIncoming--\n");
}


