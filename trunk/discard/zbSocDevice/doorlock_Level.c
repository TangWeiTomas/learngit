/***********************************************************************************
 * �� �� ��   : doorlock_Level.c
 * �� �� ��   : Edward
 * ��������   : 2016��10��12��
 * �ļ�����   : ��ά����,������Ϣ͸�����ݴ���
 * ��Ȩ˵��   : Copyright (c) 2008-2016   ������ѩ���� �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/

#include "doorlock_Level.h"
#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"

#include "zbSocPrivate.h"
#include "interface_srpcserver_defs.h"


typedef enum{
	ZBDL_CLOSE = 0x00,
	ZBDL_OPENCLOSE = 0x01,
	ZBDL_OPEN	   = 0xff
}Level_Open_Types;

typedef enum
{
	DOOR_CLOSE 		 = 0X00,
	DOOR_OPEN  		 = 0X01,
	DOOR_ALWAYS_OPEN = 0X02,
}Level_DoorState_Type;

typedef enum
{
	DOOR_INFO	= 0x08,
	DOOR_ALARM_REPORT = 0x0B,
	DOOR_USER_LIST_INFO = 0X11,
	DOOR_OPEN_MSG = 0X1C,
	DOOR_CMD_STATE_REPORT = 0X1D
}Level_Option_Types;

typedef struct
{
	tu_evtimer_t *doorLevelTimer;
	epInfo_t epInfo;
}Level_time_Types_t;

#define DOOR_CMD_USR_MNG	0x0e


#define ZBDL_MSG_SOC	0XAA

//��ŷ������·�����������״̬����
static Level_DoorState_Type doorStateFromServer = DOOR_CLOSE;

/*****************************************************************************
 * �� �� ��  : doorLevel_setOnOff
 * �� �� ��  : Edward
 * ��������  : 2016��10��12��
 * ��������  : ����level������������
 * �������  : hostCmd *cmd             ���ݻ�����
               Level_Open_Types status  ��������
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static void doorLevel_setOnOffCmd(hostCmd *cmd,Level_Open_Types status)
{
	uint8_t cmdOpt = status;
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, 0x01);

	//����-�׷�
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	if(status == 0x02)
		cmdOpt = 0xff; //����

	//���Ų���
	cmdSet8bitVal(cmd, cmdOpt);

	cmdSetCheckSum(cmd);
}

/*****************************************************************************
 * �� �� ��  : doorLevel_getInfo
 * �� �� ��  : Edward
 * ��������  : 2016��10��12��
 * ��������  : ���ɻ�ȡ������Ϣָ��
 * �������  : hostCmd *cmd  ���ݻ�����
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static void doorLevel_getInfoCmd(hostCmd *cmd)
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

static void doorLevel_settimeCmd(hostCmd *cmd,uint8_t years,uint8_t month,uint8_t dates,uint8_t hours,uint8_t minute,uint8_t seconds,uint8_t week)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x12);
	cmdSet8bitVal(cmd, 0x0a);

	//����-�׷�
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	//��������1
	cmdSet8bitVal(cmd, 0x00);

	//������ʱ��
	cmdSet8bitVal(cmd, years);
	cmdSet8bitVal(cmd, month);
	cmdSet8bitVal(cmd, dates);
	cmdSet8bitVal(cmd, hours);
	cmdSet8bitVal(cmd, minute);
	cmdSet8bitVal(cmd, seconds);
	cmdSet8bitVal(cmd, week);

	cmdSetCheckSum(cmd);

}

static void doorLevel_getRegisterInfoCmd(hostCmd *cmd,uint8_t registerType)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, 0x10);

	//����-�׷�
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//�б�����
	cmdSet8bitVal(cmd, registerType);

	cmdSetCheckSum(cmd);
}

static int doorLevel_SetUsrMngCmd(hostCmd *cmd,uint8_t usrType,uint8_t optType,uint16_t usrid,uint8_t *passwd)
{
	uint8_t mUsrtType = 0;
	uint8_t mData[4] = {0};

	
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x13);
	cmdSet8bitVal(cmd, 0x0E);

	//����-�׷�
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);


	if(optType <0 && optType > 3)
		return -1;

	switch(usrType)
	{
		case 0x00://����ID
		{
			mUsrtType = 0x00;
			mUsrtType |= optType;
			cmdSet8bitVal(cmd, mUsrtType);
			switch(optType)
			{
				case 0x00://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x01://��������
				{
					cmdSet16bitVal(cmd, usrid);
					//cmdSetStringVal(cmd,passwd,4);

					uint8_t pwd = passwd[0]<<4 | passwd[1];
					cmdSet8bitVal(cmd,pwd);
					pwd = passwd[2]<<4 | passwd[3];
					cmdSet8bitVal(cmd,pwd);
					pwd = passwd[4]<<4 | passwd[5];
					cmdSet8bitVal(cmd,pwd);
					pwd = passwd[6]<<4 | passwd[7];
					cmdSet8bitVal(cmd,pwd);			
				}
				break;
				case 0x02://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x03://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				default:
					return -1;
			}
			
		}
		break;
		case 0x01://��ƬID
		{
			mUsrtType = 0x04;
			mUsrtType |= optType;
			cmdSet8bitVal(cmd, mUsrtType);
			switch(optType)
			{
				case 0x00://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x01://��������
				{
					//cmdSet16bitVal(cmd, usrid);
					//cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x02://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x03://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				default:
					return -1;
			}
		}
		break;
		case 0x02://ָ��ID
		{
			mUsrtType = 0x08;
			mUsrtType |= optType;
			cmdSet8bitVal(cmd, mUsrtType);
			switch(optType)
			{
				case 0x00://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x01://��������
				{
					//cmdSet16bitVal(cmd, usrid);
					//cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x02://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x03://����ɾ��
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				default:
					return -1;
			}
		}
		break;
		default:
			return -1;
	}

	//����
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	cmdSetCheckSum(cmd);
	return 0;
}


static void doorLevel_usrMngCmd(hostCmd *cmd,uint8_t usrType,uint8_t optType,uint32_t usrid,uint8_t *passwd)
{

	uint8_t usr = 0;
	uint8_t opt = 0;
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x13);
	cmdSet8bitVal(cmd, 0x0e);

	//����-�׷�
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//����1
	cmdSet8bitVal(cmd, usrType&0x0c);
	cmdSet8bitVal(cmd, optType&0x03);

	usr = usrType&0x0c;
	opt = optType&0x03;
	
	cmdSetCheckSum(cmd);
}

/*****************************************************************************
 * �� �� ��  : doorLevel_SendSerialCmd
 * �� �� ��  : Edward
 * ��������  : 2016��10��12��
 * ��������  : ������ά͸������
 * �������  : uint16_t dstAddr  Ŀ���ַ
               uint8_t endpoint  Ŀ��˿ں�
               uint8_t addrMode  ��������
               uint8_t len       ���ݳ���
               uint8_t* buf      ����
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static void doorLevel_SendSerialCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t len,uint8_t* buf)
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


static void doorLevel_doorInfoInd(epInfo_t *epInfo,uint8_t *times,uint16_t dversion,uint16_t sversion,uint8_t doorbellstate,uint8_t opentype)
{
	log_debug("doorLevel_doorInfoInd");
	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_INFO_IND);
    //D2-D9 ���ƽ��
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 ���ƽ��
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11-D17 ���ƽ��
    cmdSetStringVal(&cmd,times,7);
	//D18-D19 ����Ӳ���汾
	cmdSet16bitVal(&cmd,dversion);
	//D20-D21 ��������汾
	cmdSet16bitVal(&cmd,sversion);
	//D22 ����״̬
	cmdSet8bitVal(&cmd,doorbellstate);
	//D23 ֧�ֿ�����ʽ
	cmdSet8bitVal(&cmd,opentype);
    
    makeMsgEnder(&cmd);
    
    cmdMsgSend(cmd.data,cmd.idx);
}


/*****************************************************************************
 * �� �� ��  : doorLevel_doorStateInd
 * �� �� ��  : Edward
 * ��������  : 2016��10��13��
 * ��������  : ��������״̬�ϱ�
 * �������  : epInfo_t *epInfo  �ڵ���Ϣ
               uint8_t state       ��ǰ״̬
 * �������  : ��
 * �� �� ֵ  : static
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static void doorLevel_doorStateInd(epInfo_t *epInfo,uint8_t state)
{
	log_debug("doorLevel_doorStateInd++\n");
	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_STATUS_IND);
    //D2-D9 ���ƽ��
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 ���ƽ��
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 ���ƽ��
    cmdSet8bitVal(&cmd,state);
    //D12 �ź�����
    cmdSet8bitVal(&cmd,epInfo->onlineDevRssi);
    makeMsgEnder(&cmd);
    
    cmdMsgSend(cmd.data,cmd.idx);
}

//����������־�ϱ�
static void doorLevel_realRecordInd(epInfo_t *epInfo,uint8_t state,uint8_t cardtype,uint8_t *uid,uint8_t *times)
{
	log_debug("doorLevel_realRecordInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_RECORD_IND);
    //D2-D9 ���ƽ��
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 ���ƽ��
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 ����״̬
    cmdSet8bitVal(&cmd,state);
    //D12 ��������
    cmdSet8bitVal(&cmd,cardtype);
    //D13-D16 UID0-UID3
    cmdSetStringVal(&cmd,uid,4);
    //D17-D22 ����ʱ��
    cmdSetStringVal(&cmd,times,6);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
   
}


//��������״̬�ϱ�
static void doorLevel_alarmPowerInd(epInfo_t *epInfo,uint8_t doorPowerState,uint8_t wirelessPowerState)
{
	log_debug("doorLevel_alarmPowerInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_POWER_IND);
    //D2-D9 ���ƽ��
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 ���ƽ��
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 ������ѹ
    cmdSet8bitVal(&cmd,doorPowerState);
    //D12 ���ߵ�ѹ
    cmdSet8bitVal(&cmd,wirelessPowerState);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static void doorLevel_alarmOperationInd(epInfo_t *epInfo,uint8_t alarmType,uint8_t *times)
{
	log_debug("doorLevel_alarmOperationInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_ALARMOPT_IND);
    //D2-D9 ���ƽ��
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 ���ƽ��
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 �Ƿ���������
    cmdSet8bitVal(&cmd,alarmType);
    //D12-D18 �Ƿ�����ʱ��
    cmdSetStringVal(&cmd,times,7);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static void doorLevel_registerInfoInd(epInfo_t *epInfo,uint8_t regType,uint8_t*uid,uint8_t udisize)
{
	log_debug("doorLevel_registerInfoInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_GET_LEVEL_DOOR_REG_IND);
    //D2-D9 ���ƽ��
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 ���ƽ��
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 ����
    cmdSet8bitVal(&cmd,regType);
	//UID�Ǽ���Ϣ
    cmdSet8bitVal(&cmd,udisize);
    //D12-D18 �Ƿ�����ʱ��
    cmdSetStringVal(&cmd,uid,udisize);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}

static void doorLevel_UsrMngStateInd(epInfo_t *epInfo,uint8_t state)
{
	log_debug("doorLevel_alarmOperationInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_ADD_LEVEL_DOOR_USR_IND);
    //D2-D9 ���ƽ��
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 ���ƽ��
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 �Ƿ���������
    cmdSet8bitVal(&cmd,state);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


//0x1c��������ʵʱ��¼
static void doorLevel_timerGetInfoHandle(void *args)
{
	ASSERT(args != NULL);
	log_debug("doorLevel_timerGetInfoHandle++\n");

	Level_time_Types_t *Leveltimer = (Level_time_Types_t*)args;
	doorLevel_getInfoReq(&(Leveltimer->epInfo));
	
	tu_evtimer_free(Leveltimer->doorLevelTimer);
	
	free(Leveltimer);
	
	log_debug("doorLevel_timerGetInfoHandle++\n");
}

static void doorLevel_timerGetInfo(epInfo_t *epInfo)
{
	ASSERT(epInfo != NULL);
	
	Level_time_Types_t *LevelTime = malloc(sizeof(Level_time_Types_t));
	
	if (LevelTime == NULL)
	{
		log_err("malloc failed\n");
		return;
	}
	
	memcpy(&(LevelTime->epInfo),epInfo,sizeof(epInfo_t));
	
	LevelTime->doorLevelTimer = tu_evtimer_new(main_base_event_loop);
	tu_set_evtimer(LevelTime->doorLevelTimer,DEVICE_LIWEI_DOOR_QUERY_TIME,false,doorLevel_timerGetInfoHandle,LevelTime);
}

static void doorLevel_timerDoorStateIndHandle(void *args)
{
	ASSERT(args != NULL);
	log_debug("doorLevel_timerDoorStateIndHandle++\n");

	Level_time_Types_t *Leveltimer = (Level_time_Types_t*)args;
	//doorLevel_getInfoReq(&(Leveltimer->epInfo));

	doorLevel_doorStateInd(&Leveltimer->epInfo,DOOR_CLOSE);
	
	tu_evtimer_free(Leveltimer->doorLevelTimer);
	
	free(Leveltimer);
	
	log_debug("doorLevel_timerDoorStateIndHandle++\n");
}

//��ʱȥ�ϱ���������״̬
static void doorLevel_timerDoorStateInd(epInfo_t *epInfo)
{
	ASSERT(epInfo != NULL);
	
	Level_time_Types_t *LevelTime = malloc(sizeof(Level_time_Types_t));
	
	if (LevelTime == NULL)
	{
		log_err("malloc failed\n");
		return;
	}
	
	memcpy(&(LevelTime->epInfo),epInfo,sizeof(epInfo_t));
	
	LevelTime->doorLevelTimer = tu_evtimer_new(main_base_event_loop);
	tu_set_evtimer(LevelTime->doorLevelTimer,DEVICE_LIWEI_DOOR_QUERY_TIME,false,doorLevel_timerDoorStateIndHandle,LevelTime);
}

/*****************************************************************************
 * �� �� ��  : doorLevel_precessDoorState
 * �� �� ��  : Edward
 * ��������  : 2016��11��29��
 * ��������  : ����������ǰ״̬�߼�
 * �������  : uint8_t cardType  ������������
 * �������  : ��
 * �� �� ֵ  : static
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static uint8_t doorLevel_precessDoorState(epInfo_t *epInfo,uint8_t doorOperateType,uint8_t doorOpenType)
{

	log_debug("doorLevel_precessDoorState++\n");
	uint8_t doorstate = DOOR_CLOSE;

	if(doorOperateType == 0x1c)//��¼��
	{
		doorstate = DOOR_CLOSE;//�Źر�״̬
	}
	else
	{
		//��ȡ����״̬
		switch(doorStateFromServer)
		{
			case DOOR_CLOSE://����
			{
				doorstate = DOOR_CLOSE;
			}	
			break;
			case DOOR_ALWAYS_OPEN://����
			{
				doorstate = DOOR_ALWAYS_OPEN;//����
			}
			break;
			case DOOR_OPEN://��ͨ����
			{
				doorstate = DOOR_OPEN;//����
				//������ʱ��ȡ����״̬
				//doorLevel_timerGetInfo(epInfo);
				//�Զ�ȥ�ϱ�����״̬
				doorLevel_timerDoorStateInd(epInfo);
			}
			break;
			default:
			{
				doorstate = DOOR_CLOSE;//����
				//������ʱ��ȡ����״̬
				doorLevel_timerGetInfo(epInfo);
			}
			break;
		}
	}
	log_debug("doorstate = %d\n",doorstate);
	log_debug("doorLevel_precessDoorState++\n");
	return doorstate;
}

/*****************************************************************************
 * �� �� ��  : doorLevel_realRecordReport
 * �� �� ��  : Edward
 * ��������  : 2016��10��13��
 * ��������  : ����������־�ϱ�-0x1C
 * �������  : epInfo_t *epInfo  �ڵ���Ϣ
               hostCmd *cmd      ����͸������
 * �������  : ��
 * �� �� ֵ  : static
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static void doorLevel_realRecordReport(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t area = 0; 	//����
	uint8_t unit = 0;		//¥����Ԫ
	uint8_t floor = 0 ;	//¥��
	uint8_t room = 0	;	//����
	uint8_t suite = 0 ;	//�׷�
	uint8_t cardType = 0 ; //��Ƭ����
	uint8_t uid[4] = {0};	//��Ƭ��Ϣ
	uint8_t year = 0;		//����ʱ��
	uint8_t month = 0;	//
	uint8_t dates = 0;	//
	uint8_t hours = 0;
	uint8_t minute = 0;
	uint8_t power = 0;	//��ѹ״̬

	uint8_t doorstate = 0;
	uint8_t doorOperateType = 0; //��������
	uint8_t times[6] = {0};
	uint8_t doorPowerAlarm = 0;
	uint8_t wiressPowerAlarm = 0;

	ASSERT((epInfo != NULL) && (cmd != NULL));
	
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//������Ϣ
	cmdGet8bitVal(cmd, &cardType);
	cmdGet8bitVal(cmd, &uid[0]);
	cmdGet8bitVal(cmd, &uid[1]);
	cmdGet8bitVal(cmd, &uid[2]);
	cmdGet8bitVal(cmd, &uid[3]);
	cmdGet8bitVal(cmd, &year);
	cmdGet8bitVal(cmd, &month);
	cmdGet8bitVal(cmd, &dates);
	cmdGet8bitVal(cmd, &hours);
	cmdGet8bitVal(cmd, &minute);
	cmdGet8bitVal(cmd, &power);

	doorOperateType = (cardType &0x1F);

	//��������״̬�߼�
	doorstate = doorLevel_precessDoorState(epInfo,doorOperateType,cardType & 0x40);
	
	if(year & 0x80)//�Ƿ�ʱ��
	{
		times[0] = 0;//��Ч
	}
	else
	{
		times[0] = 0x01;
		times[1] = year & 0x7F;
		times[2] = month & 0x0F;
		times[3] = dates & 0x1F;
		times[4] = hours & 0x1F;
		times[5] = minute & 0x3F;
	}

	if(month & 0x80)
	{
		doorPowerAlarm = 0x01;//����״̬
	}
	else
	{
		doorPowerAlarm = 0x00;
	}
	
	if(power & 0x80)
	{
		wiressPowerAlarm = 0x01;//����״̬
	}
	else
	{
		wiressPowerAlarm = 0x00;
	}
	
	doorLevel_doorStateInd(epInfo,doorstate);
	doorLevel_realRecordInd(epInfo,doorstate,doorOperateType,uid,times);
	doorLevel_alarmPowerInd(epInfo,doorPowerAlarm,wiressPowerAlarm);

	vdevListSetDoorState(epInfo,doorstate);
	//devState_updateSwitchVal(epInfo,doorstate);
	
/*
	if((cardType&0x1F) == 0x1c)//��������Ϊ��¼��
	{
		status = 0x00;
	}
	else
	{
		status = 0x01;
	}

	SRPC_DoorLockCtrlInd(epInfo->IEEEAddr,epInfo->endpoint,status,rssi);
	devState_updateSwitchVal(epInfo,status);
	zbSoc_ProcessEvent(epInfo,dataType,&status);
*/

#if 0
	zbDevDoorLockTime.endpoint = 0x08;
	zbDevDoorLockTime.nwkaddr = mEpInfo.nwkAddr;

	zbDevDoorLockTime.zbDevDoorLockTimer = tu_evtimer_new(main_base_event_loop);
	tu_set_evtimer(zbDevDoorLockTime.zbDevDoorLockTimer,10000,false,zbDevDoorLock_getstatus_timerhandler,&zbDevDoorLockTime);
#endif

}

/*****************************************************************************
 * �� �� ��  : doorLevel_doorInfoReport
 * �� �� ��  : Edward
 * ��������  : 2016��10��13��
 * ��������  : ��ǰ��������Ϣ--0x08
 * �������  : epInfo_t *epInfo  �ڵ���Ϣ
               hostCmd *cmd      ͸����������
 * �������  : ��
 * �� �� ֵ  : static
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static void doorLevel_doorInfoReport(epInfo_t *epInfo,hostCmd *cmd)
{
#if 0
	uint8_t recive[25] = {0};
	uint8_t doorStatus = 0;
	uint8_t status = 0;
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
	devState_updateSwitchVal(epInfo,status);
	zbSoc_ProcessEvent(epInfo,dataType,&status);
#endif

	uint8_t area = 0; 	//����
	uint8_t unit = 0;		//¥����Ԫ
	uint8_t floor = 0 ;	//¥��
	uint8_t room = 0	;	//����
	uint8_t suite = 0 ;	//�׷�
	
	uint8_t year = 0;		//
	uint8_t month = 0;	//
	uint8_t dates = 0;	//
	uint8_t hours = 0;
	uint8_t minute = 0;
	uint8_t seconds = 0;
	uint8_t week = 0;
	uint16_t dverson = 0;
	uint16_t sversion = 0;
	uint8_t times[7] = {0};

	uint8_t doorState = 0;
	uint8_t doorPowerAlarm = 0;
	uint8_t wiressPowerAlarm = 0;
	uint8_t dooropentype = 0;
	uint8_t doorbell = 0;

	ASSERT((epInfo != NULL) && (cmd != NULL));
	
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//��ȡ����ʱ��
	cmdGet8bitVal(cmd, &year);
	cmdGet8bitVal(cmd, &month);
	cmdGet8bitVal(cmd, &dates);
	cmdGet8bitVal(cmd, &hours);
	cmdGet8bitVal(cmd, &minute);
	cmdGet8bitVal(cmd, &seconds);
	cmdGet8bitVal(cmd, &week);
	
	times[0] = year & 0x7f;
	times[1] = month & 0x0f;
	times[2] = dates & 0x1f;
	times[3] = hours & 0x1f;
	times[4] = minute & 0x3f;
	times[5] = seconds & 0x3f;
	times[6] = week & 0x07;

	//���Ӳ���汾
	cmdGet16bitVal(cmd,&dverson);
	cmdGet16bitVal(cmd,&sversion);

	cmdGet8bitVal(cmd, &doorState);

	if(doorState & 0x80)
	{
		doorPowerAlarm = 0x01;//����״̬
	}
	else
	{
		doorPowerAlarm = 0x00;
	}
	
	if(doorState & 0x40)
	{
		wiressPowerAlarm = 0x01;//����״̬
	}
	else
	{
		wiressPowerAlarm = 0x00;
	}
	
	//��������ʽ
	dooropentype = doorState&0x0f; 

 	//����״̬
	if(doorState & 0x10)
		doorbell = 0x01;
	else
		doorbell = 0x00;

	//����״̬
	if(doorState & 0x20)
		doorState = 0x01;
	else
		doorState = 0x00;

	doorLevel_doorStateInd(epInfo,doorState);
	doorLevel_alarmPowerInd(epInfo,doorPowerAlarm,wiressPowerAlarm);
	doorLevel_doorInfoInd(epInfo,times,dverson,sversion,doorbell,dooropentype);
	vdevListSetDoorState(epInfo,doorState);

	//devState_updateSwitchVal(epInfo,doorState);
}

/*****************************************************************************
 * �� �� ��  : doorLevel_alarmOperationReport
 * �� �� ��  : Edward
 * ��������  : 2016��10��13��
 * ��������  : �Ƿ����������ϱ�-0X0B
 * �������  : epInfo_t *epInfo  �ڵ���Ϣ
               hostCmd *cmd      ����͸������
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
void doorLevel_alarmOperationReport(epInfo_t *epInfo,hostCmd *cmd)
{

	ASSERT((epInfo != NULL) && (cmd != NULL));
	uint8_t area = 0; 	//����
	uint8_t unit = 0;		//¥����Ԫ
	uint8_t floor = 0 ;	//¥��
	uint8_t room = 0	;	//����
	uint8_t suite = 0 ;	//�׷�

	uint8_t alarmOptType = 0;
	uint8_t year = 0;		
	uint8_t month = 0;	
	uint8_t dates = 0;	
	uint8_t hours = 0;
	uint8_t minute = 0;
	uint8_t seconds = 0;
	uint8_t week = 0;
	uint8_t times[7] = {0};

	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//��������
	cmdGet8bitVal(cmd, &alarmOptType);
	
	alarmOptType = alarmOptType&0x07;

	//��ȡ����ʱ��
	cmdGet8bitVal(cmd, &year);
	cmdGet8bitVal(cmd, &month);
	cmdGet8bitVal(cmd, &dates);
	cmdGet8bitVal(cmd, &hours);
	cmdGet8bitVal(cmd, &minute);
	cmdGet8bitVal(cmd, &seconds);
	cmdGet8bitVal(cmd, &week);
	times[0] = year & 0x7f;
	times[1] = month & 0x0f;
	times[2] = dates & 0x1f;
	times[3] = hours & 0x1f;
	times[4] = minute & 0x3f;
	times[5] = seconds & 0x3f;
	times[6] = week & 0x07;
	
	doorLevel_alarmOperationInd(epInfo,alarmOptType,times);
	
}

static void doorLevel_getRegisterInfoReport(epInfo_t *epInfo,hostCmd *cmd)
{
	ASSERT((epInfo != NULL) && (cmd != NULL));

	uint8_t area = 0; 	//����
	uint8_t unit = 0;		//¥����Ԫ
	uint8_t floor = 0 ;	//¥��
	uint8_t room = 0	;	//����
	uint8_t suite = 0 ;	//�׷�

	
	uint8_t uidsize = 0;
	uint8_t regType = 0;
	uint8_t *uid = NULL;
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//��������
	cmdGet8bitVal(cmd, &regType);

	if(cmd->size >= (cmd->idx - 2))
	{
		uidsize = (cmd->size - cmd->idx - 2);
	}

	uid = &(cmd->data[cmd->idx]);
	
	doorLevel_registerInfoInd(epInfo,regType,uid,uidsize);
	
}

void doorLevel_allSetCmdReport(epInfo_t *epInfo,hostCmd *cmd)
{
	ASSERT((epInfo != NULL) && (cmd != NULL));
	
	uint8_t area = 0; 	//����
	uint8_t unit = 0; 	//¥����Ԫ
	uint8_t floor = 0 ;	//¥��
	uint8_t room = 0	;	//����
	uint8_t suite = 0 ;	//�׷�


	uint8_t cmdid = 0;
	uint8_t result = 0;
	
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	cmdGet8bitVal(cmd, &cmdid);
	cmdGet8bitVal(cmd, &result);

	if(cmdid == DOOR_CMD_USR_MNG)
	{
		result = result==0?0:1;
		doorLevel_UsrMngStateInd(epInfo,result);
	}
	
}

//////////////////////////////////////////////////////////////////////////////
//							����ӿ�
//////////////////////////////////////////////////////////////////////////////


void doorLevel_setOnOffReq(epInfo_t *epInfo,uint8_t optype)
{
	log_debug("doorLevel_setOnOff++\n");
	ASSERT(epInfo != NULL);

	hostCmd cmd;

	memset(&cmd,0,sizeof(cmd));
	
	doorStateFromServer = optype;

	//���
	doorLevel_setOnOffCmd(&cmd,optype);

	//��������
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	
	log_debug("doorLevel_setOnOff--\n");
}

void doorLevel_getInfoReq(epInfo_t *epInfo)
{
	log_debug("doorLevel_getInfo++\n");
	ASSERT(epInfo != NULL);

	hostCmd cmd;

	memset(&cmd,0,sizeof(cmd));
	doorLevel_getInfoCmd(&cmd);
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	
	log_debug("doorLevel_getInfo--\n");
}

void doorLevel_settimeReq(epInfo_t *epInfo,uint8_t years,uint8_t month,uint8_t dates,uint8_t hours,uint8_t minute,uint8_t seconds,uint8_t week)
{
//	uint8_t year = 0;		//
//	uint8_t month = 0;	//
//	uint8_t dates = 0;	//
//	uint8_t hours = 0;
//	uint8_t minute = 0;
//	uint8_t seconds = 0;
//	uint8_t week = 0;

	log_debug("doorLevel_settime++\n");

	ASSERT(epInfo != NULL);
	
	hostCmd cmd;
	memset(&cmd,0,sizeof(cmd));
	doorLevel_settimeCmd(&cmd, years, month, dates, hours, minute, seconds, week);
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	
	log_debug("doorLevel_settime--\n");
}


/*****************************************************************************
 * �� �� ��  : doorLevel_getRegisterInfo
 * �� �� ��  : Edward
 * ��������  : 2016��10��13��
 * ��������  : ��ȡ�����Ǽ���Ϣ-0X10
 * �������  : epInfo_t *epInfo    �ڵ���Ϣ
               uint8_t registerType  �Ǽ��б�����
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
void doorLevel_getRegisterInfoReq(epInfo_t *epInfo,uint8_t registerType)
{

	log_debug("doorLevel_settime++\n");

	ASSERT(epInfo != NULL);
	
	hostCmd cmd;
	memset(&cmd,0,sizeof(cmd));
	doorLevel_getRegisterInfoCmd(&cmd, registerType);
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	log_debug("doorLevel_settime--\n");
}

void doorLevel_usrMngReq(epInfo_t *epInfo,uint8_t usrType,uint8_t optType,uint32_t usrid,uint8_t *passwd)
{
	log_debug("doorLevel_settime++\n");

	ASSERT(epInfo != NULL);
	
	hostCmd cmd;
	memset(&cmd,0,sizeof(cmd));
	doorLevel_SetUsrMngCmd(&cmd,  usrType, optType, usrid,passwd);
	doorLevel_SendSerialCmd(epInfo->nwkAddr,epInfo->endpoint,afAddr16Bit,cmd.idx,cmd.data);
	log_debug("doorLevel_settime--\n");
}

#if DEVICE_LIWEI_DOOR_OPEN_CNT
void doorLevel_getOpenCntReq(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode)
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

    cmdSet16bitVal_lh(&cmd, 0x0038);//Attr ID
   
    zbMakeMsgEnder(&cmd);
    
	usleep(1000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}

#endif
//��ά���������ڽ��մ�����
int doorLevel_ReviceMsgProcess(epInfo_t *epInfo,hostCmd *cmd)
{
	uint16_t attrID;
	uint8_t dataType;
	uint8_t sof;
	uint8_t datalen;
	uint8_t pkglen;
	uint8_t cmdtype;
	
	ASSERT(epInfo != NULL);
		
	log_debug("doorlock_ReviceMsgProcess++\n");

	cmdGet16bitVal_lh(cmd, &attrID);
    cmdGet8bitVal(cmd, &dataType);

	//���յ����أ��ر��ط�����
    zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

	if((attrID != ATTRID_BASIC_LOCK_UNLOCK) && (dataType != ZCL_DATATYPE_OCTET_STR))
		return -1;
	
	cmdGet8bitVal(cmd, &datalen);	//�����ܳ���
	cmdGet8bitVal(cmd, &sof);		//��ʼ��־
	cmdGet8bitVal(cmd, &pkglen);	//�����
	cmdGet8bitVal(cmd, &cmdtype);	//��������
	
	switch(cmdtype)
	{
		case DOOR_OPEN_MSG://ʵʱ���ż�¼
		{
			doorLevel_realRecordReport(epInfo,cmd);
    	}
		break;
		case DOOR_INFO://����״̬
		{
			doorLevel_doorInfoReport(epInfo,cmd);
		}
		break;
		case DOOR_ALARM_REPORT://����
		{
			doorLevel_alarmOperationReport(epInfo,cmd);
		}
		break;
		case DOOR_USER_LIST_INFO://��ȡ�ȼ���Ϣ
		{
			doorLevel_getRegisterInfoReport(epInfo,cmd);
		}
		break;
		case DOOR_CMD_STATE_REPORT://����״̬����
		{
			doorLevel_allSetCmdReport(epInfo,cmd);
		}
		break;
	}
	
	log_debug("doorlock_ReviceMsgProcess--\n");
	return 0;
}

