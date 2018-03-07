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

//投影仪设备名称索引
const DeviceIndex_t PJT_index_info[] = 
{
	{0,"爱普生(Epson)"},
	{1,"东芝(Toshiba)"},
	{2,"夏普(Sharp)"},
	{3,"松下(Panasonic)"},
	{4,"方正(Fangzheng)"},
	{5,"富士通(Fujitsu)"},
	{6,"日立(Hitachi)"},
	{7,"索尼(Sony)"},
	{8,"三洋(Sanyo)"},
	{9,"奥图码(Optoma)"},
	{10,"NEC(NEC)"},
	{11,"三菱(Mitsubishi)"},
	{12,"富可视(InFocus)"},
	{13,"中宝(Zhongbao)"},
	{14,"瑞诚(Rui Cheng"},
	{15,"some(some)"},
	{16,"惠普(HP)"},
	{17,"戴尔(Dell)"},
	{18,"佳能(Canon)"},
	{19,"宏基(Acer)"},
	{20,"明基(BenQ)"},
	{21,"先峰(Pioneer)"},
	{22,"晨星(Morningstar"},
	{23,"宝丽莱(The Polaroid)"},
	{24,"爱其(Aiqi)"},
	{25,"LG(LG)"},
	{26,"拍得丽(PREMIER)"},
	{27,"飞利浦(Philips)"},
	{28,"优派(ViewSonic)"},
	{29,"其它品牌"}
};

//电视名称索引
const DeviceIndex_t Tv_index_info[] = 
{
	{0, "长虹"},
	{1, "康佳"},
	{2, "TCL"},
	{3, "厦华"},
	{4, "创维(SKYWORTH)"},
	{5,	"海信(Hisense)"},
	{6,	"海尔(Haier)"},
	{7,	"金星(JINXING)"},
	{8,	"熊猫(PANDA)"},
	{9,	"索尼(sony)"},
	{10,"松下(Panasonic)"},
	{11,"乐声(Panasonic)"},
	{12,"东芝(TOSHIBA)"},
	{13,"日立(HITACHI)"},
	{14,"夏普(SHARP)"},
	{15,"声宝(Sampo)"},
	{16,"飞利浦(Philips)"},
	{17,"三星(SAMSUNG)"},
	{18,"三洋(SANYO)"},
	{19,"日电(Xinhua)"},
	{20,"西湖(XIHU)"},
	{21,"北京(BEIJING)"},
	{22,"高路华(CONROWA)"},
	{23,"乐华(ROWA)"},
	{24,"东宝TCBO_东凌TOS"},
	{25,"福日(Furi)"},
	{26,"LG(LG)"},
	{27,"百乐(Tupper)"},
	{28,"长城(CHANGCHENG)"},
	{29,"黄河(Yellow River)"},
	{30,"黄山(Huangshan)"},
	{31,"其它(Other)"}, 
	{32,"三垦(sanken)"},
	{33,"宝立创(Polytron)"},
	{34,"爱华(Aiwa)"},
	{35,"清华同方(Tongfang)"},
	{36,"先锋(Pioneer TV)"},
	{37,"乐视(LeTV)"},
	{38,"夏新(Amoi)"},
	{39,"小米(Xiaomi)"},
	{40,"康冠(Horlon)"},
};

//机顶盒索引
const DeviceIndex_t STB_index_info[] = 
{
    {0,"北京(Beijing)"},
    {1,"广东(Guangdong)"},
    {2,"广西(Guangxi)"},
    {3,"上海/东方有线(Shanghai / Oriental Cable)"},
    {4,"天津(Tianjin)"},
    {5,"重庆(Chongqing)"},
    {6,"辽宁(LIAONING)"},
    {7,"江苏(JIANGSU)"},
    {8,"湖北(Hubei)"},
    {9,"四川(Sichuan)"},
    {10,"陕西(Shaanxi)"},
    {11,"浙江(Zhejiang)"},
    {12,"湖南(Hunan)"},
    {13,"山东(Shandong)"},
    {14,"安徽(Anhui)"},
    {15,"贵州(Guizhou)"},
    {16,"黑龙江(Heilongjiang)"},
    {17,"山西(Shanxi)"},
	{18,"内蒙(Inner Mongolia)"},
	{19,"云南(Yunnan)"},
	{20,"河南(Henan)"},
	{21,"海南(Hainan)"},
    {22,"吉林(Jilin)"},
    {23,"河北(Hebei)"},
	{24,"福建(Fujian)"},
	{25,"新疆(Xinjiang)"},
	{26,"江西(Jiangxi)"},
	{27,"东太(East Pacific)"},
    {28,"其它城市(Other regions)"},
    {29,"中星九号(Star on the 9th)"},
	{30,"甘肃(Gansu)"},
	{31,"香港(Hong Kong)"},
	{32,"台湾(Taiwan)"},
	{33,"华为机顶盒(Huawei STB)"},
	{34,"长虹机顶盒(Changhong STB)"},
	{35," 康佳机顶盒(Konka STB)"},
	{36,"中兴机顶盒(ZTE STB)"},
	{37,"九洲机顶盒(Jiuzhou STB)"},     
	{38,"摩托罗拉机顶盒(Motorola STB)"},
	{39,"创维机顶盒(Skyworth STB)"},
	{40," 海信机顶盒(Hisense STB)"},
	{41,"海尔机顶盒(Haier STB)"},
	{42,"高斯贝尔机顶盒(GOSPELL STB)"},
	{43,"海美迪机顶盒(Hoi Mei Di STB)"},
	{44,"开博尔机顶盒(Open Bor STB)"},
	{45,"天柏机顶盒(DVN STB)"},
	{46,"大显机顶盒(Daxian STB)"},
	{47,"乐视机顶盒(Music as STB)"},
	{48,"斯达康机顶盒(Starcom STB)"},
	{49,"银河机顶盒(Galaxy STB)"},
	{50,"九联(Nine joint)"},
	{51,"歌华高清机顶盒(Gehua STB)"},
	{52,"天威机顶盒(Tianwei STB)"},
	{53,"夏华机顶盒(Hua Xia STB)"},
	{54,"佳创机顶盒(Jia Chong STB)"},
	{55,"佳彩机顶盒(Jia Cai STB)"},
	{56,"金网通机顶盒(Gold Netcom STB)"},
	{57,"聚友网络(Juyou Network)"   },
	{58,"美如画机顶盒(U.S. picturesque STB)"},
	{59,"东方广视(East WideSight)"},
	{60,"松下机顶盒(Panasonic STB)"},
	{61,"上广电机顶盒(SVA STB)"},
	{62,"北广机顶盒(BBEF STB"},
	{63,"全景机顶盒(Panoramic STB)"},
	{64,"航科机顶盒(Air Division STB)"},
	{65,"力合机顶盒(Leaguer STB)"},
	{66,"百胜机顶盒(Yum STB)/MATRIX"},
	{67,"MATRIX(MATRIX)"},
	{68,"浪潮机顶盒(Wave)"},
	{69,"深圳天威视讯(TOPWAY)"},
    {70,"北京歌华有线(Gehua)"},
    {71,"户户通(Huhutong)"},
    {72,"小米盒子(Millet box)"}
};

//空调设备索引
const DeviceIndex_t ARC_index_info[] =      
{
	{0,"美的(Midea)"},
	{1,"东芝(Toshiba)"},
	{2,"海尔(Haier)"},
	{3,"格力(Gree)"},
	{4,"志高(Chigo)"},
	{5,"奥克斯(Oaks)"},
	{6,"春兰(Chunlan)"},
	{7,"LG(LG)"},
	{8,"TCL(TCL)"},
	{9,"三星(Samsung)"},
	{10,"小三星(Small Samsung)"},
	{11,"三洋(Sanyo)"},
	{12,"三凌重工(Sanling Heavy Industries)"},
	{13,"三凌机电(Sanling Electrical)"},
	{14,"长虹(Changhong)"},
	{15,"松下(Panasonic)"},
	{16,"乐声(Sound of music)"},
	{17,"内田(Uchida)"},
	{18,"海信(Hisense)"},
	{19,"新科(Shinco)"},
	{20,"日立(Hitachi)"},
	{21,"华凌(Hualing)"},
	{22,"科龙(Kelon)"},
	{23,"华宝(Warburg)"},
	{24,"威力(Might)"},
	{25,"长岭(Changling)"},
	{26,"迎燕(Ying Yan)"},
	{27,"蓝波(Blue Wave)"},
	{28,"玉兔(Rabbit)"},
	{29,"开利(Carrier)"},
	{30,"东宝(Toho)"},
	{31,"高路华(Gaoluhua)"},
	{32,"澳柯玛(Aucma)"},
	{33,"胜风1(Shengfeng)"},
	{34,"扬子(Yangzi)"},
	{35,"惠而浦(Whirlpool)"},
	{36,"大金(Daikin)"},
	{37," 汇丰(HSBC)"},
	{38,"东芝2(Toshiba)"},
	{39," 伊莱克斯(Electrolux)"},
	{40,"熊猫(Panda)"},
	{41,"上凌(Ling on)"},
	{42,"天元(Tianyuan)"},
	{43,"其它品牌(Other brands)"},
	{44,"百合(Lily)"},
	{45,"SimpLlcrty(SimpLlcrty)"},
	{46,"大宇(Daewoo)"},
	{47,"冠远(Clarent)"},
	{48,"科朗(Colones)"},
	{49,"南风(Southwind)"},
	{50,"群达(Kunda)"},
	{51,"先科(Yushchenko)"},
	{52,"新飞(Freenet)"},
	{53,"胜风2(Shengfeng)"},
	{54,"泰国(Thailand)"},
	{55,"格兰仕(Glanz)"},
	{56,"乐华(Leroy)"},
	{57,"富士通(Fujitsu)"},
	{58,"约克(York)"},
	{59,"麦克维尔(McQuay)"},
	{60,"三垦(sanken)"},
	{61," 宝立创(polytron)"},
	{62," 夏普(SHARP)"},
	{63," 镇堡(ZHEN BAO)"},
	{64," 万宝(WANBAO)"},
	{65," 樱花(YINGHUA)"}
};

//网络机顶盒
const DeviceIndex_t IPTV_index_info[] =  
{
	{0," 烽火(beacon)"},
	{1," 华为(Huawei)"},
	{2," 斯达康(UTS)"},
	{3," 同洲(states)"},
	{4," 中兴(ZTE)"},
	{5," 大亚(Daya)"},
	{6," 长虹(Changhong)"}
};
//=========================================
//设备类型数量
#define PJT_DEVICE_SIZE 	30
#define	FAN_DEVICE_SIZE		22
#define	STB_DEVICE_SIZE 	73
#define	DVD_DEVICE_SIZE		161
#define TV_DEVICE_SIZE		41 
#define	IPTV_DEVICE_SIZE	7
#define ARC_DEVICE_SIZE		66	//空调设备数量
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

//设备码库类型数量
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
//设备电源键位于码库的位置
#define PJT_POWER_INDEX_START 		1
#define	FAN_POWER_INDEX_START		1
#define	STB_POWER_INDEX_START 		1
#define	DVD_POWER_INDEX_START		11
#define TV_POWER_INDEX_START		11 
#define	IPTV_POWER_INDEX_START		1

//=========================================
//COM码位于码库的位置
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


//获取设备的索引值
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

//获取设备的索引值
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

//获取设备的数量
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


//将电视等红外设备进行数据封包，红外设备搜寻
uint16_t Create_Irc_Query_Package(uint8_t deviceType,uint16_t deviceNameId,uint16_t deviceModel,hostCmd *cmd)
{
	uint16_t DataTableIndex = -1;//存储设备的红外码索引变化
	if(deviceType > IRC_DEVICE_TYPE_SIZE)
		return -1;
	
	//参数检测，检测deviceNameId 和deviceModel
	if(GetDeviceSize(deviceType,deviceNameId,deviceModel) > 0)
	{
		DataTableIndex = GetDeviceIndexInTable(deviceType,deviceNameId,deviceModel);
		if(DataTableIndex >= 0)
		{
			if(deviceType == IRC_DEVICE_TYPE_ARC) //空调类设备
			{
				Create_Irc_Arc_Query_Package(deviceNameId,deviceModel,cmd);
			}
			else//非空调类设备
			{
				uint8_t keyIndex = GetDevicePowerKey(deviceType);
				log_debug("PowerKey = %x \n",keyIndex);
				Generate_Irc_Package(deviceType,DataTableIndex,1,&keyIndex,cmd);//生成码值
			}
		}
	}
	return DataTableIndex;
}



//将电视等红外设备进行数据封包，红外设备搜寻
uint16_t Create_Irc_Query_Device_Number_Package(uint8_t deviceType,uint16_t deviceNameId)
{
	if(deviceType > IRC_DEVICE_TYPE_SIZE)
		return false;
	return GetDeviceSize(deviceType,deviceNameId,0);
}

//讲电视的红外进行封包
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
	
	if(deviceType == IRC_DEVICE_TYPE_ARC ) //空调类设备
	{
		ret = Create_Irc_Arc_Ctr_Package(deviceTableIndex,keyLength,deviceKey,cmd);
	}
	else	//非空调类设备
	{
		ret = Generate_Irc_Package(deviceType,deviceTableIndex,keyLength,deviceKey,cmd);//生成码值
	}
	
	return ret;
}

//空调设备
uint8_t Create_Irc_Arc_Query_Package(uint16_t deviceNameId,uint16_t deviceModel,hostCmd *cmd)
{
#if ARC_ENABLE
	uint16_t deviceNum = 0; 
	uint16_t DataTableIndex = 0;
	if(deviceNameId > ARC_DEVICE_SIZE) //大于设备种类数量
		return false;
	
	deviceNum = g_remote_arc_info[deviceNameId][0];

	if(deviceModel > deviceNum)
		return false;
	
	//获取码组号
	DataTableIndex = g_remote_arc_info[deviceNameId][deviceModel];

	if(DataTableIndex > ARC_TABLE_LIST_SIZE)
		return false;
	
	memset(cmd,0,sizeof(hostCmd));
	cmdSet8bitVal(cmd,0x30);
	cmdSet8bitVal(cmd,0x01);
	cmdSet16bitVal(cmd,DataTableIndex); //码组号
	cmdSet8bitVal(cmd,0x1b);//7B0 设置温度25度
	cmdSet8bitVal(cmd,0x01);//7B1 设置风量
	cmdSet8bitVal(cmd,0x02);//7B2 设置手动风向
	cmdSet8bitVal(cmd,0x01);//7B3 设置设置自动风向
	cmdSet8bitVal(cmd,0x01);//7B4 设置开关数据	//开启
	cmdSet8bitVal(cmd,0x01);//7B5 设置键名对应数据
	cmdSet8bitVal(cmd,0x05);//7B6 设置模式对应数据

	uint8_t length = arc_table[DataTableIndex][0];
	cmdSet8bitVal(cmd,length+1);
	cmdSetStringVal(cmd,(uint8_t *)&arc_table[DataTableIndex][1],length);
	cmdSet8bitVal(cmd,0xff);
	PackageSetCheckSumFCS(cmd);
#endif
	return true;
}


//空调设备控制
uint8_t Create_Irc_Arc_Ctr_Package(uint16_t deviceTableIndex,uint8_t keyLength,uint8_t*devicekey,hostCmd *cmd)
{
#if ARC_ENABLE
	uint16_t deviceNum = -1; 

	if(deviceTableIndex > ARC_TABLE_LIST_SIZE)
		return false;
	
	memset(cmd,0,sizeof(hostCmd));
	cmdSet8bitVal(cmd,0x30);
	cmdSet8bitVal(cmd,0x01);
	
	cmdSet16bitVal(cmd,deviceTableIndex); //码组号
	
	/*
	cmdSet8bitVal(cmd,temp);//7B0 设置温度25度
	cmdSet8bitVal(cmd,windData);//7B1 设置风量
	cmdSet8bitVal(cmd,windManual);//7B2 设置手动风向
	cmdSet8bitVal(cmd,windAuto);//7B3 设置设置自动风向
	cmdSet8bitVal(cmd,openData);//7B4 设置开关数据
	cmdSet8bitVal(cmd,keyData);//7B5 设置键名对应数据
	cmdSet8bitVal(cmd,arcMode);//7B6 设置模式对应数据
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
原始数据发送规则:
	手机，电脑通过蓝牙或wifi发命令给模块，启动模块采集数据, 然后读回数据并保存：
2.1 ,手机或电脑发送：30H+20H+50H ,三个字节,然后用原来遥控器对准转发模块上的红外发射管发一个码:长按1到2Ｓ)
2.2  手机或电脑等待蓝牙或wifi发回采集到的数据存到数组：learn_data_out2［］：
2.3  发学习数据2：0x30，0x03,+数据（第2.2步得到的数据去掉第一个字节：得229个字节）+校验===共232字节，用时按键发回；
*/
//将学习到的码封包，发送到红外转发器
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
