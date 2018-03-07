#ifndef __DEVICE_CODE_H__
#define __DEVICE_CODE_H__
#include "globalVal.h"

#define  remote_pjt_data_struct		uint8_t
#define  remote_fan_data_struct		uint8_t
#define  remote_stb_data_struct		uint8_t
#define  remote_dvd_data_struct		uint8_t
#define  remote_tv_data_struct		uint8_t
#define  remote_arc_data_struct		uint8_t
#define  remote_iptv_data_struct	uint8_t

typedef struct DeviceIndex{
	uint16_t index;
	uint8_t *name;
}DeviceIndex_t;

#if PJT_ENABLE
	typedef enum{
	PJT_KEY_OPEN 		=1 ,//开机
	PJT_KEY_CLOSE 		=3 ,//关机
	PJT_KEY_COMPUTER 	=5 ,//电脑
	PJT_KEY_VIDEO 		=7 ,//视频
	PJT_KEY_SIGNEL 		=9 ,//信号源
	PJT_KEY_ZOOM_UP 	=11 ,//变焦＋
	PJT_KEY_ZOOM_DOWN 	=13 ,//变焦－
	PJT_KEY_PICTURE_UP 	=15 ,//画面＋
	PJT_KEY_PICTURE_DOWN =17 ,//画面－
	PJT_KEY_MENU 		=19 ,//菜单
	PJT_KEY_ENTER 		=21 ,//确认
	PJT_KEY_UP 			=23 ,//上
	PJT_KEY_LEFT 		=25 ,//左
	PJT_KEY_RIGHT 		=27 ,//右
	PJT_KEY_DOWN 		=29 ,//下
	PJT_KEY_EXIT 		=31 ,//退出
	PJT_KEY_VOL_UP 		=33 ,//音量＋
	PJT_KEY_VOL_DOWN 	=35 ,//音量－
	PJT_KEY_MUTE 		=37 ,//静音
	PJT_KEY_AUTO 		=39 ,//自动
	PJT_KEY_PAUSE 		=41 ,//暂停
	PJT_KEY_BRIGHTNESS	=43 ,//亮度
	}PJT_KEY;
	extern	const remote_pjt_data_struct stb_pjt_table[][49];
	extern	const int remote_pjt_info[][112];
#endif

#if FAN_ENABLE
	typedef enum{
	FAN_KEY_ONOFF 			=1,//开关
	FAN_KEY_ON_WINDSPEED 	=3,//开/风速
	FAN_KEY_SHOOK_HEAD		=5,//摇头
	FAN_KEY_MODE 			=7,//风类(模式)
	FAN_KEY_TIMER 			=9,//定时
	FAN_KEY_LIGHT 			=11,//灯光
	FAN_KEY_ANION 			=13,//负离子
	FAN_KEY_NUM_1 			=15,//数字键1
	FAN_KEY_NUM_2 			=17,//数字键2
	FAN_KEY_NUM_3			=19,//数字键3
	FAN_KEY_NUM_4 			=21,//数字键4
	FAN_KEY_NUM_5 			=23,//数字键5
	FAN_KEY_NUM_6 			=25,//数字键6
	FAN_KEY_NUM_7 			=27,//数字键7
	FAN_KEY_NUM_8 			=29,//数字键8
	FAN_KEY_NUM_9 			=31,//数字键9
	FAN_KEY_SLEEP 			=33,//睡眠
	FAN_KEY_REFRIGERATION 	=35,//制冷
	FAN_KEY_AIR_VOLUME 		=37,//风量
	FAN_KEY_LOW_SPEED 		=39,//低速
	FAN_KEY_MID_SPEED 		=41,//中速
	FAN_KEY_HIGHT_SPEED 	=43,//高速
	}FAN_KEY;
	extern	const remote_fan_data_struct stb_fan_table[][49];
	extern	const int remote_fan_info[][149];
#endif

#if STB_ENABLE
	//机顶盒按键定义
	typedef enum{
		STB_KEY_STANDBY 		= 1, //待机键开机
		STB_KEY_NUM_1			= 3, //数字键1
		STB_KEY_NUM_2			= 5, //数字键2
		STB_KEY_NUM_3			= 7, //数字键3
		STB_KEY_NUM_4			= 9, //数字键4
		STB_KEY_NUM_5			= 11,//数字键5
		STB_KEY_NUM_6			= 13,//数字键6
		STB_KEY_NUM_7			= 15,//数字键7
		STB_KEY_NUM_8			= 17,//数字键8
		STB_KEY_NUM_9			= 19,//数字键9
		STB_KEY_GUIDE			= 21,//导视
		STB_KEY_NUM_0			= 23,//数字键0
		STB_KEY_BACK			= 25,//返回键
		STB_KEY_UP				= 27,//上
		STB_KEY_LEFT			= 29,//左
		STB_KEY_ENTER			= 31,//确定
		STB_KEY_RIGHT			= 33,//右
		STB_KEY_DOWN			= 35,//下
		STB_KEY_VOL_UP			= 37,//音量 +
		STB_KEY_VOL_DOWN		= 39,//音量－
		STB_KEY_CHANNEL_UP		= 41,//频道 +
		STB_KEY_CHANNEL_DOWN	= 43,//频道－
		STB_KEY_MENU			= 45//菜单键
	}STB_KEY;
	extern	const remote_stb_data_struct stb_data_table[][51];
	extern	const int remote_stb_info[][547];
#endif

#if DVD_ENABLE
	typedef enum{
	DVD_KEY_LEFT 		= 1,//左
	DVD_KEY_UP 			= 3,//上
	DVD_KEY_OK 			= 5,//OK
	DVD_KEY_DOWN 		= 7,//下
	DVD_KEY_RIGHT 		= 9,//右
	DVD_KEY_POWER 		= 11,//电源
	DVD_KEY_MUTE 		= 13,//静音
	DVD_KEY_BACKWARD 	= 15,//快倒
	DVD_KEY_PLAY 		= 17,//播放
	DVD_KEY_FORWARD 	= 19,//快进
	DVD_KEY_PREVIOUS 	= 21,//上一曲
	DVD_KEY_STOP 		= 23,//停止
	DVD_KEY_NEXT 		= 25,//下一曲
	DVD_KEY_SYS 		= 27,//制式
	DVD_KEY_PAUSE 		= 29,//暂停
	DVD_KEY_TITLE 		= 31,//标题
	DVD_KEY_SWITCH 		= 33,//开关仓
	DVD_KEY_MENU 		= 35,//菜单
	DVD_KEY_BACK 		= 37,//返回
	}DVD_KEY;
	extern	const remote_dvd_data_struct dvd_data_table[][43];
	extern	const int remote_dvd_info[][602];
	extern	const int remote_dvd_2_info[][2];
#endif 

#if TV_ENABLE
	//电视机按键定义
	typedef enum{
	TV_KEY_VOL_DOWN 	=  1,//音量－
	TV_KEY_CHANNEL_UP 	=  3,//频道＋
	TV_KEY_MENU 		=  5,//菜单
	TV_KEY_CHANNEL_DOWN	=  7,//频道－
	TV_KEY_VOL_UP 		=  9,//音量＋
	TV_KEY_POWER 		= 11,//电源
	TV_KEY_MUTE 		= 13,//静音
	TV_KEY_NUM_1 		= 15,//数字键1
	TV_KEY_NUM_2 		= 17,//数字键2
	TV_KEY_NUM_3 		= 19,//数字键3
	TV_KEY_NUM_4 		= 21,//数字键4
	TV_KEY_NUM_5 		= 23,//数字键5
	TV_KEY_NUM_6 		= 25,//数字键6
	TV_KEY_NUM_7 		= 27,//数字键7
	TV_KEY_NUM_8 		= 29,//数字键8
	TV_KEY_NUM_9 		= 31,//数字键9
	TV_KEY_JUMP 		= 33,//按键 -/--
	TV_KEY_NUM_0 		= 35,//数字键0
	TV_KEY_AV_TV 		= 37,//AV/TV
	TV_KEY_BACK 		= 39,//返回
	TV_KEY_ENTER 		= 41,//确认
	TV_KEY_UP 			= 43,//上
	TV_KEY_LEFT 		= 45,//左
	TV_KEY_RIGHT 		= 47,//右
	TV_KEY_DOWN 		= 49//下
	}TV_KEY;
	extern	const remote_tv_data_struct tv_table[][55];
	extern	const int TV_info[][2062];
	extern	const int remote_tv_info[][2];
#endif

#if ARC_ENABLE
	extern	const  remote_arc_data_struct arc_table[ ][76];
	extern	const int g_remote_arc_info[][832];
	extern	const int g_remote_arc_2_info[][2];
#endif 

#if IPTV_ENBALE
	typedef enum{
	IPTV_KEY_POWER 			= 1,//电源
	IPTV_KEY_MUTE 			= 3,//静音
	IPTV_KEY_VOL_UP 		= 5,//音量＋
	IPTV_KEY_VOL_DOWN 		= 7,//音量－
	IPTV_KEY_CHANNEL_UP 	= 9,//频道＋(主页)
	IPTV_KEY_CHANNEL_DOWN 	= 11,//频道－(菜单)
	IPTV_KEY_UP 			= 13,//上
	IPTV_KEY_LEFT 			= 15,//左
	IPTV_KEY_OK 			= 17,//OK
	IPTV_KEY_RIGHT 			= 19,//右
	IPTV_KEY_DOWN 			= 21,//下
	IPTV_KEY_PLAY_PAUSE 	= 23,//暂停/播放
	IPTV_KEY_NUM_1 			= 25,//数字键1
	IPTV_KEY_NUM_2 			= 27,//数字键2
	IPTV_KEY_NUM_3 			= 29,//数字键3
	IPTV_KEY_NUM_4 			= 31,//数字键4
	IPTV_KEY_NUM_5 			= 33,//数字键5
	IPTV_KEY_NUM_6 			= 35,//数字键6
	IPTV_KEY_NUM_7 			= 37,//数字键7
	IPTV_KEY_NUM_8 			= 39,//数字键8
	IPTV_KEY_NUM_9 			= 41,//数字键9
	IPTV_KEY_NUM_0 			= 43,//数字键0
	IPTV_KEY_BACK 			= 45//返回
	}IPTV_KEY;
	extern	const remote_iptv_data_struct remote_IPTV_table[][51];
	extern	const int remote_IPTV_info[][65];
	extern	const int remote_IPTV_2_info[][2];
#endif

//红外产品类型定义
typedef enum{
	IRC_DEVICE_TYPE_PJT  = 0x00, //投影仪
	IRC_DEVICE_TYPE_FAN  = 0x01, //风扇
	IRC_DEVICE_TYPE_STB  = 0x02, //机顶盒
	IRC_DEVICE_TYPE_DVD  = 0x03, //DVD
	IRC_DEVICE_TYPE_TV   = 0x04, //TV
	IRC_DEVICE_TYPE_IPTV = 0x05, //IPTV 网络机顶盒
	IRC_DEVICE_TYPE_ARC  = 0X06	//空调
}IRC_DEVICE_TYPE_t;

//=========================================================================

//产品类型数量
#define IRC_DEVICE_TYPE_SIZE	7

uint16_t Create_Irc_Query_Device_Number_Package(uint8_t deviceType,uint16_t deviceNameId);
uint16_t Create_Irc_Query_Package(uint8_t deviceType,uint16_t deviceNameId,uint16_t deviceModel,hostCmd *cmd);
uint8_t Create_Irc_Ctrl_Package(uint8_t deviceType,uint16_t deviceTableIndex,uint8_t keyLength,uint8_t* deviceKey,hostCmd *cmd);
int Creat_Irc_LearnCode_Packege(uint8_t *srcCode,uint16_t srcLength,hostCmd *cmd);

#endif
