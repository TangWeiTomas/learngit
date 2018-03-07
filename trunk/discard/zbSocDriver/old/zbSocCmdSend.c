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
* ������ :makeMsgEnder(FS_uint8 data)
* ����   :  �������ݰ��İ�β����
* ����   ��*cmd:���ݰ�ָ��,dir Ϊ���ݰ��ķ���
* ���   ����
* ����   ��uint8  ���ش�����TRUE ����ʧ��FALSE����ɹ�
************************************************************************/
uint8 zbMakeMsgEnder(hostCmd *cmd)
{
    cmd->data[SOC_MSG_LEN_POS] = ((cmd->idx-4)&0xff);
    cmd->data[SOC_MSG_DATALEN_POS] = ((cmd->idx-SOC_MSG_DATALEN_POS-1)&0xff);
    cmdSetFCS(cmd);
    return true;
}

/************************************************************************
* ������ :zbSoc_RevertFactorySettingCmd(uint8 status)
* ����   :   �ָ�Э��������������������
* ����   ��
* ���   ����
* ����   ��0:����ɹ�
************************************************************************/
void zbSoc_RevertFactorySettingCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;

    printf("zbSoc_RevertFactorySettingCmd !\n");

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER); 	//����Reserved
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint	//����Reserved
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, afAddrBroadcast);//Addr mode			//����Reserved
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control					//����Reserved
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number //����Reserved
    cmdSet8bitVal(&cmd, APP_ZCL_CMD_REVERT_FACTORY_SETTING);//cmd ID
    cmdSet16bitVal_lh(&cmd, 0x0);//ZCL CLUSTER ID   				//����Reserved

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);
}


/************************************************************************
* ������ :zbSoc_RevertOneDevFactorySettingCmd(uint8 status)
* ����   :   �ָ������豸������������������
* ����   ��
* ���   ����
* ����   ��0:����ɹ�
************************************************************************/
void zbSoc_RevertOneDevFactorySettingCmd(uint16 dstAddr,uint8 endpoint)
{
    hostCmd cmd;
    cmd.idx=0;

    printf("zbSoc_RevertOneDevFactorySettingCmd +++++++++++++++!\n");

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸�ָ���豸
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
* ������ :zbSoc_GetDevIDCmd(void)
* ����   :   ��ȡ�豸ID ��������
* ����   ��
* ���   ����
* ����   ��0:����ɹ�
************************************************************************/
void zbSoc_GetState(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
{
	hostCmd cmd;
    cmd.idx=0;
	LOG_PRINT("zbSoc_GetState++\n");
	
    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_GET_STATE);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type
    cmdSet8bitVal(&cmd, 0x2);
    cmdSet16bitVal(&cmd, 0xffff);//����ѧϰģʽ����

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);
	
}

/************************************************************************
* ������ :zbSoc_GetDevIDCmd(void)
* ����   :   ��ȡ�豸ID ��������
* ����   ��
* ���   ����
* ����   ��0:����ɹ�
************************************************************************/
void zbSoc_GetDevIDCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;
	
    printf("zbSoc_GetDevIDCmd. \n");

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER);
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
* ������ :zbSoc_GetDevMACCmd(void)
* ����   :   ��ȡ�豸ID ��������
* ����   ��
* ���   ����
* ����   ��0:����ɹ�
************************************************************************/
void zbSoc_GetDevMACCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;

    LOG_DEBUG("zbSoc_GetDevMACCmd. \n");

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
	cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
	cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
	//���͸����е�·���豸,����������
	cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER);
	cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint
	cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
	cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
* ������ :zbSoc_OpenNwkCmd(uint8 status)
* ����   :   ����������������
* ����   ��
* ���   ����
* ����   ��0:����ɹ�
************************************************************************/
void zbSoc_OpenNwkCmd(uint8 duration)
{
    hostCmd cmd;
    cmd.idx=0;

    LOG_PRINT("zbSoc_OpenNwkCmd: duration %ds\n", duration);

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER);	//����Reserved
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint	//����Reserved
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, afAddrBroadcast);//Addr mode			//����Reserved
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control					//����Reserved
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number//����Reserved
    cmdSet8bitVal(&cmd, APP_ZCL_CMD_OPEN_NETWORK);//cmd ID
    cmdSet16bitVal_lh(&cmd, duration);//ZCL CLUSTER ID			

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);

    //wait for message to be consumed
    usleep(30);
}

/************************************************************************
* ������ :zbSoc_CoorSystemResetCmd(uint8 status)
* ����   :   Э������λ��������
* ����   ��
* ���   ����
* ����   ��0:����ɹ�
************************************************************************/
void zbSoc_CoorSystemResetCmd(void)
{
    hostCmd cmd;
    cmd.idx=0;

    printf("zbSoc_CoorSystemResetCmd\n");

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, MT_NWKADDR_BROADCAST_ALL_ROUTER);
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_NO_USE);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
* ������ :zbSoc_SetGenOnOffState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* ����   :   ����On/Off��������
* ����   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_SetGenOnOffState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_ON_OFF);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, state);//ZCL COMMAND ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_QueryLightValueState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* ����   :   ��ѯ����״ֵ̬
* ����   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_QueryLightValueState(uint16 dstAddr, uint8 endpoint, uint8 addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_ON_OFF);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_ON_OFF);//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}



/************************************************************************
* ������ :zbSoc_QueryDoorLockPowerValueState(uint8 state, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* ����   :   ��ѯ��������ֵ
* ����   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_QueryDoorLockPowerValueState(uint16 dstAddr, uint8 endpoint, uint8 addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_READ);//ZCL COMMAND ID
    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_POWER_VALUE);//Attr ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_SetTempIntervalReportReq(uint16 intervalTime, uint16 dstAddr,uint8 endpoint, uint8 addrMode)
* ����   :   �����¶��ϱ��ļ��ʱ��
* ����   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_SetTempIntervalReportReq(uint16 intervalTime, uint16 dstAddr,uint8 endpoint, uint8 addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
* ������ :zbSoc_SetDevValidReq(bool onoff, uint16 dstAddr,uint8 endpoint, uint8 addrMode)
* ����   :   ����ͨ�ýڵ��豸����/���õĽӿ�
* ����   :  state - 0: Off, 1: On.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_SetDevValidReq(bool onoff, uint16 dstAddr,uint8 endpoint, uint8 addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
* ������ :zbSoc_IRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 ctrlcmd)
* ����   :   ���ƺ���ת�����ڵ����ѧϰģʽ
* ����   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_IRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ctrlcmd)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, 0x2);
    cmdSet16bitVal(&cmd, ctrlcmd);//����ѧϰģʽ����

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_IRCDevCtrlSetLearnAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr)
* ����   :   ���ƺ���ת�����ڵ����ѧϰģʽ
* ����   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_IRCDevCtrlSetLearnAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, 0x3);
    cmdSet16bitVal(&cmd, IRC_CTRL_SET_LEARN_ADDR);//����ѧϰģʽ����
    cmdSet8bitVal(&cmd, cmdaddr);

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_IRCDevCtrlSendAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr)
* ����   :   ���ƺ���ת�����ڵ����ѧϰģʽ
* ����   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_IRCDevCtrlSendAddrCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdaddr)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, 0x3);
    cmdSet16bitVal(&cmd, IRC_CTRL_SEND_CMD_ADDR);//����ѧϰģʽ����
    cmdSet8bitVal(&cmd, cmdaddr);

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_RemoteIRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 ctrlcmd)
* ����   :   ����Զ�̺���ת�����ڵ����ѧϰģʽ
* ����   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_RemoteIRCDevCtrlLearnModeCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ctrlcmd)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_LONG_OCTET_STR);//Data Type

    cmdSet16bitVal_lh(&cmd, 0x0002);
    cmdSet16bitVal(&cmd, ctrlcmd);//����ѧϰģʽ����

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_RemoteIRCDevCtrlCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 ctrlcmd)
* ����   :   ����Զ�̺���ת�����ڵ����ѧϰģʽ
* ����   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_RemoteIRCDevCtrlCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint16 ircDataLen,uint8 *ircData)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x00);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, ZCL_CMD_WRITE);//ZCL COMMAND ID

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_IR_OPER);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_LONG_OCTET_STR);//Data Type

    cmdSet16bitVal_lh(&cmd, ircDataLen);//����ѧϰģʽ����
    cmdSetStringVal(&cmd, ircData,ircDataLen);//����ѧϰģʽ����

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_CurtainDevCtrlCmdReq(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdvalue)
* ����   :   ���ʹ�����������������
* ����   :  dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_CurtainDevCtrlCmdReq(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 cmdvalue)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸����е�·���豸,����������
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_ON_OFF);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, cmdvalue);//ZCL COMMAND ID    
    							  //COMMAND_OPEN(0x04��)
    							  //COMMAND_CLOSE(0x05��)
    							  //COMMAND_STOP(0x06��ͣ)

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_RemoveGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* ����   :   ������Ա����
* ����   :  groupId - Group ID of the Scene.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
************************************************************************/
void zbSoc_AddGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint,uint8 addrMode)
{
    hostCmd cmd;
    cmd.idx =0;

    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸�ָ���Ľڵ��豸
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_GROUPS);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
* ������ :zbSoc_RemoveGroupMemberCmd(uint16 groupId, uint16 dstAddr, uint8 endpoint, uint8 addrMode)
* ����   :   �Ƴ����Ա����
* ����   :  groupId - Group Id
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
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
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸�ָ���Ľڵ��豸
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_GROUPS);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, COMMAND_GROUP_GET_REMOVE);//cmd ID
    cmdSet16bitVal_lh(&cmd, groupId);//ZCL Group ID

    zbMakeMsgEnder(&cmd);

    zbSocCmdSend(cmd.data,cmd.idx);
}

/************************************************************************
* ������ :zbSoc_AddSceneMemberCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr,
                                            uint8 endpoint, uint8 addrMode,uint16 deviceid,
                                            uint8 data1,uint8 data2,uint8 data3,uint8 data4)
* ����   :   Store Scene.
* ����   :  groupId - Group Id
                    sceneId - Scene ID of the Scene.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
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
            cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
            cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
            cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
            cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
            //���͸�ָ���Ľڵ��豸
            cmdSet16bitVal_lh(&cmd, dstAddr);
            cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
            cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_SCENES);//ZCL CLUSTER ID
            cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
        clusterid=0x0300;//�ʵ�
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
* ������ :zbSoc_RemoveSceneMemberCmd(uint16 groupId, uint8 sceneId, uint16 dstAddr,
                                                                uint8 endpoint, uint8 addrMode)
* ����   :   �Ƴ�������Ա
* ����   :  groupId - Group Id
                    sceneId - Scene ID of the Scene.
                    dstAddr - Nwk Addr or Group ID of the Light(s) to be controled.
                    endpoint - endpoint of the Light.
                    addrMode - Unicast or Group cast.
* ���   :  ��
* ����   :  0:����ɹ�
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
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸�ָ���Ľڵ��豸
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_SCENES);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
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
    cmdSet8bitVal(&cmd, 0);//len Ԥ��λ
    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
    cmdSet8bitVal(&cmd, MT_APP_MSG);//CMD1
    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
    //���͸�ָ���Ľڵ��豸
    cmdSet16bitVal_lh(&cmd, dstAddr);
    cmdSet8bitVal(&cmd, endpoint);//Dest APP Endpoint
    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_SCENES);//ZCL CLUSTER ID
    cmdSet8bitVal(&cmd, 0);//DataLen Ԥ��λ
    cmdSet8bitVal(&cmd, addrMode);//Addr mode
    cmdSet8bitVal(&cmd, 0x01);//Zcl Frame control
    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
    cmdSet8bitVal(&cmd, COMMAND_SCENE_RECALL);//cmd ID
    cmdSet16bitVal_lh(&cmd, groupId);//ZCL Group ID
    cmdSet8bitVal(&cmd, sceneId++);//Scene ID

    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}


