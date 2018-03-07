/**************************************************************************************************
 * Filename:       globalVal.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    ����ȫ�ֱ������ļ�.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00    �½��ļ�
 *
 */

#ifndef GLOBAL_VAL_H
#define GLOBAL_VAL_H

#ifdef __cplusplus
extern "C" {
#endif
/*******************************INCLUDE**************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

//#include "zbSocCmd.h"
#include "comParse.h"
#include <pthread.h>
#include "Types.h"

/*******************************TYPEDEFS**************************************************/
//΢�ʱ
#define msleep(mirc)				\
		do{							\
			if(mirc < 1000)			\
				usleep(mirc*1000); 	\
		}while(0);

/*******************************Zigbee Soc************************************************/


/*******************************�̼��汾��Ϣ**********************************************/
//�̼��汾��
//��1λ:��ǰ�ܰ汾��
//��2λ:0���԰汾��1:�����汾
//��3λ:��ǰ�汾�ĵڼ����汾
//#define VERSION	"3.0.0"
#define VERSION	"3.0.7"
/*******************************ȫ��loop����**************************************************/
extern struct event_base *main_base_event_loop;

/*******************************WIFI Soc**************************************************/
extern uint8_t roomfairy_WifiMac[6];

/*******************************����ע����**********************************************/
extern bool roomfairy_registerFlag;

/*******************************�����������ܺ궨��****************************************/
//�豸��ʱʱ��
#define MAX_ONLINE_TIMEOUT_COUNTER     	3
//����������ʱ����
#define MAX_UART_TIMEOUT_COUNTER     	3

/*******************************ϵͳ��ʱ����������********************************************/
#define CONNECT_REPORT_TIME     		  	  SecTime(5)
#define HEART_PACKET_INTERVAL_TIME     		  SecTime(55)      //������55s һ��
#define CLIENT_HEART_PACKET_INTERVAL_TIME     SecTime(120)     //60s���һ��socket����
#define OUT_OF_POWER_TIME					  SecTime(300)	   //300s�������豸û�б����������������˶ϵ�ϵͳ
#define DEVICE_POWERVALUE_TIME     		      SecTime(7200)    // SecTime(7200)       //����2Сʱ����һ��

#define HEART_UART_REPORT_TIME     		  	  SecTime(30)		//����ÿ30s����һ����ѯ����

#define TCP_SEND_INTERVAL_TIME				  (500000)			//	usleep() 500ms

#define DEVICE_CMD_RESEND_INTERVAL_TIME		  SecTime(3)		//�豸�����ظ����

#define PANALRM_DEVICE_INCOMING_TIMER		  SecTime(90)  //�ϱ����60s

/*******************************�������ܺ궨��********************************************/
//��ѹ�Զ��ϱ�ʹ��λ
#define POWER_REPORT_ENBALE 			ENABLE
//���˶ϵ�ʹ��λ
#define OUT_POWER_ENBALE  				DISABLE
//��ϣ�����豸ʹ��λ
#define USE_MASTER_CONTROL				ENABLE
//���ڷ��������Ƿ����ת��
#define DISPOSE_ESC						DISABLE //0:��ת�� 1:ת��

/*******************************��⹦�ܺ궨��********************************************/
//�յ�һ��ƥ��ʹ��
#define ONE_KEY_MATCH_ARC_ENABLE 		ENABLE
//����һ��ƥ��ʹ��
#define ONE_KEY_MATCH_TV_ENABLE  		ENABLE
//������һ��ƥ��ʹ��
#define ONE_KEY_MATCH_STB_ENABLE 		ENABLE

#define STB_ENABLE 						ENABLE 	//������
#define TV_ENABLE  						ENABLE	//����
#define ARC_ENABLE 						ENABLE	//�յ�
#define IPTV_ENBALE 					ENABLE	//���������
#define FAN_ENABLE						DISABLE//����
#define PJT_ENABLE						DISABLE	//ͶӰ��
#define DVD_ENABLE						DISABLE	//DVD
/*******************************ϵͳ��־��¼�ļ�********************************************/
extern bool use_log_debug;
/*******************************���·���������********************************************/
#define URLSIZE				1024
#define FILE_NAME_SIZE		128
#define UPDATESERVER		"ftp://wx.feixuekj.cn/"
#define FWHEADNAME			"SmartHotalHost_fw_V"
#define CONFIGFILE			"SmartHotalHost.ini"

#define LOCAL_SERVER_TCP_PORT 8100

#define LOCAL_SERVER_UDP_PORT 8200

/*******************************��ά��������********************************************/
#define DEVICE_LIWEI_DOOR_SUPPERT 				ENABLE
#define DEVICE_LIWEI_DOOR_QUERY_TIME		  	SecTime(10)		//�ط�������
#define DEVICE_LIWEI_DOOR_OPEN_CNT				0
/*******************************����Ȩ�޹���********************************************/
#define PERMIMNG		DISABLE//�Ƿ�ʹ��

#if PERMIMNG
#ifdef OPENWRT_TEST
 #define PermiMngFilePath		"/etc/config/permisstion"
 #define PermiMngUrl			"wx.feixuekj.cn"
#else
 #define PermiMngFilePath		"./permisstion"
 #define PermiMngUrl			"192.168.1.68"
#endif

#define PermiMngPort			 8100
#define PermiMngKey				 PermiMngUrl
#define PermiMngRequestTime		 SecTime(86400)//���24*60*60 1������һ��
#define PermiMngRequestFaileTime SecTime(3600)
#define PermiMngRequestSendTime	 SecTime(300)
#endif

/*******************************signel factory********************************************/

#define SIGNEL_FACTORY			ENABLE//����signel factory���ܣ�������λ��Э����Ҳ��λ


/*******************************signel factory********************************************/

#ifdef __cplusplus
}

#endif

#endif /* GLOBAL_VAL_H */
