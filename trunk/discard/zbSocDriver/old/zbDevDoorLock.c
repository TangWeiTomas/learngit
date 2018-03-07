#include "zbDevDoorLock.h"
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

#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "errorCode.h"
#include "interface_devicelist.h"
#include "LogUtils.h"
#include "Polling.h"
#include "PackageUtils.h"
#include "interface_srpcserver.h"
#include "One_key_match.h"
#include "Out_Of_Power.h"
#include "zbSocMasterControl.h"

typedef enum{
	ZBDL_CLOSE = 00,
	ZBDL_OPENCLOSE = 01,
	ZBDL_OPEN	   = 255
}zbDevDoorLock_OPEN_Types;


#define ZBDL_MSG_SOC	0XAA


typedef struct
{
	tu_evtimer_t *zbDevDoorLockTimer;
	uint16 nwkaddr;
	uint8 endpoint;
}zbDevDoorLockTimer_t;

zbDevDoorLockTimer_t zbDevDoorLockTime;

int zbDevDoorLock_SetDoorOnOff(hostCmd *cmd,zbDevDoorLock_OPEN_Types status);

void zbDevDoorLock_virtualDeviceRegister(epInfoExtended_t *epInfoEx)
{
	epInfo_t *epinfo;
	uint32_t context = 0;
    epInfo_t newData;
	epInfo_t virtualDevice;//�����豸

	if(epInfoEx->epInfo->deviceID == ZB_DEV_LEVEL_DOORLOCK)
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
				if((memcmp(newData.IEEEAddr,epinfo->IEEEAddr,8) == 0)&&(epinfo->deviceID != ZB_DEV_LEVEL_DOORLOCK))
				{
					//devListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
					if(epinfo->deviceID == ZB_DEV_ONOFF_DOORLOCK)//�忨ȡ���豸����
					{
						virtualDevice.endpoint = 9;
						virtualDevice.deviceID = ZB_DEV_ONOFF_DOORLOCK;
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
				if((memcmp(newData.IEEEAddr,epinfo->IEEEAddr,8) == 0)&&(epinfo->deviceID != ZB_DEV_LEVEL_DOORLOCK))
				{
					devListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
					vdevListRemoveDeviceByIeeeEp(epinfo->IEEEAddr,epinfo->endpoint);
				}
			}
			
			//��������豸
			memcpy(&virtualDevice,&newData,sizeof(epInfo_t));
			epInfoEx->epInfo = &virtualDevice;

			//1.�����������
			virtualDevice.endpoint = 9;
			virtualDevice.deviceID = ZB_DEV_ONOFF_DOORLOCK;
			devListAddDevice(&virtualDevice);
			zbSoc_DevRegisterReporting(epInfoEx);
		}
	}
	else
	{
		zbSoc_DevRegisterReporting(epInfoEx);
	}
}


static void zbDevDoorLock_SetCmd(uint16 dstAddr, uint8 endpoint, uint8 addrMode, uint8 len,uint8* buf)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_DOORLOCK_UART_MSG);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, len);//���ݳ���
	cmdSetStringVal(&cmd,buf,len);//����
   
    zbMakeMsgEnder(&cmd);
    
	usleep(1000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}


uint8 zbDevDoorLock_SetOnOffState(epInfo_t *epInfo,uint8 switchcmd)
{
	log_debug("zbDevDoorLock_SetOnOffState++\n");
	epInfo_t mEpInfo;
	epInfo_t *mepinfo;
	uint32_t context = 0;
	if(epInfo == NULL)
		return -1;
	
	memcpy(&mEpInfo,epInfo,sizeof(epInfo_t));

	//�жϵ�ǰ���豸��0X010A�����豸
	//epInfo = devListGetDeviceByIeeeDeviceID(mEpInfo.IEEEAddr,ZB_DEV_LEVEL_DOORLOCK);
	epInfo = devListGetDeviceByIeeeEp(mEpInfo.IEEEAddr,0x08);
	
	if(epInfo!=NULL) //��0X010A����
	{
		if(epInfo->deviceID == ZB_DEV_LEVEL_DOORLOCK)
		{
			hostCmd cmd;
			memset(&cmd,0,sizeof(cmd));
			zbDevDoorLock_SetDoorOnOff(&cmd,ZBDL_OPENCLOSE);
			zbDevDoorLock_SetCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
		}
		else
		{
			zbSoc_SetGenOnOffState(switchcmd,mEpInfo.nwkAddr,mEpInfo.endpoint,afAddr16Bit);
		}
	}
	else	//û�ҵ�0X010A����
	{
		zbSoc_SetGenOnOffState(switchcmd,mEpInfo.nwkAddr,mEpInfo.endpoint,afAddr16Bit);
	}
	log_debug("zbSoc_MasterControlSetOnOffState--\n");
	return 0;
}


/*******************************************************************************************
											��ά��������
*******************************************************************************************/

int zbDevDoorLock_SetDoorOnOff(hostCmd *cmd,zbDevDoorLock_OPEN_Types status)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, 0x01);

	//����-�׷�
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//���Ų���
	cmdSet8bitVal(cmd, 0x01);

	cmdSetCheckSum(cmd);
}

//0x07
int zbDevDoorLock_getDoorStatus(hostCmd *cmd)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, 0x07);

	//����-�׷�
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//���������Բ�������
	cmdSet8bitVal(cmd, 0x00);

	cmdSetCheckSum(cmd);
}

void zbDevDoorLock_getstatus_timerhandler(void *args)
{
	log_debug("zbDevDoorLock_getstatus_timerhandler++\n");
	zbDevDoorLockTimer_t *zbtimer = (zbDevDoorLockTimer_t*)args;
	hostCmd cmd;
	memset(&cmd,0,sizeof(cmd));
	zbDevDoorLock_getDoorStatus(&cmd);
	zbDevDoorLock_SetCmd(zbtimer->nwkaddr,zbtimer->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	tu_evtimer_free(zbtimer->zbDevDoorLockTimer);
	log_debug("zbDevDoorLock_getstatus_timerhandler++\n");
}

//����״̬�ϱ�����

int zbDevDoorLock_StatusReportProcess(epInfo_t *epInfo,hostCmd *cmd,uint8 rssi)
{
	uint16 attrID;
	uint8 dataType;
	uint8 sof;
	uint8 datalen;
	uint8 pkglen;
	uint8 cmdtype;
	epInfo_t mEpInfo;
	
	if(epInfo == NULL)
		return -1;
		
	log_debug("zbDevDoorLock_StatusReportProcess++\n");
	
	memcpy(&mEpInfo,epInfo,sizeof(epInfo_t));

	cmdGet16bitVal_lh(cmd, &attrID);
    cmdGet8bitVal(cmd, &dataType);

	epInfo = vdevListGetDeviceByIeeeEp(mEpInfo.IEEEAddr,0x09);

	if(epInfo == NULL)
		return;

	//���յ����أ��ر��ط�����
    zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

	if((attrID == ATTRID_BASIC_LOCK_UNLOCK) && (dataType == ZCL_DATATYPE_OCTET_STR))
	{
		cmdGet8bitVal(cmd, &datalen);//�����ܳ���
		cmdGet8bitVal(cmd, &sof);//��ʵ��־
		cmdGet8bitVal(cmd, &pkglen);//��ʵ��־
		cmdGet8bitVal(cmd, &cmdtype);//��ʵ��־
		
		switch(cmdtype)
		{
			case 0x1c://ʵʱ���ż�¼
			{
				uint8 status = 0x00;
				uint8 area = 0;		//����
				uint8 floors = 0;	//¥��
				uint8 floor = 0	;	//¥��
				uint8 room = 0	;	//����
				uint8 rooms = 0	;	//�׷�
				uint8 cardType = 0 ; //��Ƭ����
				
				cmdGet8bitVal(cmd, &area);
				cmdGet8bitVal(cmd, &floors);
				cmdGet8bitVal(cmd, &floor);
				cmdGet8bitVal(cmd, &room);
				cmdGet8bitVal(cmd, &rooms);
				cmdGet8bitVal(cmd, &cardType);
				
				if((cardType&0x1F) == 0x1c)//��������Ϊ��¼��
				{
					status = 0x00;
				}
				else
				{
					status = 0x01;
				}

				SRPC_DoorLockCtrlInd(epInfo->IEEEAddr,epInfo->endpoint,status,rssi);
        		//devState_updateSwitchVal(epInfo,status);
        		vdevListSetDoorState(epInfo,status);
        		zbSoc_ProcessEvent(epInfo,dataType,&status);
			
				#if 0
				zbDevDoorLockTime.endpoint = 0x08;
				zbDevDoorLockTime.nwkaddr = mEpInfo.nwkAddr;
				
				zbDevDoorLockTime.zbDevDoorLockTimer = tu_evtimer_new(main_base_event_loop);
				tu_set_evtimer(zbDevDoorLockTime.zbDevDoorLockTimer,10000,false,zbDevDoorLock_getstatus_timerhandler,&zbDevDoorLockTime);
				#endif
        	}
			break;
			case 0x08://����״̬
			{
				uint8 recive[25] = {0};
				uint8 doorStatus = 0;
				uint8 status = 0;
				cmdGetStringVal(cmd,recive,pkglen-3);
				doorStatus = recive[16];
				if(doorStatus & 0x20)
				{
					status = 0x01;	
				}
				else
				{
					status = 0x00;
				}
				
				SRPC_DoorLockCtrlInd(epInfo->IEEEAddr,epInfo->endpoint,status,rssi);
        		//devState_updateSwitchVal(epInfo,status);
        		vdevListSetDoorState(epInfo,status);
        		zbSoc_ProcessEvent(epInfo,dataType,&status);
			}
		}
	}
	log_debug("zbDevDoorLock_StatusReportProcess--\n");
	return 0;
}


