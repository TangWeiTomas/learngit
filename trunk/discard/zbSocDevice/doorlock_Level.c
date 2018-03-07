/***********************************************************************************
 * 文 件 名   : doorlock_Level.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年10月12日
 * 文件描述   : 力维门锁,串口信息透传数据处理
 * 版权说明   : Copyright (c) 2008-2016   无锡飞雪网络 技术有限公司
 * 其    他   : 
 * 修改日志   : 
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

//存放服务器下发下来的门锁状态命令
static Level_DoorState_Type doorStateFromServer = DOOR_CLOSE;

/*****************************************************************************
 * 函 数 名  : doorLevel_setOnOff
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月12日
 * 函数功能  : 生成level门锁开门命令
 * 输入参数  : hostCmd *cmd             数据缓冲区
               Level_Open_Types status  开门类型
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_setOnOffCmd(hostCmd *cmd,Level_Open_Types status)
{
	uint8_t cmdOpt = status;
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, 0x01);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	if(status == 0x02)
		cmdOpt = 0xff; //常开

	//开门参数
	cmdSet8bitVal(cmd, cmdOpt);

	cmdSetCheckSum(cmd);
}

/*****************************************************************************
 * 函 数 名  : doorLevel_getInfo
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月12日
 * 函数功能  : 生成获取门锁信息指令
 * 输入参数  : hostCmd *cmd  数据缓存区
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_getInfoCmd(hostCmd *cmd)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x0b);
	cmdSet8bitVal(cmd, 0x07);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//参数，可以不用设置
	cmdSet8bitVal(cmd, 0x00);

	cmdSetCheckSum(cmd);
}

static void doorLevel_settimeCmd(hostCmd *cmd,uint8_t years,uint8_t month,uint8_t dates,uint8_t hours,uint8_t minute,uint8_t seconds,uint8_t week)
{
	cmdSet8bitVal(cmd, ZBDL_MSG_SOC);
	cmdSet8bitVal(cmd, 0x12);
	cmdSet8bitVal(cmd, 0x0a);

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	//保留参数1
	cmdSet8bitVal(cmd, 0x00);

	//参数，时间
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

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//列表类型
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

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);


	if(optType <0 && optType > 3)
		return -1;

	switch(usrType)
	{
		case 0x00://密码ID
		{
			mUsrtType = 0x00;
			mUsrtType |= optType;
			cmdSet8bitVal(cmd, mUsrtType);
			switch(optType)
			{
				case 0x00://单独删除
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x01://单独增加
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
				case 0x02://连续删除
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x03://连续删除
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
		case 0x01://卡片ID
		{
			mUsrtType = 0x04;
			mUsrtType |= optType;
			cmdSet8bitVal(cmd, mUsrtType);
			switch(optType)
			{
				case 0x00://单独删除
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x01://单独增加
				{
					//cmdSet16bitVal(cmd, usrid);
					//cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x02://连续删除
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x03://连续删除
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
		case 0x02://指纹ID
		{
			mUsrtType = 0x08;
			mUsrtType |= optType;
			cmdSet8bitVal(cmd, mUsrtType);
			switch(optType)
			{
				case 0x00://单独删除
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x01://单独增加
				{
					//cmdSet16bitVal(cmd, usrid);
					//cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x02://连续删除
				{
					cmdSet16bitVal(cmd, usrid);
					cmdSetStringVal(cmd,&mData[0],4);
				}
				break;
				case 0x03://连续删除
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

	//保留
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

	//区域-套房
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);
	cmdSet8bitVal(cmd, 0x00);

	//参数1
	cmdSet8bitVal(cmd, usrType&0x0c);
	cmdSet8bitVal(cmd, optType&0x03);

	usr = usrType&0x0c;
	opt = optType&0x03;
	
	cmdSetCheckSum(cmd);
}

/*****************************************************************************
 * 函 数 名  : doorLevel_SendSerialCmd
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月12日
 * 函数功能  : 发送力维透传数据
 * 输入参数  : uint16_t dstAddr  目标地址
               uint8_t endpoint  目标端口号
               uint8_t addrMode  发送类型
               uint8_t len       数据长度
               uint8_t* buf      数据
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_SendSerialCmd(uint16_t dstAddr, uint8_t endpoint, uint8_t addrMode, uint8_t len,uint8_t* buf)
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

    cmdSet16bitVal_lh(&cmd, ATTRID_BASIC_DOORLOCK_UART_MSG);//Attr ID
    cmdSet8bitVal(&cmd, ZCL_DATATYPE_OCTET_STR);//Data Type

    cmdSet8bitVal(&cmd, len);//数据长度
	cmdSetStringVal(&cmd,buf,len);//数据
   
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
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11-D17 控制结果
    cmdSetStringVal(&cmd,times,7);
	//D18-D19 门锁硬件版本
	cmdSet16bitVal(&cmd,dversion);
	//D20-D21 门锁软件版本
	cmdSet16bitVal(&cmd,sversion);
	//D22 门铃状态
	cmdSet8bitVal(&cmd,doorbellstate);
	//D23 支持开锁方式
	cmdSet8bitVal(&cmd,opentype);
    
    makeMsgEnder(&cmd);
    
    cmdMsgSend(cmd.data,cmd.idx);
}


/*****************************************************************************
 * 函 数 名  : doorLevel_doorStateInd
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 门锁开关状态上报
 * 输入参数  : epInfo_t *epInfo  节点信息
               uint8_t state       当前状态
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_doorStateInd(epInfo_t *epInfo,uint8_t state)
{
	log_debug("doorLevel_doorStateInd++\n");
	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_DOORLOCK_STATUS_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 控制结果
    cmdSet8bitVal(&cmd,state);
    //D12 信号质量
    cmdSet8bitVal(&cmd,epInfo->onlineDevRssi);
    makeMsgEnder(&cmd);
    
    cmdMsgSend(cmd.data,cmd.idx);
}

//门锁开门日志上报
static void doorLevel_realRecordInd(epInfo_t *epInfo,uint8_t state,uint8_t cardtype,uint8_t *uid,uint8_t *times)
{
	log_debug("doorLevel_realRecordInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_RECORD_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁状态
    cmdSet8bitVal(&cmd,state);
    //D12 开门类型
    cmdSet8bitVal(&cmd,cardtype);
    //D13-D16 UID0-UID3
    cmdSetStringVal(&cmd,uid,4);
    //D17-D22 门锁时间
    cmdSetStringVal(&cmd,times,6);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
   
}


//门锁电量状态上报
static void doorLevel_alarmPowerInd(epInfo_t *epInfo,uint8_t doorPowerState,uint8_t wirelessPowerState)
{
	log_debug("doorLevel_alarmPowerInd++\n");

	hostCmd cmd;
    cmd.idx = 0;
    
    makeMsgHeader(&cmd,CMD_MSG_DIR_IND);
    //D0 D1 Opcode
    cmdSet16bitVal(&cmd,WIFI_ZIGBEE_LEVEL_DOOR_POWER_IND);
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 门锁电压
    cmdSet8bitVal(&cmd,doorPowerState);
    //D12 无线电压
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
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 非法操作类型
    cmdSet8bitVal(&cmd,alarmType);
    //D12-D18 非法操作时间
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
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 类型
    cmdSet8bitVal(&cmd,regType);
	//UID登记信息
    cmdSet8bitVal(&cmd,udisize);
    //D12-D18 非法操作时间
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
    //D2-D9 控制结果
    cmdSetStringVal(&cmd,epInfo->IEEEAddr,8);
    //D10 控制结果
    cmdSet8bitVal(&cmd,epInfo->endpoint);
    //D11 非法操作类型
    cmdSet8bitVal(&cmd,state);
   
    makeMsgEnder(&cmd);
    cmdMsgSend(cmd.data,cmd.idx);
}


//0x1c门锁开门实时记录
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

//定时去上报门锁开关状态
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
 * 函 数 名  : doorLevel_precessDoorState
 * 负 责 人  : Edward
 * 创建日期  : 2016年11月29日
 * 函数功能  : 处理门锁当前状态逻辑
 * 输入参数  : uint8_t cardType  门锁开门类型
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static uint8_t doorLevel_precessDoorState(epInfo_t *epInfo,uint8_t doorOperateType,uint8_t doorOpenType)
{

	log_debug("doorLevel_precessDoorState++\n");
	uint8_t doorstate = DOOR_CLOSE;

	if(doorOperateType == 0x1c)//记录卡
	{
		doorstate = DOOR_CLOSE;//门关闭状态
	}
	else
	{
		//获取门锁状态
		switch(doorStateFromServer)
		{
			case DOOR_CLOSE://关门
			{
				doorstate = DOOR_CLOSE;
			}	
			break;
			case DOOR_ALWAYS_OPEN://常开
			{
				doorstate = DOOR_ALWAYS_OPEN;//常开
			}
			break;
			case DOOR_OPEN://普通开门
			{
				doorstate = DOOR_OPEN;//开启
				//启动定时获取门锁状态
				//doorLevel_timerGetInfo(epInfo);
				//自动去上报门锁状态
				doorLevel_timerDoorStateInd(epInfo);
			}
			break;
			default:
			{
				doorstate = DOOR_CLOSE;//开启
				//启动定时获取门锁状态
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
 * 函 数 名  : doorLevel_realRecordReport
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 门锁开门日志上报-0x1C
 * 输入参数  : epInfo_t *epInfo  节点信息
               hostCmd *cmd      串口透传数据
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void doorLevel_realRecordReport(epInfo_t *epInfo,hostCmd *cmd)
{
	uint8_t area = 0; 	//区域
	uint8_t unit = 0;		//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房
	uint8_t cardType = 0 ; //卡片类型
	uint8_t uid[4] = {0};	//卡片信息
	uint8_t year = 0;		//开门时间
	uint8_t month = 0;	//
	uint8_t dates = 0;	//
	uint8_t hours = 0;
	uint8_t minute = 0;
	uint8_t power = 0;	//电压状态

	uint8_t doorstate = 0;
	uint8_t doorOperateType = 0; //操作类型
	uint8_t times[6] = {0};
	uint8_t doorPowerAlarm = 0;
	uint8_t wiressPowerAlarm = 0;

	ASSERT((epInfo != NULL) && (cmd != NULL));
	
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//有用信息
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

	//处理门锁状态逻辑
	doorstate = doorLevel_precessDoorState(epInfo,doorOperateType,cardType & 0x40);
	
	if(year & 0x80)//非法时间
	{
		times[0] = 0;//无效
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
		doorPowerAlarm = 0x01;//报警状态
	}
	else
	{
		doorPowerAlarm = 0x00;
	}
	
	if(power & 0x80)
	{
		wiressPowerAlarm = 0x01;//报警状态
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
	if((cardType&0x1F) == 0x1c)//卡的类型为记录卡
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
 * 函 数 名  : doorLevel_doorInfoReport
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 当前门锁的信息--0x08
 * 输入参数  : epInfo_t *epInfo  节点信息
               hostCmd *cmd      透传串口数据
 * 输出参数  : 无
 * 返 回 值  : static
 * 调用关系  : 
 * 其    它  : 

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

	uint8_t area = 0; 	//区域
	uint8_t unit = 0;		//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房
	
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

	//获取门锁时间
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

	//软件硬件版本
	cmdGet16bitVal(cmd,&dverson);
	cmdGet16bitVal(cmd,&sversion);

	cmdGet8bitVal(cmd, &doorState);

	if(doorState & 0x80)
	{
		doorPowerAlarm = 0x01;//报警状态
	}
	else
	{
		doorPowerAlarm = 0x00;
	}
	
	if(doorState & 0x40)
	{
		wiressPowerAlarm = 0x01;//报警状态
	}
	else
	{
		wiressPowerAlarm = 0x00;
	}
	
	//允许开锁方式
	dooropentype = doorState&0x0f; 

 	//门铃状态
	if(doorState & 0x10)
		doorbell = 0x01;
	else
		doorbell = 0x00;

	//门锁状态
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
 * 函 数 名  : doorLevel_alarmOperationReport
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 非法操作数据上报-0X0B
 * 输入参数  : epInfo_t *epInfo  节点信息
               hostCmd *cmd      串口透传数据
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
void doorLevel_alarmOperationReport(epInfo_t *epInfo,hostCmd *cmd)
{

	ASSERT((epInfo != NULL) && (cmd != NULL));
	uint8_t area = 0; 	//区域
	uint8_t unit = 0;		//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房

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

	//操作类型
	cmdGet8bitVal(cmd, &alarmOptType);
	
	alarmOptType = alarmOptType&0x07;

	//获取门锁时间
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

	uint8_t area = 0; 	//区域
	uint8_t unit = 0;		//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房

	
	uint8_t uidsize = 0;
	uint8_t regType = 0;
	uint8_t *uid = NULL;
	cmdGet8bitVal(cmd, &area);
	cmdGet8bitVal(cmd, &unit);
	cmdGet8bitVal(cmd, &floor);
	cmdGet8bitVal(cmd, &room);
	cmdGet8bitVal(cmd, &suite);

	//操作类型
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
	
	uint8_t area = 0; 	//区域
	uint8_t unit = 0; 	//楼栋单元
	uint8_t floor = 0 ;	//楼层
	uint8_t room = 0	;	//房间
	uint8_t suite = 0 ;	//套房


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
//							对外接口
//////////////////////////////////////////////////////////////////////////////


void doorLevel_setOnOffReq(epInfo_t *epInfo,uint8_t optype)
{
	log_debug("doorLevel_setOnOff++\n");
	ASSERT(epInfo != NULL);

	hostCmd cmd;

	memset(&cmd,0,sizeof(cmd));
	
	doorStateFromServer = optype;

	//组包
	doorLevel_setOnOffCmd(&cmd,optype);

	//发送数据
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
 * 函 数 名  : doorLevel_getRegisterInfo
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月13日
 * 函数功能  : 获取门锁登记信息-0X10
 * 输入参数  : epInfo_t *epInfo    节点信息
               uint8_t registerType  登记列表类型
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

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

    cmdSet16bitVal_lh(&cmd, 0x0038);//Attr ID
   
    zbMakeMsgEnder(&cmd);
    
	usleep(1000);//300ms
    zbSocCmdSend(cmd.data,cmd.idx);
}

#endif
//力维门锁，串口接收处理函数
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

	//接收到返回，关闭重发机制
    zblist_remove(epInfo->nwkAddr,epInfo->endpoint);

	if((attrID != ATTRID_BASIC_LOCK_UNLOCK) && (dataType != ZCL_DATATYPE_OCTET_STR))
		return -1;
	
	cmdGet8bitVal(cmd, &datalen);	//数据总长度
	cmdGet8bitVal(cmd, &sof);		//起始标志
	cmdGet8bitVal(cmd, &pkglen);	//命令长度
	cmdGet8bitVal(cmd, &cmdtype);	//命令类型
	
	switch(cmdtype)
	{
		case DOOR_OPEN_MSG://实时开门记录
		{
			doorLevel_realRecordReport(epInfo,cmd);
    	}
		break;
		case DOOR_INFO://门锁状态
		{
			doorLevel_doorInfoReport(epInfo,cmd);
		}
		break;
		case DOOR_ALARM_REPORT://报警
		{
			doorLevel_alarmOperationReport(epInfo,cmd);
		}
		break;
		case DOOR_USER_LIST_INFO://获取等级信息
		{
			doorLevel_getRegisterInfoReport(epInfo,cmd);
		}
		break;
		case DOOR_CMD_STATE_REPORT://命令状态返回
		{
			doorLevel_allSetCmdReport(epInfo,cmd);
		}
		break;
	}
	
	log_debug("doorlock_ReviceMsgProcess--\n");
	return 0;
}

