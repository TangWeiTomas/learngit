#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/msg.h>
#include <stdint.h>

#include "Types.h"
#include "DeviceCode.h"
#include "zbSocCmd.h"

//#include "Socket_Interface.h"
#include "fileMng.h"
#include "zbSocCmd.h"
#include "Zigbee_device_Heartbeat_Manager.h"
#include "interface_deviceStatelist.h"
#include "interface_eventlist.h"
#include "SimpleDBTxt.h"
#include "scene_manager.h"
#include "interface_timetasklist.h"

//ͶӰ���豸��������
const DeviceIndex_t PJT_index_info[] = 
{
	{0,"������(Epson)"},
	{1,"��֥(Toshiba)"},
	{2,"����(Sharp)"},
	{3,"����(Panasonic)"},
	{4,"����(Fangzheng)"},
	{5,"��ʿͨ(Fujitsu)"},
	{6,"����(Hitachi)"},
	{7,"����(Sony)"},
	{8,"����(Sanyo)"},
	{9,"��ͼ��(Optoma)"},
	{10,"NEC(NEC)"},
	{11,"����(Mitsubishi)"},
	{12,"������(InFocus)"},
	{13,"�б�(Zhongbao)"},
	{14,"���(Rui Cheng"},
	{15,"some(some)"},
	{16,"����(HP)"},
	{17,"����(Dell)"},
	{18,"����(Canon)"},
	{19,"���(Acer)"},
	{20,"����(BenQ)"},
	{21,"�ȷ�(Pioneer)"},
	{22,"����(Morningstar"},
	{23,"������(The Polaroid)"},
	{24,"����(Aiqi)"},
	{25,"LG(LG)"},
	{26,"�ĵ���(PREMIER)"},
	{27,"������(Philips)"},
	{28,"����(ViewSonic)"},
	{29,"����Ʒ��"}
};

//������������
const DeviceIndex_t Tv_index_info[] = 
{
	{0, "����"},
	{1, "����"},
	{2, "TCL"},
	{3, "�û�"},
	{4, "��ά(SKYWORTH)"},
	{5,	"����(Hisense)"},
	{6,	"����(Haier)"},
	{7,	"����(JINXING)"},
	{8,	"��è(PANDA)"},
	{9,	"����(sony)"},
	{10,"����(Panasonic)"},
	{11,"����(Panasonic)"},
	{12,"��֥(TOSHIBA)"},
	{13,"����(HITACHI)"},
	{14,"����(SHARP)"},
	{15,"����(Sampo)"},
	{16,"������(Philips)"},
	{17,"����(SAMSUNG)"},
	{18,"����(SANYO)"},
	{19,"�յ�(Xinhua)"},
	{20,"����(XIHU)"},
	{21,"����(BEIJING)"},
	{22,"��·��(CONROWA)"},
	{23,"�ֻ�(ROWA)"},
	{24,"����TCBO_����TOS"},
	{25,"����(Furi)"},
	{26,"LG(LG)"},
	{27,"����(Tupper)"},
	{28,"����(CHANGCHENG)"},
	{29,"�ƺ�(Yellow River)"},
	{30,"��ɽ(Huangshan)"},
	{31,"����(Other)"}, 
	{32,"����(sanken)"},
	{33,"������(Polytron)"},
	{34,"����(Aiwa)"},
	{35,"�廪ͬ��(Tongfang)"},
	{36,"�ȷ�(Pioneer TV)"},
	{37,"����(LeTV)"},
	{38,"����(Amoi)"},
	{39,"С��(Xiaomi)"},
	{40,"����(Horlon)"},
};

//����������
const DeviceIndex_t STB_index_info[] = 
{
    {0,"����(Beijing)"},
    {1,"�㶫(Guangdong)"},
    {2,"����(Guangxi)"},
    {3,"�Ϻ�/��������(Shanghai / Oriental Cable)"},
    {4,"���(Tianjin)"},
    {5,"����(Chongqing)"},
    {6,"����(LIAONING)"},
    {7,"����(JIANGSU)"},
    {8,"����(Hubei)"},
    {9,"�Ĵ�(Sichuan)"},
    {10,"����(Shaanxi)"},
    {11,"�㽭(Zhejiang)"},
    {12,"����(Hunan)"},
    {13,"ɽ��(Shandong)"},
    {14,"����(Anhui)"},
    {15,"����(Guizhou)"},
    {16,"������(Heilongjiang)"},
    {17,"ɽ��(Shanxi)"},
	{18,"����(Inner Mongolia)"},
	{19,"����(Yunnan)"},
	{20,"����(Henan)"},
	{21,"����(Hainan)"},
    {22,"����(Jilin)"},
    {23,"�ӱ�(Hebei)"},
	{24,"����(Fujian)"},
	{25,"�½�(Xinjiang)"},
	{26,"����(Jiangxi)"},
	{27,"��̫(East Pacific)"},
    {28,"��������(Other regions)"},
    {29,"���Ǿź�(Star on the 9th)"},
	{30,"����(Gansu)"},
	{31,"���(Hong Kong)"},
	{32,"̨��(Taiwan)"},
	{33,"��Ϊ������(Huawei STB)"},
	{34,"���������(Changhong STB)"},
	{35," ���ѻ�����(Konka STB)"},
	{36,"���˻�����(ZTE STB)"},
	{37,"���޻�����(Jiuzhou STB)"},     
	{38,"Ħ������������(Motorola STB)"},
	{39,"��ά������(Skyworth STB)"},
	{40," ���Ż�����(Hisense STB)"},
	{41,"����������(Haier STB)"},
	{42,"��˹����������(GOSPELL STB)"},
	{43,"�����ϻ�����(Hoi Mei Di STB)"},
	{44,"������������(Open Bor STB)"},
	{45,"��ػ�����(DVN STB)"},
	{46,"���Ի�����(Daxian STB)"},
	{47,"���ӻ�����(Music as STB)"},
	{48,"˹�￵������(Starcom STB)"},
	{49,"���ӻ�����(Galaxy STB)"},
	{50,"����(Nine joint)"},
	{51,"�軪���������(Gehua STB)"},
	{52,"����������(Tianwei STB)"},
	{53,"�Ļ�������(Hua Xia STB)"},
	{54,"�Ѵ�������(Jia Chong STB)"},
	{55,"�Ѳʻ�����(Jia Cai STB)"},
	{56,"����ͨ������(Gold Netcom STB)"},
	{57,"��������(Juyou Network)"   },
	{58,"���续������(U.S. picturesque STB)"},
	{59,"��������(East WideSight)"},
	{60,"���»�����(Panasonic STB)"},
	{61,"�Ϲ�������(SVA STB)"},
	{62,"���������(BBEF STB"},
	{63,"ȫ��������(Panoramic STB)"},
	{64,"���ƻ�����(Air Division STB)"},
	{65,"���ϻ�����(Leaguer STB)"},
	{66,"��ʤ������(Yum STB)/MATRIX"},
	{67,"MATRIX(MATRIX)"},
	{68,"�˳�������(Wave)"},
	{69,"����������Ѷ(TOPWAY)"},
    {70,"�����軪����(Gehua)"},
    {71,"����ͨ(Huhutong)"},
    {72,"С�׺���(Millet box)"}
};

//�յ��豸����
const DeviceIndex_t ARC_index_info[] =      
{
	{0,"����(Midea)"},
	{1,"��֥(Toshiba)"},
	{2,"����(Haier)"},
	{3,"����(Gree)"},
	{4,"־��(Chigo)"},
	{5,"�¿�˹(Oaks)"},
	{6,"����(Chunlan)"},
	{7,"LG(LG)"},
	{8,"TCL(TCL)"},
	{9,"����(Samsung)"},
	{10,"С����(Small Samsung)"},
	{11,"����(Sanyo)"},
	{12,"�����ع�(Sanling Heavy Industries)"},
	{13,"�������(Sanling Electrical)"},
	{14,"����(Changhong)"},
	{15,"����(Panasonic)"},
	{16,"����(Sound of music)"},
	{17,"����(Uchida)"},
	{18,"����(Hisense)"},
	{19,"�¿�(Shinco)"},
	{20,"����(Hitachi)"},
	{21,"����(Hualing)"},
	{22,"����(Kelon)"},
	{23,"����(Warburg)"},
	{24,"����(Might)"},
	{25,"����(Changling)"},
	{26,"ӭ��(Ying Yan)"},
	{27,"����(Blue Wave)"},
	{28,"����(Rabbit)"},
	{29,"����(Carrier)"},
	{30,"����(Toho)"},
	{31,"��·��(Gaoluhua)"},
	{32,"�Ŀ���(Aucma)"},
	{33,"ʤ��1(Shengfeng)"},
	{34,"����(Yangzi)"},
	{35,"�ݶ���(Whirlpool)"},
	{36,"���(Daikin)"},
	{37," ���(HSBC)"},
	{38,"��֥2(Toshiba)"},
	{39," ������˹(Electrolux)"},
	{40,"��è(Panda)"},
	{41,"����(Ling on)"},
	{42,"��Ԫ(Tianyuan)"},
	{43,"����Ʒ��(Other brands)"},
	{44,"�ٺ�(Lily)"},
	{45,"SimpLlcrty(SimpLlcrty)"},
	{46,"����(Daewoo)"},
	{47,"��Զ(Clarent)"},
	{48,"����(Colones)"},
	{49,"�Ϸ�(Southwind)"},
	{50,"Ⱥ��(Kunda)"},
	{51,"�ȿ�(Yushchenko)"},
	{52,"�·�(Freenet)"},
	{53,"ʤ��2(Shengfeng)"},
	{54,"̩��(Thailand)"},
	{55,"������(Glanz)"},
	{56,"�ֻ�(Leroy)"},
	{57,"��ʿͨ(Fujitsu)"},
	{58,"Լ��(York)"},
	{59,"���ά��(McQuay)"},
	{60,"����(sanken)"},
	{61," ������(polytron)"},
	{62," ����(SHARP)"},
	{63," ��(ZHEN BAO)"},
	{64," ��(WANBAO)"},
	{65," ӣ��(YINGHUA)"}
};

//���������
const DeviceIndex_t IPTV_index_info[] =  
{
	{0," ���(beacon)"},
	{1," ��Ϊ(Huawei)"},
	{2," ˹�￵(UTS)"},
	{3," ͬ��(states)"},
	{4," ����(ZTE)"},
	{5," ����(Daya)"},
	{6," ����(Changhong)"}
};
//=========================================
//�豸��������
#define PJT_DEVICE_SIZE 	30
#define	FAN_DEVICE_SIZE		22
#define	STB_DEVICE_SIZE 	73
#define	DVD_DEVICE_SIZE		161
#define TV_DEVICE_SIZE		41 
#define	IPTV_DEVICE_SIZE	7
#define ARC_DEVICE_SIZE		66	//�յ��豸����
const int IRC_DEVICE_SIZE[] = 
{
	PJT_DEVICE_SIZE, 	
	FAN_DEVICE_SIZE	,	
	STB_DEVICE_SIZE ,	
	DVD_DEVICE_SIZE	,	
	TV_DEVICE_SIZE ,
	IPTV_DEVICE_SIZE,
	ARC_DEVICE_SIZE
};
//=========================================

//�豸�����������
#define PJT_TABLE_LIST_SIZE 	112
#define	FAN_TABLE_LIST_SIZE		151
#define	STB_TABLE_LIST_SIZE 	5451
#define	DVD_TABLE_LIST_SIZE		2867
#define TV_TABLE_LIST_SIZE		11057 
#define	IPTV_TABLE_LIST_SIZE	66
#define ARC_TABLE_LIST_SIZE		860

const int IRC_TABLE_LIST_SIZE[] = 
{
	PJT_TABLE_LIST_SIZE, 	
	FAN_TABLE_LIST_SIZE	,	
	STB_TABLE_LIST_SIZE ,	
	DVD_TABLE_LIST_SIZE	,	
	TV_TABLE_LIST_SIZE ,
	IPTV_TABLE_LIST_SIZE,
	ARC_TABLE_LIST_SIZE
};
//=========================================
//�豸��Դ��λ������λ��
#define PJT_POWER_INDEX_START 		1
#define	FAN_POWER_INDEX_START		1
#define	STB_POWER_INDEX_START 		1
#define	DVD_POWER_INDEX_START		11
#define TV_POWER_INDEX_START		11 
#define	IPTV_POWER_INDEX_START		1

//=========================================
//COM��λ������λ��
#define PJT_COM_CODE_START 		45
#define	FAN_COM_CODE_START		45
#define	STB_COM_CODE_START 		47
#define	DVD_COM_CODE_START		39
#define TV_COM_CODE_START		51 
#define	IPTV_COM_CODE_START		47

//=========================================

//=========================================


uint8_t Create_Irc_Arc_Ctr_Package(uint16_t deviceTableIndex,uint8_t keyLength,uint8_t*devicekey,hostCmd *cmd);
uint8_t Create_Irc_Arc_Query_Package(uint16_t deviceNameId,uint16_t deviceModel,hostCmd *cmd);

uint8_t PackageSetCheckSumFCS(hostCmd *cmd)
{
    uint8_t fcs=0;
    if((cmd->idx + 1) < MaxPacketLength)
    {
        uint16_t cnt;
        for(cnt=0; cnt<cmd->idx; cnt++)
        {
            fcs += cmd->data[cnt];
        }
        cmd->data[cmd->idx++] = fcs;
        return false;
    }
    else
    {
        return true;
    }
}


uint8_t Generate_Irc_Package(uint8_t deviceType,uint16_t CodeIndex,uint8_t keyLength,uint8_t *deviceKey,hostCmd *cmd)
{
	memset(cmd,0,sizeof(hostCmd));
	cmdSet8bitVal(cmd,0x30);
	cmdSet8bitVal(cmd,0x00);
	uint8_t index = (*deviceKey);
	
	switch(deviceType)
	{
#if PJT_ENABLE
	case IRC_DEVICE_TYPE_PJT:
			
			cmdSet8bitVal(cmd,stb_pjt_table[CodeIndex][0]); //F_CODE
			//cmdSetStringVal(cmd,deviceKey,keyLength);
			cmdSetStringVal(cmd,(uint8_t *)&stb_pjt_table[CodeIndex][index],2);
			cmdSetStringVal(cmd,(uint8_t *)&stb_pjt_table[CodeIndex][PJT_COM_CODE_START],4);
		break;
#endif
#if FAN_ENABLE
	case IRC_DEVICE_TYPE_FAN:
			cmdSet8bitVal(cmd,stb_fan_table[CodeIndex][0]); //F_CODE
			//cmdSetStringVal(cmd,deviceKey,keyLength);
			cmdSetStringVal(cmd,(uint8_t *)&stb_fan_table[CodeIndex][index],2);
			cmdSetStringVal(cmd,(uint8_t *)&stb_fan_table[CodeIndex][FAN_COM_CODE_START],4);
		break;
#endif
#if STB_ENABLE
	case IRC_DEVICE_TYPE_STB:	
			cmdSet8bitVal(cmd,stb_data_table[CodeIndex][0]); //F_CODE
			//cmdSetStringVal(cmd,deviceKey,keyLength);
			cmdSetStringVal(cmd,(uint8_t *)&stb_data_table[CodeIndex][index],2);
			cmdSetStringVal(cmd,(uint8_t *)&stb_data_table[CodeIndex][STB_COM_CODE_START],4);
		break;
#endif
#if DVD_ENABLE
	case IRC_DEVICE_TYPE_DVD:	
			cmdSet8bitVal(cmd,dvd_data_table[CodeIndex][0]); //F_CODE
			//cmdSetStringVal(cmd,deviceKey,keyLength);
			cmdSetStringVal(cmd,(uint8_t *)&dvd_data_table[CodeIndex][index],2);
			cmdSetStringVal(cmd,(uint8_t *)&dvd_data_table[CodeIndex][DVD_COM_CODE_START],4);
		break;
#endif
#if TV_ENABLE
	case IRC_DEVICE_TYPE_TV:	
			cmdSet8bitVal(cmd,tv_table[CodeIndex][0]); //F_CODE
			//cmdSetStringVal(cmd,deviceKey,keyLength);
			cmdSetStringVal(cmd,(uint8_t *)&tv_table[CodeIndex][index],2);
			cmdSetStringVal(cmd,(uint8_t *)&tv_table[CodeIndex][TV_COM_CODE_START],4);
			
		break;
#endif
#if IPTV_ENBALE
	case IRC_DEVICE_TYPE_IPTV:
			cmdSet8bitVal(cmd,remote_IPTV_table[CodeIndex][0]); //F_CODE
			//cmdSetStringVal(cmd,deviceKey,keyLength);
			cmdSetStringVal(cmd,(uint8_t *)&remote_IPTV_table[CodeIndex][index],2);
			cmdSetStringVal(cmd,(uint8_t *)&remote_IPTV_table[CodeIndex][IPTV_COM_CODE_START],4);
		break;
#endif
	default:
		memset(cmd,0,sizeof(hostCmd));
		return false;
	}
	PackageSetCheckSumFCS(cmd);
	return true;
}


//��ȡ�豸������ֵ
uint8_t GetDevicePowerKey(uint8_t deviceType)
{
	
	switch(deviceType)
	{
	case IRC_DEVICE_TYPE_PJT:
		#if PJT_ENABLE
			return PJT_KEY_OPEN;
		#endif
		break;
	case IRC_DEVICE_TYPE_FAN:
		#if FAN_ENABLE
			return FAN_KEY_ONOFF;
		#endif
		break;
	case IRC_DEVICE_TYPE_STB:
		#if STB_ENABLE
			return STB_KEY_STANDBY;
		#endif
		break;
	case IRC_DEVICE_TYPE_DVD:
		#if DVD_ENABLE
			return DVD_KEY_POWER;
		#endif
		break;
	case IRC_DEVICE_TYPE_TV:
		#if TV_ENABLE
			return TV_KEY_POWER;
		#endif
		break;
	case IRC_DEVICE_TYPE_IPTV:
		#if IPTV_ENBALE
			return IPTV_KEY_POWER;
		#endif
		break;
	default:
		break;
	}
	
	return -1;
}

//��ȡ�豸������ֵ
uint16_t GetDeviceIndexInTable(uint8_t deviceType,uint16_t deviceNameId,uint16_t deviceModel)
{
	uint16_t CodeIndex  = -1;
	
	switch(deviceType)
	{
	case IRC_DEVICE_TYPE_PJT:
		#if PJT_ENABLE
			CodeIndex = remote_pjt_info[deviceNameId][deviceModel];
		#endif
		break;
	case IRC_DEVICE_TYPE_FAN:
		#if FAN_ENABLE
			CodeIndex = remote_fan_info[deviceNameId][deviceModel];
		#endif
		break;
	case IRC_DEVICE_TYPE_STB:
		#if STB_ENABLE
			CodeIndex = remote_stb_info[deviceNameId][deviceModel];
		#endif
		break;
	case IRC_DEVICE_TYPE_DVD:
		#if DVD_ENABLE
			CodeIndex = remote_dvd_info[deviceNameId][deviceModel];
		#endif
		break;
	case IRC_DEVICE_TYPE_TV:
		#if TV_ENABLE
			CodeIndex = TV_info[deviceNameId][deviceModel];
		#endif
		break;
	case IRC_DEVICE_TYPE_IPTV:
		#if IPTV_ENBALE
			CodeIndex = remote_IPTV_info[deviceNameId][deviceModel];
		#endif
		break;
	case IRC_DEVICE_TYPE_ARC:
		#if ARC_ENABLE
			CodeIndex = g_remote_arc_info[deviceNameId][deviceModel];
		#endif
		break;
	default:
		break;
	}

	if((CodeIndex == -1)||(CodeIndex > IRC_TABLE_LIST_SIZE[deviceType]))
		return -1;
	
	return CodeIndex;
}

//��ȡ�豸������
uint16_t GetDeviceSize(uint8_t deviceType,uint16_t deviceNameId,uint16_t deviceModel)
{
	uint16_t deviceNum  = 0;
	if(deviceNameId > IRC_DEVICE_SIZE[deviceType])
		return false;
	
	switch(deviceType)
	{
		
	case IRC_DEVICE_TYPE_PJT:
		#if PJT_ENABLE
			deviceNum = remote_pjt_info[deviceNameId][0];
		#endif
		break;
	case IRC_DEVICE_TYPE_FAN:
		#if FAN_ENABLE
			deviceNum = remote_fan_info[deviceNameId][0];
		#endif
		break;
	case IRC_DEVICE_TYPE_STB:
		#if STB_ENABLE
			deviceNum = remote_stb_info[deviceNameId][0];
		#endif
		break;
	case IRC_DEVICE_TYPE_DVD:
		#if DVD_ENABLE
			deviceNum = remote_dvd_info[deviceNameId][0];
		#endif
		break;
	case IRC_DEVICE_TYPE_TV:
		#if TV_ENABLE
			deviceNum = TV_info[deviceNameId][0];
		#endif
		break;
	case IRC_DEVICE_TYPE_IPTV:
		#if IPTV_ENBALE
			deviceNum = remote_IPTV_info[deviceNameId][0];
		#endif
		break;
	case IRC_DEVICE_TYPE_ARC:
		#if ARC_ENABLE
			deviceNum = g_remote_arc_info[deviceNameId][0];
		#endif
		break;
	default:
		break;
	}
	
	if(deviceNum < deviceModel)
		return false;
	log_debug("deviceNum = %d\n",deviceNum);
	return deviceNum;
}


//�����ӵȺ����豸�������ݷ���������豸��Ѱ
uint16_t Create_Irc_Query_Package(uint8_t deviceType,uint16_t deviceNameId,uint16_t deviceModel,hostCmd *cmd)
{
	uint16_t DataTableIndex = -1;//�洢�豸�ĺ����������仯
	if(deviceType > IRC_DEVICE_TYPE_SIZE)
		return -1;
	
	//������⣬���deviceNameId ��deviceModel
	if(GetDeviceSize(deviceType,deviceNameId,deviceModel) > 0)
	{
		DataTableIndex = GetDeviceIndexInTable(deviceType,deviceNameId,deviceModel);
		if(DataTableIndex >= 0)
		{
			if(deviceType == IRC_DEVICE_TYPE_ARC) //�յ����豸
			{
				Create_Irc_Arc_Query_Package(deviceNameId,deviceModel,cmd);
			}
			else//�ǿյ����豸
			{
				uint8_t keyIndex = GetDevicePowerKey(deviceType);
				log_debug("PowerKey = %x \n",keyIndex);
				Generate_Irc_Package(deviceType,DataTableIndex,1,&keyIndex,cmd);//������ֵ
			}
		}
	}
	return DataTableIndex;
}



//�����ӵȺ����豸�������ݷ���������豸��Ѱ
uint16_t Create_Irc_Query_Device_Number_Package(uint8_t deviceType,uint16_t deviceNameId)
{
	if(deviceType > IRC_DEVICE_TYPE_SIZE)
		return false;
	return GetDeviceSize(deviceType,deviceNameId,0);
}

//�����ӵĺ�����з��
uint8_t Create_Irc_Ctrl_Package(uint8_t deviceType,uint16_t deviceTableIndex,uint8_t keyLength,uint8_t* deviceKey,hostCmd *cmd)
{
	uint8_t ret = false;
	uint8_t len = 0;
	if(deviceType > IRC_DEVICE_TYPE_SIZE)
		return false;


	log_debug("deviceType = %d\n,deviceTableIndex = %d\n,keyLength = %d\n",deviceType,deviceTableIndex,keyLength);

	#ifndef NDEBUG
	for(len = 0;len<keyLength;len++)
		log_debug("%d\n",deviceKey[len]);
	#endif
	
	if(deviceType == IRC_DEVICE_TYPE_ARC ) //�յ����豸
	{
		ret = Create_Irc_Arc_Ctr_Package(deviceTableIndex,keyLength,deviceKey,cmd);
	}
	else	//�ǿյ����豸
	{
		ret = Generate_Irc_Package(deviceType,deviceTableIndex,keyLength,deviceKey,cmd);//������ֵ
	}
	
	return ret;
}

//�յ��豸
uint8_t Create_Irc_Arc_Query_Package(uint16_t deviceNameId,uint16_t deviceModel,hostCmd *cmd)
{
#if ARC_ENABLE
	uint16_t deviceNum = 0; 
	uint16_t DataTableIndex = 0;
	if(deviceNameId > ARC_DEVICE_SIZE) //�����豸��������
		return false;
	
	deviceNum = g_remote_arc_info[deviceNameId][0];

	if(deviceModel > deviceNum)
		return false;
	
	//��ȡ�����
	DataTableIndex = g_remote_arc_info[deviceNameId][deviceModel];

	if(DataTableIndex > ARC_TABLE_LIST_SIZE)
		return false;
	
	memset(cmd,0,sizeof(hostCmd));
	cmdSet8bitVal(cmd,0x30);
	cmdSet8bitVal(cmd,0x01);
	cmdSet16bitVal(cmd,DataTableIndex); //�����
	cmdSet8bitVal(cmd,0x1b);//7B0 �����¶�25��
	cmdSet8bitVal(cmd,0x01);//7B1 ���÷���
	cmdSet8bitVal(cmd,0x02);//7B2 �����ֶ�����
	cmdSet8bitVal(cmd,0x01);//7B3 ���������Զ�����
	cmdSet8bitVal(cmd,0x01);//7B4 ���ÿ�������	//����
	cmdSet8bitVal(cmd,0x01);//7B5 ���ü�����Ӧ����
	cmdSet8bitVal(cmd,0x05);//7B6 ����ģʽ��Ӧ����

	uint8_t length = arc_table[DataTableIndex][0];
	cmdSet8bitVal(cmd,length+1);
	cmdSetStringVal(cmd,(uint8_t *)&arc_table[DataTableIndex][1],length);
	cmdSet8bitVal(cmd,0xff);
	PackageSetCheckSumFCS(cmd);
#endif
	return true;
}


//�յ��豸����
uint8_t Create_Irc_Arc_Ctr_Package(uint16_t deviceTableIndex,uint8_t keyLength,uint8_t*devicekey,hostCmd *cmd)
{
#if ARC_ENABLE
	uint16_t deviceNum = -1; 

	if(deviceTableIndex > ARC_TABLE_LIST_SIZE)
		return false;
	
	memset(cmd,0,sizeof(hostCmd));
	cmdSet8bitVal(cmd,0x30);
	cmdSet8bitVal(cmd,0x01);
	
	cmdSet16bitVal(cmd,deviceTableIndex); //�����
	
	/*
	cmdSet8bitVal(cmd,temp);//7B0 �����¶�25��
	cmdSet8bitVal(cmd,windData);//7B1 ���÷���
	cmdSet8bitVal(cmd,windManual);//7B2 �����ֶ�����
	cmdSet8bitVal(cmd,windAuto);//7B3 ���������Զ�����
	cmdSet8bitVal(cmd,openData);//7B4 ���ÿ�������
	cmdSet8bitVal(cmd,keyData);//7B5 ���ü�����Ӧ����
	cmdSet8bitVal(cmd,arcMode);//7B6 ����ģʽ��Ӧ����
	*/
	
	cmdSetStringVal(cmd,devicekey,keyLength);
	uint8_t length = arc_table[deviceTableIndex][0];
	cmdSet8bitVal(cmd,length+1);
	cmdSetStringVal(cmd,(uint8_t *)&arc_table[deviceTableIndex][1],length);
	cmdSet8bitVal(cmd,0xff);
	PackageSetCheckSumFCS(cmd);
#endif
	return true;
}

/*
ԭʼ���ݷ��͹���:
	�ֻ�������ͨ��������wifi�������ģ�飬����ģ��ɼ�����, Ȼ��������ݲ����棺
2.1 ,�ֻ�����Է��ͣ�30H+20H+50H ,�����ֽ�,Ȼ����ԭ��ң������׼ת��ģ���ϵĺ��ⷢ��ܷ�һ����:����1��2��)
2.2  �ֻ�����Եȴ�������wifi���زɼ��������ݴ浽���飺learn_data_out2�ۣݣ�
2.3  ��ѧϰ����2��0x30��0x03,+���ݣ���2.2���õ�������ȥ����һ���ֽڣ���229���ֽڣ�+У��===��232�ֽڣ���ʱ�������أ�
*/
//��ѧϰ�������������͵�����ת����
int Creat_Irc_LearnCode_Packege(uint8_t *srcCode,uint16_t srcLength,hostCmd *cmd)
{
	if(srcCode == NULL)
		return -1;
	memset(cmd,0,sizeof(hostCmd));
	cmd->idx = 0;
	cmdSet8bitVal(cmd,0x30);
	cmdSet8bitVal(cmd,0x03);
	cmdSetStringVal(cmd,&srcCode[1],srcLength-1);
	PackageSetCheckSumFCS(cmd);
	return 0;
}
