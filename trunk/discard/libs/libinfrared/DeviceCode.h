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
	PJT_KEY_OPEN 		=1 ,//����
	PJT_KEY_CLOSE 		=3 ,//�ػ�
	PJT_KEY_COMPUTER 	=5 ,//����
	PJT_KEY_VIDEO 		=7 ,//��Ƶ
	PJT_KEY_SIGNEL 		=9 ,//�ź�Դ
	PJT_KEY_ZOOM_UP 	=11 ,//�佹��
	PJT_KEY_ZOOM_DOWN 	=13 ,//�佹��
	PJT_KEY_PICTURE_UP 	=15 ,//���棫
	PJT_KEY_PICTURE_DOWN =17 ,//���棭
	PJT_KEY_MENU 		=19 ,//�˵�
	PJT_KEY_ENTER 		=21 ,//ȷ��
	PJT_KEY_UP 			=23 ,//��
	PJT_KEY_LEFT 		=25 ,//��
	PJT_KEY_RIGHT 		=27 ,//��
	PJT_KEY_DOWN 		=29 ,//��
	PJT_KEY_EXIT 		=31 ,//�˳�
	PJT_KEY_VOL_UP 		=33 ,//������
	PJT_KEY_VOL_DOWN 	=35 ,//������
	PJT_KEY_MUTE 		=37 ,//����
	PJT_KEY_AUTO 		=39 ,//�Զ�
	PJT_KEY_PAUSE 		=41 ,//��ͣ
	PJT_KEY_BRIGHTNESS	=43 ,//����
	}PJT_KEY;
	extern	const remote_pjt_data_struct stb_pjt_table[][49];
	extern	const int remote_pjt_info[][112];
#endif

#if FAN_ENABLE
	typedef enum{
	FAN_KEY_ONOFF 			=1,//����
	FAN_KEY_ON_WINDSPEED 	=3,//��/����
	FAN_KEY_SHOOK_HEAD		=5,//ҡͷ
	FAN_KEY_MODE 			=7,//����(ģʽ)
	FAN_KEY_TIMER 			=9,//��ʱ
	FAN_KEY_LIGHT 			=11,//�ƹ�
	FAN_KEY_ANION 			=13,//������
	FAN_KEY_NUM_1 			=15,//���ּ�1
	FAN_KEY_NUM_2 			=17,//���ּ�2
	FAN_KEY_NUM_3			=19,//���ּ�3
	FAN_KEY_NUM_4 			=21,//���ּ�4
	FAN_KEY_NUM_5 			=23,//���ּ�5
	FAN_KEY_NUM_6 			=25,//���ּ�6
	FAN_KEY_NUM_7 			=27,//���ּ�7
	FAN_KEY_NUM_8 			=29,//���ּ�8
	FAN_KEY_NUM_9 			=31,//���ּ�9
	FAN_KEY_SLEEP 			=33,//˯��
	FAN_KEY_REFRIGERATION 	=35,//����
	FAN_KEY_AIR_VOLUME 		=37,//����
	FAN_KEY_LOW_SPEED 		=39,//����
	FAN_KEY_MID_SPEED 		=41,//����
	FAN_KEY_HIGHT_SPEED 	=43,//����
	}FAN_KEY;
	extern	const remote_fan_data_struct stb_fan_table[][49];
	extern	const int remote_fan_info[][149];
#endif

#if STB_ENABLE
	//�����а�������
	typedef enum{
		STB_KEY_STANDBY 		= 1, //����������
		STB_KEY_NUM_1			= 3, //���ּ�1
		STB_KEY_NUM_2			= 5, //���ּ�2
		STB_KEY_NUM_3			= 7, //���ּ�3
		STB_KEY_NUM_4			= 9, //���ּ�4
		STB_KEY_NUM_5			= 11,//���ּ�5
		STB_KEY_NUM_6			= 13,//���ּ�6
		STB_KEY_NUM_7			= 15,//���ּ�7
		STB_KEY_NUM_8			= 17,//���ּ�8
		STB_KEY_NUM_9			= 19,//���ּ�9
		STB_KEY_GUIDE			= 21,//����
		STB_KEY_NUM_0			= 23,//���ּ�0
		STB_KEY_BACK			= 25,//���ؼ�
		STB_KEY_UP				= 27,//��
		STB_KEY_LEFT			= 29,//��
		STB_KEY_ENTER			= 31,//ȷ��
		STB_KEY_RIGHT			= 33,//��
		STB_KEY_DOWN			= 35,//��
		STB_KEY_VOL_UP			= 37,//���� +
		STB_KEY_VOL_DOWN		= 39,//������
		STB_KEY_CHANNEL_UP		= 41,//Ƶ�� +
		STB_KEY_CHANNEL_DOWN	= 43,//Ƶ����
		STB_KEY_MENU			= 45//�˵���
	}STB_KEY;
	extern	const remote_stb_data_struct stb_data_table[][51];
	extern	const int remote_stb_info[][547];
#endif

#if DVD_ENABLE
	typedef enum{
	DVD_KEY_LEFT 		= 1,//��
	DVD_KEY_UP 			= 3,//��
	DVD_KEY_OK 			= 5,//OK
	DVD_KEY_DOWN 		= 7,//��
	DVD_KEY_RIGHT 		= 9,//��
	DVD_KEY_POWER 		= 11,//��Դ
	DVD_KEY_MUTE 		= 13,//����
	DVD_KEY_BACKWARD 	= 15,//�쵹
	DVD_KEY_PLAY 		= 17,//����
	DVD_KEY_FORWARD 	= 19,//���
	DVD_KEY_PREVIOUS 	= 21,//��һ��
	DVD_KEY_STOP 		= 23,//ֹͣ
	DVD_KEY_NEXT 		= 25,//��һ��
	DVD_KEY_SYS 		= 27,//��ʽ
	DVD_KEY_PAUSE 		= 29,//��ͣ
	DVD_KEY_TITLE 		= 31,//����
	DVD_KEY_SWITCH 		= 33,//���ز�
	DVD_KEY_MENU 		= 35,//�˵�
	DVD_KEY_BACK 		= 37,//����
	}DVD_KEY;
	extern	const remote_dvd_data_struct dvd_data_table[][43];
	extern	const int remote_dvd_info[][602];
	extern	const int remote_dvd_2_info[][2];
#endif 

#if TV_ENABLE
	//���ӻ���������
	typedef enum{
	TV_KEY_VOL_DOWN 	=  1,//������
	TV_KEY_CHANNEL_UP 	=  3,//Ƶ����
	TV_KEY_MENU 		=  5,//�˵�
	TV_KEY_CHANNEL_DOWN	=  7,//Ƶ����
	TV_KEY_VOL_UP 		=  9,//������
	TV_KEY_POWER 		= 11,//��Դ
	TV_KEY_MUTE 		= 13,//����
	TV_KEY_NUM_1 		= 15,//���ּ�1
	TV_KEY_NUM_2 		= 17,//���ּ�2
	TV_KEY_NUM_3 		= 19,//���ּ�3
	TV_KEY_NUM_4 		= 21,//���ּ�4
	TV_KEY_NUM_5 		= 23,//���ּ�5
	TV_KEY_NUM_6 		= 25,//���ּ�6
	TV_KEY_NUM_7 		= 27,//���ּ�7
	TV_KEY_NUM_8 		= 29,//���ּ�8
	TV_KEY_NUM_9 		= 31,//���ּ�9
	TV_KEY_JUMP 		= 33,//���� -/--
	TV_KEY_NUM_0 		= 35,//���ּ�0
	TV_KEY_AV_TV 		= 37,//AV/TV
	TV_KEY_BACK 		= 39,//����
	TV_KEY_ENTER 		= 41,//ȷ��
	TV_KEY_UP 			= 43,//��
	TV_KEY_LEFT 		= 45,//��
	TV_KEY_RIGHT 		= 47,//��
	TV_KEY_DOWN 		= 49//��
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
	IPTV_KEY_POWER 			= 1,//��Դ
	IPTV_KEY_MUTE 			= 3,//����
	IPTV_KEY_VOL_UP 		= 5,//������
	IPTV_KEY_VOL_DOWN 		= 7,//������
	IPTV_KEY_CHANNEL_UP 	= 9,//Ƶ����(��ҳ)
	IPTV_KEY_CHANNEL_DOWN 	= 11,//Ƶ����(�˵�)
	IPTV_KEY_UP 			= 13,//��
	IPTV_KEY_LEFT 			= 15,//��
	IPTV_KEY_OK 			= 17,//OK
	IPTV_KEY_RIGHT 			= 19,//��
	IPTV_KEY_DOWN 			= 21,//��
	IPTV_KEY_PLAY_PAUSE 	= 23,//��ͣ/����
	IPTV_KEY_NUM_1 			= 25,//���ּ�1
	IPTV_KEY_NUM_2 			= 27,//���ּ�2
	IPTV_KEY_NUM_3 			= 29,//���ּ�3
	IPTV_KEY_NUM_4 			= 31,//���ּ�4
	IPTV_KEY_NUM_5 			= 33,//���ּ�5
	IPTV_KEY_NUM_6 			= 35,//���ּ�6
	IPTV_KEY_NUM_7 			= 37,//���ּ�7
	IPTV_KEY_NUM_8 			= 39,//���ּ�8
	IPTV_KEY_NUM_9 			= 41,//���ּ�9
	IPTV_KEY_NUM_0 			= 43,//���ּ�0
	IPTV_KEY_BACK 			= 45//����
	}IPTV_KEY;
	extern	const remote_iptv_data_struct remote_IPTV_table[][51];
	extern	const int remote_IPTV_info[][65];
	extern	const int remote_IPTV_2_info[][2];
#endif

//�����Ʒ���Ͷ���
typedef enum{
	IRC_DEVICE_TYPE_PJT  = 0x00, //ͶӰ��
	IRC_DEVICE_TYPE_FAN  = 0x01, //����
	IRC_DEVICE_TYPE_STB  = 0x02, //������
	IRC_DEVICE_TYPE_DVD  = 0x03, //DVD
	IRC_DEVICE_TYPE_TV   = 0x04, //TV
	IRC_DEVICE_TYPE_IPTV = 0x05, //IPTV ���������
	IRC_DEVICE_TYPE_ARC  = 0X06	//�յ�
}IRC_DEVICE_TYPE_t;

//=========================================================================

//��Ʒ��������
#define IRC_DEVICE_TYPE_SIZE	7

uint16_t Create_Irc_Query_Device_Number_Package(uint8_t deviceType,uint16_t deviceNameId);
uint16_t Create_Irc_Query_Package(uint8_t deviceType,uint16_t deviceNameId,uint16_t deviceModel,hostCmd *cmd);
uint8_t Create_Irc_Ctrl_Package(uint8_t deviceType,uint16_t deviceTableIndex,uint8_t keyLength,uint8_t* deviceKey,hostCmd *cmd);
int Creat_Irc_LearnCode_Packege(uint8_t *srcCode,uint16_t srcLength,hostCmd *cmd);

#endif
