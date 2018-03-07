/***********************************************************************************
 * �� �� ��   : mt_zbSocCmd.c
 * �� �� ��   : Edward
 * ��������   : 2016��7��19��
 * �ļ�����   : zigbee mt ��ص�����
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/

#include "mt_zbSocCmd.h"
#include "zbSocPrivate.h"
#include "errorCode.h"
#include "logUtils.h"
#include "zbSocCmd.h"

typedef struct 
{
	uint8_t  ExtendedPanid[8];
	uint8_t  ExtendedAddress[8];
	uint8_t  devstate;
	uint8_t  Channels;
	uint16_t Panid;
	uint16_t ShortAddress;
	uint16_t ParentAddress;
}ZDO_Coordinator_Info_t;


static ZDO_Coordinator_Info_t CoordinatorInfo;

//////////////////////////////////Globals//////////////////////////////////////////
/*******************************/
/***        Channels         ***/
/*******************************/

//Channels:
//-DDEFAULT_CHANLIST=0x04000000  // 26 - 0x1A
//-DDEFAULT_CHANLIST=0x02000000  // 25 - 0x19
//-DDEFAULT_CHANLIST=0x01000000  // 24 - 0x18
//-DDEFAULT_CHANLIST=0x00800000  // 23 - 0x17
//-DDEFAULT_CHANLIST=0x00400000  // 22 - 0x16
//-DDEFAULT_CHANLIST=0x00200000  // 21 - 0x15
//-DDEFAULT_CHANLIST=0x00100000  // 20 - 0x14
//-DDEFAULT_CHANLIST=0x00080000  // 19 - 0x13
//-DDEFAULT_CHANLIST=0x00040000  // 18 - 0x12
//-DDEFAULT_CHANLIST=0x00020000  // 17 - 0x11
//-DDEFAULT_CHANLIST=0x00010000  // 16 - 0x10
//-DDEFAULT_CHANLIST=0x00008000  // 15 - 0x0F
//-DDEFAULT_CHANLIST=0x00004000  // 14 - 0x0E
//-DDEFAULT_CHANLIST=0x00002000  // 13 - 0x0D
//-DDEFAULT_CHANLIST=0x00001000  // 12 - 0x0C
//-DDEFAULT_CHANLIST=0x00000800  // 11 - 0x0B

const uint32_t Channels[] = 
{
	0x00000800,	//ͨ��11
	0x00001000,
	0x00002000,
	0x00004000,
	0x00008000,
	0x00010000,
	0x00020000,
	0x00040000,
	0x00080000,
	0x00100000,
	0x00200000,
	0x00400000,
	0x00800000,
	0x01000000,
	0x02000000,
	0x04000000 //ͨ��26
};

//�����ڸı�Э�������ŵ�ʱ���˱���������Ϊtrue
//��������ɻ���û�����ã��˱�������Ϊfalse
static bool ChangeCoordinatorChannels = false;

bool mt_getChangeCoordinatorChannels(void)
{
	return ChangeCoordinatorChannels;
}

void mt_setChangeCoordinatorChannels(bool state)
{
	ChangeCoordinatorChannels = state;
}

int mt_Util_Set_Channels_sreq(uint8_t channel)
{
	hostCmd cmd;
	
	cmd.idx =0;
	log_debug("mt_Util_Set_Channels_sreq++\n");
	//11��15��20��25�ŵ�
	if((11<=channel)&&(channel<=26))
	{
		//���ÿ��Ʊ�������ֹ�������
		mt_setChangeCoordinatorChannels(true);
		cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
    	cmdSet8bitVal(&cmd, 0);									//len Ԥ��λ
   	 	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_UTIL);	//CMD0
    	cmdSet8bitVal(&cmd, MT_UTIL_SET_CHANNELS);				//CMD1
    	cmdSet32bitVal_lh(&cmd,Channels[channel-11]);
    	zbMakeMsgEnder(&cmd);
    	zbSocCmdSend(cmd.data,cmd.idx);
    	return 0;
	}
	log_debug("mt_Util_Set_Channels_sreq--\n");
	return -1;
}

int8_t mt_Util_Set_Channels_srsp(hostCmd *cmd)
{
	uint8_t rspState = 0;
	if(NULL != cmd)
	{
		cmdGet8bitVal(cmd,&rspState);
		if(rspState == SUCCESS)
		{
			uint16_t id 	 = 0x03;
			uint8_t offset = 0x00;
			uint8_t vlen 	 = 0x01;
			uint8_t value  = 0x02;
			
			mt_Sys_Osal_Nv_Write_sreq(id,offset,vlen,&value);
			return 0;
		}
	}
	return -1;
}

//��ȡЭ�����ڵ���Ϣ
int8_t mt_Util_Get_Device_Info_sreq(void)
{
	hostCmd cmd;
	cmd.idx =0;
	
	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);									//len Ԥ��λ
	cmdSet8bitVal(&cmd, MT_RPC_CMD_AREQ | MT_RPC_SYS_UTIL);	//CMD0
	cmdSet8bitVal(&cmd, MT_UTIL_GET_DEVICE_INFO);			//CMD1

	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);

	return 0;
}

//zigbeeЭ��������
int8_t mt_Sys_Reset_sreq(void)
{
	hostCmd cmd;
	cmd.idx =0;
	
	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);									//len Ԥ��λ
	cmdSet8bitVal(&cmd, MT_RPC_CMD_AREQ | MT_RPC_SYS_SYS);	//CMD0
	cmdSet8bitVal(&cmd, MT_SYS_RESET_REQ);				//CMD1
	cmdSet8bitVal(&cmd,MT_SYS_RESET_REQ);
	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
	
	return 0;
}

int8_t mt_Sys_Reset_srsp(hostCmd *cmd)
{
	uint8_t Reason = 0;
	uint8_t TransportRev = 0;
	uint8_t Product = 0;
	uint8_t MinorRet = 0;
	uint8_t HwRev = 0;
	
	if(NULL != cmd)
	{
		cmdGet8bitVal(cmd,&Reason);
		cmdGet8bitVal(cmd,&TransportRev);
		cmdGet8bitVal(cmd,&Product);
		cmdGet8bitVal(cmd,&MinorRet);
		cmdGet8bitVal(cmd,&HwRev);
		log_debug("Reason %d \nTransportRev %d\nProduct %d\nMinorRet %d\nHwRev %d\n",Reason,TransportRev,Product,MinorRet,HwRev);
		if(0x02 == Reason)
		{
			log_debug("mt_Sys_Reset_srsp++\n");
			SRPC_SetCoordResetInd(Reason);
		}
	}
	
	return 0;
}

int8_t mt_Sys_Osal_Nv_Write_sreq(uint16_t id,uint8_t offset,uint8_t vlen,uint8_t *value)
{
	hostCmd cmd;
	cmd.idx =0;

	if(NULL == value)
		return -1;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);									//len Ԥ��λ
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_SYS);	//CMD0
	cmdSet8bitVal(&cmd, MT_SYS_OSAL_NV_WRITE);				//CMD1
	cmdSet16bitVal_lh(&cmd,id);
	cmdSet8bitVal(&cmd,offset);
	cmdSet8bitVal(&cmd,vlen);
	cmdSetStringVal_lh(&cmd,value,vlen);
	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
	
	return 0;
}

int8_t mt_Sys_Osal_Nv_Write_srsp(hostCmd *cmd)
{
	uint8_t rspState = 0;
	if(NULL != cmd)
	{
		cmdGet8bitVal(cmd,&rspState);
		if(rspState==SUCCESS)
		{
			//�������������Э�������ŵ�����λЭ����
			if(true == mt_getChangeCoordinatorChannels())
			{	
				mt_Sys_Reset_sreq();
				mt_setChangeCoordinatorChannels(false);
				SRPC_MtUtilSetChannelsCfm(YY_STATUS_SUCCESS);
			}
			
			return 0;
		}
	}

	return -1;
}


//��ȡ�����PanID���ŵ�����Ϣ
int8_t mt_Zdo_Ext_Nwk_Info_sreq(void)
{
	hostCmd cmd;
	cmd.idx =0;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);									//len Ԥ��λ
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_ZDO);	//CMD0
	cmdSet8bitVal(&cmd, MT_ZDO_EXT_NWK_INFO);					//CMD1
	//Data
	cmdSet8bitVal(&cmd,0x00);
	cmdSet8bitVal(&cmd,0x00);
	cmdSet8bitVal(&cmd,0x00);
	cmdSet8bitVal(&cmd,0x00);
	cmdSet8bitVal(&cmd,0x00);
	
	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
	return 0;
}


//��ȡ�����ŵ���PANID��Ϣ����
int8_t mt_Zdo_Ext_Nwk_Info_srsp(hostCmd *cmd)
{
//	uint16_t Panid  =0;
//	uint8_t  devstate = 0;
//	uint8_t Channels = 0;
//	uint16_t ShortAddress = 0;
//	uint16_t ParentAddress  = 0;
//	uint8_t  ExtendedPanid[8] = {0};
//	uint8_t  ExtendedAddress[8] = {0};
	

	cmdGet16bitVal_lh(cmd,&CoordinatorInfo.ShortAddress);
	cmdGet8bitVal(cmd,&CoordinatorInfo.devstate);
	cmdGet16bitVal_lh(cmd,&CoordinatorInfo.Panid);
	cmdGet16bitVal_lh(cmd,&CoordinatorInfo.ParentAddress);
	cmdGetStringVal_lh(cmd,CoordinatorInfo.ExtendedPanid,8);
	cmdGetStringVal_lh(cmd,CoordinatorInfo.ExtendedAddress,8);
	cmdGet8bitVal(cmd,&CoordinatorInfo.Channels);

	log_debug("devState:%d PANID:0x%x  Channels:0x%x\n",CoordinatorInfo.devstate,CoordinatorInfo.Panid,CoordinatorInfo.Channels);
	
	SRPC_GetCoordPanidAndChannelsInd(CoordinatorInfo.ExtendedPanid,CoordinatorInfo.Panid,CoordinatorInfo.Channels,CoordinatorInfo.devstate);
	
	return 0;
}

uint16_t mt_zdo_getCoorPaind(void)
{
	log_debug("CoordinatorInfo.devstate = %d\n",CoordinatorInfo.devstate);
	if(CoordinatorInfo.devstate==DEV_ZB_COORD)
		return CoordinatorInfo.Panid;
		
	return 0;
}

uint8_t mt_zdo_getCoorChannel(void)
{
	log_debug("CoordinatorInfo.devstate = %d\n",CoordinatorInfo.devstate);
	if(CoordinatorInfo.devstate==DEV_ZB_COORD)
		return CoordinatorInfo.Channels;
		
	return 0;
}

uint8_t mt_zdo_getCoorState(void)
{
	return CoordinatorInfo.devstate;
}

//���ݶ̵�ַ��ȡIEEE��ַ
uint8_t mt_Zdo_Ieee_addr_req(uint16_t ShortAddr)
{
	hostCmd cmd;
	cmd.idx =0;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);									//len Ԥ��λ
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_ZDO);	//CMD0
	cmdSet8bitVal(&cmd, MT_ZDO_IEEE_ADDR_REQ);				//CMD1
	cmdSet16bitVal_lh(&cmd,ShortAddr);
	cmdSet8bitVal(&cmd,SINGLE_DEVICE_RESPONSE);				//Reqtype
	cmdSet8bitVal(&cmd,0X00);
	
	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
	return 0;
}

//�����̵�ַ
int8_t mt_Zdo_Ieee_addr_rsp(hostCmd *cmd,epInfo_t *epinfo)
{
	int8_t ret = -1;
	log_debug("mt_Zdo_Ieee_addr_rsp++\n");
	cmdGet8bitVal(cmd,&ret);
	if(ret == SUCCESS)
	{
		cmdGetStringVal_lh(cmd,epinfo->IEEEAddr,8);
		cmdGet16bitVal_lh(cmd,&(epinfo->nwkAddr));
	}
	return ret;
}

int8_t mt_Zdo_Active_ep_req(uint16_t ShortAddr)
{
	hostCmd cmd;
	cmd.idx =0;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);									//len Ԥ��λ
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_ZDO);	//CMD0
	cmdSet8bitVal(&cmd, MT_ZDO_ACTIVE_ED_REQ);				//CMD1
	cmdSet16bitVal_lh(&cmd,ShortAddr);
	cmdSet16bitVal_lh(&cmd,ShortAddr);
	
	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
	return 0;
}

int8_t mt_Zdo_Simple_Desc_Req(uint16_t ShortAddr,uint8_t endpoint)
{
	hostCmd cmd;
	cmd.idx =0;

	cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
	cmdSet8bitVal(&cmd, 0);									//len Ԥ��λ
	cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_ZDO);	//CMD0
	cmdSet8bitVal(&cmd, MT_ZDO_SIMPLE_DESC_REQ);				//CMD1
	cmdSet16bitVal_lh(&cmd,ShortAddr);
	cmdSet16bitVal_lh(&cmd,ShortAddr);
	cmdSet8bitVal(&cmd,endpoint);
	
	zbMakeMsgEnder(&cmd);
	zbSocCmdSend(cmd.data,cmd.idx);
	return 0;
}


