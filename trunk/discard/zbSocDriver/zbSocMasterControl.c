/**************************************************************************************************
 * Filename:       zbSocMasterControl.c
 * Author:             edward
 * E-Mail:          ouxiangping@feixuekj.cn
 * Description:    �����豸�������豸������
 *
 *  Copyright (C) 2014 fei xue keji Company - http://www.feixuekj.cn
 *
 * Version:         1.00  (2014-11-30,10:03)    :   Create the file.
 *
 *
 *************************************************************************/

//#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <fcntl.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>

#include "Zigbee_device_Heartbeat_Manager.h"
#include "interface_vDeviceList.h"
#include "zbSocUart.h"
#include "zbSocCmd.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "errorCode.h"
#include "interface_devicelist.h"
#include "logUtils.h"
#include "Polling.h"
#include "PackageUtils.h"
#include "interface_srpcserver.h"
#include "One_key_match.h"
#include "Out_Of_Power.h"
#include "zbSocMasterControl.h"

#define MASTER_CONTROL_VIRTUAL_DEVICE_SIZE			16
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_SIZE	14

//����忨ȡ���豸
#define MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH	 0x01
//�����Ŵ�
#define MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT	 0X02 
//���⿪�ز���
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE2	 0X03
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE3	 0X04 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE4	 0X05 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE5	 0X06 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE6	 0X07 

#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE7	 0X09 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE8	 0X0a 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE9	 0X0b 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE10	 0X0c 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE11	 0X0d 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE12	 0X0e 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE13	 0X0f 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE14	 0X10 
#define MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE15	 0X11 


#define MASTER_CONTROL_ENDPOINT		0x08

const int MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH[MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_SIZE] = {
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE2,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE3,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE4,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE5,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE6,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE7,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE8,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE9,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE10,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE11,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE12,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE13,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE14,
	MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_LINE15
};

//��Ϫ�����豸ע��
void zbSoc_MasterControlRegister(epInfoExtended_t *epInfoEx)
{	
	epInfo_t *epinfo;
	uint32_t context = 0;
	
    epInfo_t newData;
	epInfo_t virtualDevice;//�����豸

	//����������豸�����������ڵ��豸
	if(epInfoEx->epInfo->deviceID == ZB_DEV_MASTER_CONTROL)
	{
		//�����µ�����
		memcpy(&newData,epInfoEx->epInfo,sizeof(epInfo_t));

		//����
		if(epInfoEx->type == EP_INFO_TYPE_UPDATED) //�������е��豸
		{
			//���ҷ���Ҫ�������
			while((epinfo = devListGetNextDev(&context)) != NULL)
			{
				//ɾ�����з���Ҫ�������
				if((memcmp(newData.IEEEAddr,epinfo->IEEEAddr,Z_EXTADDR_LEN) == 0)&&(epinfo->deviceID != ZB_DEV_MASTER_CONTROL))
				{
					memcpy(&virtualDevice,epinfo,sizeof(epInfo_t));
					
					//devListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
					if(epinfo->deviceID == ZB_DEV_POWER_SWITCH)			//�忨ȡ���豸����
					{
						virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH;
						virtualDevice.deviceID = ZB_DEV_POWER_SWITCH;
						devListModifyRecordByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
						vdevListModifyByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
					}
					else if(epinfo->deviceID == ZB_DEV_DOOR_SENSOR)
					{
						virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT;
						virtualDevice.deviceID = ZB_DEV_DOOR_SENSOR;
						devListModifyRecordByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
						vdevListModifyByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
					}
					else if(epinfo->deviceID == ZB_DEV_ONOFF_SWITCH)
					{
						virtualDevice.endpoint = epinfo->endpoint;
						virtualDevice.deviceID = epinfo->deviceID;
						devListModifyRecordByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
						vdevListModifyByIeeeEp(virtualDevice.IEEEAddr,virtualDevice.endpoint,&virtualDevice);
					}
				}
			}
		}
		else if(epInfoEx->type == EP_INFO_TYPE_NEW) //������豸
		{
			//ɾ��ԭ�е������豸��Ϣ
			while((epinfo = devListGetNextDev(&context)) != NULL)
			{
				//ɾ�����з���Ҫ�������
				if((memcmp(newData.IEEEAddr,epinfo->IEEEAddr,8) == 0)&&(epinfo->deviceID != ZB_DEV_MASTER_CONTROL))
				{
					devListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
					vdevListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
				}
			}
			
			//��������豸
			memcpy(&virtualDevice,&newData,sizeof(epInfo_t));
			epInfoEx->epInfo = &virtualDevice;

			//1.���ȡ�翪��
			virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH;
			virtualDevice.deviceID = ZB_DEV_POWER_SWITCH;
			devListAddDevice(&virtualDevice);
			zbSoc_DevRegisterReporting(epInfoEx);

			//2.����ŴŸ�Ӧ
			virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT;
			virtualDevice.deviceID = ZB_DEV_DOOR_SENSOR;
			devListAddDevice(&virtualDevice);
			zbSoc_DevRegisterReporting(epInfoEx);

			//3.���14·����
			for(context = 0; context < MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_SIZE;context++)
			{
				virtualDevice.endpoint = MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH[context];
				virtualDevice.deviceID = ZB_DEV_ONOFF_SWITCH;
				devListAddDevice(&virtualDevice);
				zbSoc_DevRegisterReporting(epInfoEx);
			}
		}
		
	}
	else
	{
		zbSoc_DevRegisterReporting(epInfoEx);
	}
}


void zbSoc_MasterControlSetCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t deviceType,uint8_t switchCmd)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_LXZK_DEV_SET);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_UINT16);//Data Type

    cmdSet8bitVal(&cmd, deviceType);//�豸���ͣ����صĵڼ�·�豸
	cmdSet8bitVal(&cmd, switchCmd);//�豸״̬
   
    zbMakeMsgEnder(&cmd);
    
//	usleep(300000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}

//����Ƿ��������豸
uint8_t zbSoc_MasterControlCheck(uint8_t* ieeeAddr,uint8_t endpoint)
{
	epInfo_t *epInfo = NULL;
	epInfo = vdevListGetDeviceByIeeeEp(ieeeAddr,MASTER_CONTROL_ENDPOINT);
	
	if(epInfo!=NULL)
	{
		if(epInfo->deviceID == ZB_DEV_MASTER_CONTROL)
		{
			return true;
		}
	}
	
	return false;
}

void zbSoc_MasterControlQueryOnOffStateCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode,uint8_t deviceType)
{
    hostCmd cmd;
    cmd.idx =0;

//    cmdSet8bitVal(&cmd, SOC_MSG_FLAG);
//    cmdSet8bitVal(&cmd, 0);					//len Ԥ��λ
//    cmdSet8bitVal(&cmd, MT_RPC_CMD_SREQ | MT_RPC_SYS_APP);//CMD0
//    cmdSet8bitVal(&cmd, MT_APP_MSG);		//CMD1
//    cmdSet8bitVal(&cmd, MT_SOC_APP_ENDPOINT);//APP Endpoint
//    //���͸����е�·���豸,����������
//    cmdSet16bitVal_lh(&cmd, dstAddr);
//    cmdSet8bitVal(&cmd, endpoint);			//Dest APP Endpoint
//    cmdSet16bitVal_lh(&cmd, ZCL_CLUSTER_ID_GEN_BASIC);//ZCL CLUSTER ID
//    cmdSet8bitVal(&cmd, 0);					//DataLen Ԥ��λ
//    cmdSet8bitVal(&cmd, addrMode);			//Addr mode
//    cmdSet8bitVal(&cmd, 0x00);				//Zcl Frame control
//    cmdSet8bitVal(&cmd, zbTransSeqNumber++);//Zcl Squence Number
//    cmdSet8bitVal(&cmd, ZCL_CMD_READ);		//ZCL COMMAND ID
//    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_LX_MASTER);	//Attr ID //��ȡ�����豸״̬

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

    cmdSet16bitVal_lh(&cmd, ATTRID_LXZK_DEV_GET);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_UINT8);//Data Type

    cmdSet8bitVal(&cmd, deviceType);//�豸���ͣ����صĵڼ�·�豸
//	cmdSet8bitVal(&cmd, switchCmd);//�豸״̬
   
    zbMakeMsgEnder(&cmd);
    zbSocCmdSend(cmd.data,cmd.idx);
}


uint8_t zbSoc_MasterControlSetOnOffState(epInfo_t *epInfo,uint8_t switchcmd)
{
	log_debug("zbSoc_MasterControlSetOnOffState++\n");

//	vepInfo_t *vepInfo = NULL;
	epInfo_t *pvepInfo = NULL;
	
	ASSERT(epInfo != NULL);
	
	//memcpy(&mEpInfo,epInfo,sizeof(epInfo_t));
	
	//�жϵ�ǰ���豸�Ƿ������豸ģ���
	pvepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_ENDPOINT);

	if(pvepInfo != NULL)
	{
//		pvepInfo = &(vepInfo->epInfo);
		if(pvepInfo->deviceID == ZB_DEV_MASTER_CONTROL)//�ҵ��豸Ϊ���ذ��豸
		{
			log_debug("is ZB_DEV_MASTER_CONTROL\n");
			//���������豸
			zbSoc_MasterControlSetCmd(pvepInfo->nwkAddr,pvepInfo->endpoint,afAddr16Bit,epInfo->endpoint,switchcmd);
		}
		else//�ҵ������Ƿ������豸
		{
			log_debug("Not ZB_DEV_MASTER_CONTROL\n");
			zbSoc_SetGenOnOffState(switchcmd,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
	}
	else	//û�ҵ�DevieIDΪ���ذ��豸
	{
		log_debug("Not ZB_DEV_MASTER_CONTROL\n");
		zbSoc_SetGenOnOffState(switchcmd,epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
	}
	
	log_debug("zbSoc_MasterControlSetOnOffState--\n");
	return 0;
}


uint8_t zbSoc_MasterControlQueryOnOffState(epInfo_t *epInfo)
{
	epInfo_t mEpInfo;

//	vepInfo_t *vepInfo = NULL;
	epInfo_t *pvepInfo = NULL;
	
	ASSERT(epInfo != NULL);
	
	//memcpy(&mEpInfo,epInfo,sizeof(epInfo_t));

	pvepInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_ENDPOINT);
	
	if(pvepInfo != NULL)
	{
//		pvepInfo = &(vepInfo->epInfo);
		if(pvepInfo->deviceID == ZB_DEV_MASTER_CONTROL)//�ҵ��豸Ϊ���ذ��豸
		{
			//���������豸
			zbSoc_MasterControlQueryOnOffStateCmd(pvepInfo->nwkAddr,pvepInfo->endpoint,afAddr16Bit,epInfo->endpoint);
		}
		else//�ҵ������Ƿ������豸
		{
			zbSoc_QueryLightValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
		}
	}
	else	//û�ҵ�DevieIDΪ���ذ��豸
	{
		zbSoc_QueryLightValueState(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit);
	}
	
	return 0;
}

//�����豸���豸״̬�ϱ�����
uint8_t zbSoc_MasterControlReportResolve(hostCmd *cmd,epInfo_t *epInfo)
{
	uint16_t attrID;
	uint8_t dataType;
	uint8_t deviceType;
	uint8_t deviceState;
	uint8_t deviceCnt = 0;
	uint8_t cnt = 0;
	epInfo_t *devInfo = NULL;

	ASSERT(epInfo != NULL && cmd != NULL);
	
	log_debug("zbSoc_MasterControlReportResolve++\n");

	cmdGet16bitVal_lh(cmd, &attrID);
    cmdGet8bitVal(cmd, &dataType);

	//�����豸״̬�ϱ�
	if((attrID == ATTRID_BASIC_LX_MASTER) && (dataType == ZCL_DATATYPE_UINT16))
	{
		//ע���ֽ���
		cmdGet8bitVal(cmd, &deviceState);//״̬
		cmdGet8bitVal(cmd, &deviceType);//�ڼ�·
		
		devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,deviceType);

		if((devInfo==NULL )|| (devInfo->registerflag == false))
			return -1;

		//�����豸�е�RSSI
		devInfo->onlineDevRssi = epInfo->onlineDevRssi;
		
		if(deviceType == MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH)
		{
			//���յ����أ��ر��ط�����
            zblist_remove(devInfo->nwkAddr,devInfo->endpoint);
			//����ȡ�翪�ص�״̬�仯�ϱ�
			SRPC_PowerSwitchCtrlInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState,devInfo->onlineDevRssi);
		}
		else if(deviceType == MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT)
		{
			SRPC_ComAlarmStateInd(devInfo->IEEEAddr,devInfo->endpoint,devInfo->deviceID,deviceState);
			if(deviceState == On)//�����Ŵŵ�״̬��ʵ�ʵ��෴
			{
				Out_Of_Power_Start(OUT_OF_POWER_TIME);
			}
		}
		else
		{
			//���յ����أ��ر��ط�����
            zblist_remove(devInfo->nwkAddr,devInfo->endpoint);
			//���ؽڵ�Ŀ���״̬�仯�ϱ�
            SRPC_SwitchStateInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState,devInfo->onlineDevRssi);
		}
		zbSoc_ProcessEvent(devInfo,deviceState);
		
	}

/*
	if((attrID == ZB_ATTID_MASTER_CONTROL) && (dataType == ZCL_DATATYPE_UINT16))
	{
		
		
		devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,deviceType);

		if((devInfo==NULL )|| (devInfo->registerflag == false))
			return -1;

		//�����豸�е�RSSI
		devInfo->onlineDevRssi = epInfo->onlineDevRssi;
		
		if(deviceType == MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH)
		{
			//���յ����أ��ر��ط�����
            zblist_remove(devInfo->nwkAddr,devInfo->endpoint);
			//����ȡ�翪�ص�״̬�仯�ϱ�
			SRPC_PowerSwitchCtrlInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState,devInfo->onlineDevRssi);
		}
		else if(deviceType == MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT)
		{
			SRPC_ComAlarmStateInd(devInfo->IEEEAddr,devInfo->endpoint,devInfo->deviceID,deviceState);
			if(deviceState == On)//�����Ŵŵ�״̬��ʵ�ʵ��෴
			{
				Out_Of_Power_Start(OUT_OF_POWER_TIME);
			}
		}
		else
		{
			//���յ����أ��ر��ط�����
            zblist_remove(devInfo->nwkAddr,devInfo->endpoint);
			//���ؽڵ�Ŀ���״̬�仯�ϱ�
            SRPC_SwitchStateInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState,devInfo->onlineDevRssi);
		}
		zbSoc_ProcessEvent(devInfo,deviceState);
	}
	
	*/

	log_debug("zbSoc_MasterControlReportResolve--\n");
	return 0;
}

//��ȡ�����ϱ�
uint8_t zbSoc_MasterControlReadRspResolve(hostCmd *cmd,epInfo_t *epInfo)
{
	uint8_t mCount =0;
	uint8_t datalength = 0;
	uint8_t deviceState[16] = {0};
	epInfo_t *devInfo = NULL;
//	epInfo_t mEpInfo;
	
	if(epInfo == NULL)
		return -1;

	log_debug("zbSoc_MasterControlReadRspResolve--\n");

//	memcpy(&mEpInfo,epInfo,sizeof(epInfo_t));
	
    cmdGet8bitVal(cmd, &datalength);

	if (datalength <= MASTER_CONTROL_VIRTUAL_DEVICE_SIZE)
		cmdGetStringVal(cmd,deviceState,datalength);
	else
		return -1;

	//�忨ȡ���豸
	
	devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_VIRTUAL_DEVICE_POWERSWITCH);
	if(devInfo != NULL)
	{
		//�����豸RSSI
		devInfo->onlineDevRssi = epInfo->onlineDevRssi;
		//�����豸״̬
		vdevListSetPowerSwitchState(devInfo,deviceState[0]);
		
		SRPC_PowerSwitchCtrlInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState[0],devInfo->onlineDevRssi);
		//zbSoc_ProcessEvent(devInfo,deviceState[0]);
	}
	
	devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_VIRTUAL_DEVICE_DOORCONTACT);
	if(devInfo != NULL)
	{
		//�����豸RSSI
		devInfo->onlineDevRssi = epInfo->onlineDevRssi;
		vdevListSetAlarmState(devInfo,deviceState[1]);
		
		SRPC_ComAlarmStateInd(devInfo->IEEEAddr,devInfo->endpoint,devInfo->deviceID,deviceState[1]);
//		zbSoc_ProcessEvent(devInfo,ZCL_DATATYPE_UINT8,&deviceState[1]);
	}

	for(mCount =0;mCount<MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH_SIZE;mCount++)
	{
		devInfo = vdevListGetDeviceByIeeeEp(epInfo->IEEEAddr,MASTER_CONTROL_VIRTUAL_DEVICE_SWITCH[mCount]);
		if(devInfo != NULL)
		{
			//�����豸RSSI
			devInfo->onlineDevRssi = epInfo->onlineDevRssi;
			vdevListSetSwitchState(devInfo,deviceState[mCount+2]);
		
			SRPC_SwitchStateInd(devInfo->IEEEAddr,devInfo->endpoint,deviceState[mCount+2],devInfo->onlineDevRssi);
//			zbSoc_ProcessEvent(devInfo,ZCL_DATATYPE_UINT8,&deviceState[mCount+2]);
		}
	}
	log_debug("zbSoc_MasterControlReadRspResolve--\n");
	return 0;
}