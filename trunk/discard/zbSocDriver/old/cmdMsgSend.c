/**************************************************************************************************
 * Filename:       cmdMsgSend.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *
 *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/msg.h>

#include "cJSON.h"
#include "hal_defs.h"
#include "hal_types.h"

#include "comMsgPrivate.h"
#include "cmdMsgSend.h"
#include "globalVal.h"
#include "LogUtils.h"

/***************************************************************************************************
 * @fn      cmdMsgSend
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void cmdMsgSend(uint8* cmdMsg,uint16 cmdMsgLen)
{
    bool rtn;

    rtn = SocketInterface_Send_packet(cmdMsg,cmdMsgLen);
    //socketSeverSend(cmdMsg, cmdMsgLen, Remote_Socket->socketFd);
    if (rtn == false)
    {
        LOG_PRINT("[cmdMsgSend]:ERROR writing to socket\n");
    }
    return;
}

/************************************************************************
* 函数名 :makeMsgHeader(hostCmd *cmd,uint8 dir)
* 描述   :  设置数据包的包头数据
* 输入   ：*cmd:数据包指针,dir 为数据包的方向
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
bool makeMsgHeader(hostCmd *cmd,uint8 dir)
{
    cmdSet8bitVal(cmd,CMD_MSG_FLAG);
    cmdSet16bitVal(cmd,0);//数据包长度先置为0
    cmdSet16bitVal(cmd,CMD_MSG_TP);
    cmdSetStringVal(cmd,roomfairy_WifiMac,6);
    cmdSet8bitVal(cmd,dir);
    return true;
}

/************************************************************************
* 函数名 :makeMsgEnder(FS_uint8 data)
* 描述   :  设置数据包的包尾数据
* 输入   ：*cmd:数据包指针,dir 为数据包的方向
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
bool makeMsgEnder(hostCmd *cmd)
{
    cmd->data[CMD_MSG_LEN0_POS] = (((cmd->idx-3)>>8)&0xff);
    cmd->data[CMD_MSG_LEN1_POS] = ((cmd->idx-3)&0xff);
    cmdSetFCS(cmd);
    return true;
}

/************************************************************************
* 函数名 :zigbeeDev_ConfigSsidPasswdCfm(uint8 status)
* 描述   :   配置路由器的ssid和密码命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_ConfigSsidPasswdCfm(uint8 status)
{
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
* 函数名 :zigbeeDev_ConfigEthernetCmdCfm(uint8 status)
* 描述   :   配置路由器的ssid和密码命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_ConfigEthernetCmdCfm(uint8 status)
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
* 函数名 :zigbeeDev_ConfigApSSidCmdCfm(uint8 status)
* 描述   :   配置路由器热点命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_ConfigApSSidCmdCfm(uint8 status)
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
* 函数名 :zigbeeDev_GetMacAddrCfm(uint8 status)
* 描述   :   获取Mac地址应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_GetMacAddrCfm(uint8* macAddr)
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
* 函数名 :zigbeeDev_GetDevAddrCfm(uint8 status)
* 描述   :   获取Dev地址应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_GetDevAddrCfm(uint8* devAddr)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_GET_DEV_ADDR_CMD_CFM);
    //D2 Mac Addr
    cmdSetStringVal(&cmd,devAddr,6);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zigbeeDev_OpenNwkCfm(uint8 status)
* 描述   :   开放网络命令
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_OpenNwkCfm(uint8 status)
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
* 函数名 :zigbeeDev_RevertFactorySettingCfm(uint8 status)
* 描述   :   恢复协调器出厂设置命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_RevertFactorySettingCfm(uint8 status)
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
* 函数名 :zigbeeDev_RevertOneFactorySettingCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
* 描述   :   恢复单个设备至出厂设置命令应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_RevertOneFactorySettingCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_REVERT_ONE_DEV_FACTORY_SETTING_CMD_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_RoomfairyRegisterInd
 *
 * @brief   发送zigbee插座的注册信息到服务器
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_RoomfairyRegisterInd(void)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ROOMFAIRY_REGISTER_IND);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_ComDevRegisterInd
 *
 * @brief   发送zigbee节点通用的注册信息到服务器
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_ComDevRegisterInd(uint8* ieeeAddr,uint16 deviceid,uint8 portVal)
{
    uint8 cnt;
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_DEV_REGISTER_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    cmdSet8bitVal(&cmd,portVal);
    cmdSet16bitVal(&cmd,deviceid);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/************************************************************************
* 函数名 :zigbeeDev_heartPacketInd(uint8 devCount)
* 描述   :   设备心跳包消息上报
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void zigbeeDev_heartPacketInd(uint8 devCount,uint8* pBuf,uint16 pBufLen)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ONLINE_DEVICE_HEART_IND);
    //D2 devCount
    cmdSet8bitVal(&cmd,devCount);
    //发送所有的IEEE 值,IEEE len + Endpoint len=8+1=9
    cmdSetStringVal(&cmd,pBuf,pBufLen);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_QueryAllUnregisterDevCfm
 *
 * @brief   发送查询到的所有未注册的节点应答
 * @param
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_QueryAllUnregisterDevCfm(uint8 devCount,uint8* buf,uint16 bufLen)
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


/************************************************************************
* 函数名 :roomfairy_LightCtrlCfm(uint8 status)
* 描述   :   主机灯光调节及控制应答
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void roomfairy_LightCtrlCfm(uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ROOMFAIRY_LIGHT_CTRL_CFM);
    //D2 Status
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_PeriodTimerSwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_PeriodTimerSwitchCtrlCfm(uint8 status,uint8 taskId)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_PERIOD_TIMER_SWITCH_SET_CFM);
    //D2 控制结果
    cmdSet8bitVal(&cmd,status);
    //D3 任务ID
    cmdSet8bitVal(&cmd,taskId);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_DeletePeriodTimerSwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_DeletePeriodTimerSwitchCtrlCfm(uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DELETE_PERIOD_TIMER_SWITCH_CFM);
    //D2 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_AddDevGroupCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_AddDevGroupCfm(uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ADD_DEV_GROUP_CFM);
    //D2 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      zigbeeDev_SwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_SwitchCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SWITCH_CTRL_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_SwitchStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_SwitchStateInd(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_SWITCH_STATUS_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    cmdSet8bitVal(&cmd,endpoint);
    cmdSet8bitVal(&cmd,status);
    //D12 信号质量
    cmdSet8bitVal(&cmd,rssi);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      zigbeeDev_EnterIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_EnterIRCLearnModeCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_ENTER_LEARN_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_ExitIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_ExitIRCLearnModeCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_EXIT_LEARN_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_ExitIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_SetIRCLearnAddrCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_SET_LEARN_ADDR_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_ExitIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_IRCLearnRetInd(uint8* ieeeAddr,uint8 endpoint,uint8 addr)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_LEARN_STATUS_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,addr);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_SendIRCCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_SendIRCCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_SEND_CTRL_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_EnterRemoteIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_EnterRemoteIRCLearnModeCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_ENTER_LEARN_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_ExitRemoteIRCLearnModeCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_ExitRemoteIRCLearnModeCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_EXIT_LEARN_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_SendRemoteIRCCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_SendRemoteIRCCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_REMOTE_IRC_SEND_CTRL_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_RemoteIRCLearnRetInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_RemoteIRCLearnRetInd(uint8* ieeeAddr,uint8 endpoint,uint16 dataLen,uint8 *data)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_IRC_LEARN_STATUS_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet16bitVal(&cmd,dataLen);
    cmdSetStringVal(&cmd,data,dataLen);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_WinCurtainCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_WinCurtainCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_WIN_CURTAIN_CTRL_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      zigbeeDev_HumitureLightCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_HumitureLightCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_HUMITURE_LIGHT_CTRL_CFM);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      zigbeeDev_HumitureLightStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_HumitureLightStateInd(uint8* ieeeAddr,uint8 endpoint,uint16 temp,uint16 hum,uint16 ilum)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_HUMITURE_LIGHT_STATUS_IND);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11-D12 温度值
    cmdSet16bitVal(&cmd,temp);
    //D13-D14 湿度值
    cmdSet16bitVal(&cmd,hum);
    //D15-D16 光强度值
    cmdSet16bitVal(&cmd,ilum);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_QueryHumitureLightStateCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_QueryHumitureLightStateCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint16 temp,uint16 hum,uint16 light)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_HUMITURE_LIGHT_STATUS_CFM);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11-D12 温度值
    cmdSet16bitVal(&cmd,temp);
    //D13-D14 湿度值
    cmdSet16bitVal(&cmd,hum);
    //D15-D16 光照值
    cmdSet16bitVal(&cmd,light);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_HumitureIntervalCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_HumitureIntervalCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_HUMITURE_CTRL_CFM);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_HumitureStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_HumitureStateInd(uint8* ieeeAddr,uint8 endpoint,uint16 temp,uint16 hum)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_HUMITURE_STATUS_IND);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11-D12 温度值
    cmdSet16bitVal(&cmd,temp);
    //D13-D14 湿度值
    cmdSet16bitVal(&cmd,hum);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      zigbeeDev_QueryHumitureStateCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_QueryHumitureStateCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint16 temp,uint16 hum)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_HUMITURE_STATUS_CFM);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11处理状态
    cmdSet8bitVal(&cmd,status);
    //D12-D13 温度值
    cmdSet16bitVal(&cmd,temp);
    //D14-D15 湿度值
    cmdSet16bitVal(&cmd,hum);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_EnvironmentTempHumStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_EnvironmentTempHumStateInd(uint8* ieeeAddr,uint8 endpoint,uint16 temp,uint16 hum)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ENVIRONMENT_TEMPHUM_STATUS_IND);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11-D12 温度值
    cmdSet16bitVal(&cmd,temp);
    //D13-D14 湿度值
    cmdSet16bitVal(&cmd,hum);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_EnvironmentLightStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_EnvironmentLightStateInd(uint8* ieeeAddr,uint8 endpoint,uint8 lightState)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ENVIRONMENT_LIGHT_STATUS_IND);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11 光照等级值
    cmdSet8bitVal(&cmd,lightState);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_EnvironmentPM25StateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_EnvironmentPM25StateInd(uint8* ieeeAddr,uint8 endpoint,uint8 pmState)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ENVIRONMENT_PM25_STATUS_IND);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11 PM2.5 值
    cmdSet8bitVal(&cmd,pmState);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_EnvironmentNoiceStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_EnvironmentNoiceStateInd(uint8* ieeeAddr,uint8 endpoint,uint8 noiceState)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ENVIRONMENT_NOICE_STATUS_IND);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11 噪音等级值
    cmdSet8bitVal(&cmd,noiceState);

    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      zigbeeDev_ComAlarmCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_ComAlarmCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_ALARM_CTRL_CFM);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_ComAlarmStateInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_ComAlarmStateInd(uint8* ieeeAddr,uint8 endpoint,uint16 deviceID,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_COM_ALARM_STATUS_IND);
    //D2-D9 IEEE地址
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 端口号
    cmdSet8bitVal(&cmd,endpoint);
    //D11~D12状态值
    cmdSet16bitVal(&cmd,deviceID);
    //D13状态值
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


/***************************************************************************************************
 * @fn      zigbeeDev_DoorLockCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_DoorLockCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_CTRL_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_DoorLockCtrlInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_DoorLockCtrlInd(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_STATUS_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    //D12 信号质量
    cmdSet8bitVal(&cmd,rssi);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_DoorLockPowerValueInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_DoorLockPowerValueInd(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_POWER_VALUE_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    //D12 信号质量
    cmdSet8bitVal(&cmd,rssi);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_DoorLockPowerValueInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_DoorLockPowerValueResp(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_DOORLOCK_POWER_VALUE_RESP);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet16bitVal(&cmd,status);
    //D12 信号质量
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_QueryPowerValueCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_QueryPowerValueCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_QUERY_DOORLOCK_POWER_VALUE_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_PowerSwitchCtrlCfm
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_PowerSwitchCtrlCfm(uint8* ieeeAddr,uint8 endpoint,uint8 status)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_CFM);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_POWERSWITCH_CTRL_CFM);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

/***************************************************************************************************
 * @fn      zigbeeDev_PowerSwitchCtrlInd
 *
 * @brief   Send a message over SRPC to a clients.
 * @param   uint8_t* srpcMsg - message to be sent
 *
 * @return  Status
 ***************************************************************************************************/
void zigbeeDev_PowerSwitchCtrlInd(uint8* ieeeAddr,uint8 endpoint,uint8 status,uint8 rssi)
{
    hostCmd cmd;
    cmd.idx = 0;
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_POWERSWITCH_STATUS_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,ieeeAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,status);
    //D12 信号质量
    cmdSet8bitVal(&cmd,rssi);
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}
