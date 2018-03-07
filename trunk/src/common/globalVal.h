/**************************************************************************************************
 * Filename:       globalVal.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    定义全局变量的文件.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00    新建文件
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
#include "GwComDef.h"

/*******************************TYPEDEFS**************************************************/
//微妙定时
#define msleep(mirc)				\
		do{							\
			if(mirc < 1000)			\
				usleep(mirc*1000); 	\
		}while(0);

/*******************************Zigbee Soc************************************************/


/*******************************固件版本信息**********************************************/
//固件版本号
//第1位:当前总版本号
//第2位:0测试版本，1:发布版本
//第3位:当前版本的第几个版本
//#define VERSION	"3.0.0"
#define VERSION	"3.0.14"
#define SUB_VERSION "3.0.12.0"

/*******************************全局loop队列**************************************************/
extern struct event_base *main_base_event_loop;

/*******************************WIFI Soc**************************************************/
extern uint8_t roomfairy_WifiMac[6];

/*******************************主机注册标记**********************************************/
extern bool roomfairy_registerFlag;

/*******************************心跳参数功能宏定义****************************************/
//设备超时时间
#define MAX_ONLINE_TIMEOUT_COUNTER     	3
#define MAX_ONLIEN_TIMEOUT_IAS_ZONE_CNT	70
//串口心跳超时次数
#define MAX_UART_TIMEOUT_COUNTER     		3

/*******************************系统定时任务间隔设置********************************************/
#define CONNECT_REPORT_TIME     		  	  	SecTime(5)
#define HEART_PACKET_INTERVAL_TIME     		  SecTime(55)      //心跳包55s 一次
#define CLIENT_HEART_PACKET_INTERVAL_TIME   SecTime(180)     //60s检测一次socket链接
#define OUT_OF_POWER_TIME					  				SecTime(300)	   //300s后其他设备没有被触发，则启动离人断电系统
#define DEVICE_POWERVALUE_TIME     		      SecTime(7200)    // SecTime(7200)       //电量2小时发送一次

#define HEART_UART_REPORT_TIME     		  	  SecTime(30)		//串口每30s发送一条查询命令

#define TCP_SEND_INTERVAL_TIME				  		(500000)			//	usleep() 500ms

#define DEVICE_CMD_RESEND_INTERVAL_TIME		  SecTime(3)		//设备命令重复间隔

#define PANALRM_DEVICE_INCOMING_TIMER		  	SecTime(90)  //上报间隔60s

/*******************************参数功能宏定义********************************************/
//电压自动上报使能位
#define POWER_REPORT_ENBALE 			ENABLE
//离人断电使能位
#define OUT_POWER_ENBALE  				DISABLE
//龙希主控设备使能位
#define USE_MASTER_CONTROL				ENABLE
//串口发送数据是否进行转义
#define DISPOSE_ESC								DISABLE //0:不转义 1:转义

/*******************************码库功能宏定义********************************************/
//空调一键匹配使能
#define ONE_KEY_MATCH_ARC_ENABLE 		ENABLE
//电视一键匹配使能
#define ONE_KEY_MATCH_TV_ENABLE  		ENABLE
//机顶盒一键匹配使能
#define ONE_KEY_MATCH_STB_ENABLE 		ENABLE

#define STB_ENABLE 						ENABLE 	//机顶盒
#define TV_ENABLE  						ENABLE	//电视
#define ARC_ENABLE 						ENABLE	//空调
#define IPTV_ENBALE 					ENABLE	//网络机顶盒
#define FAN_ENABLE						DISABLE //风扇
#define PJT_ENABLE						DISABLE	//投影仪
#define DVD_ENABLE						DISABLE	//DVD

/*******************************GPRS模块设置********************************************/

#define SUPPORT_GPRS_MODULE				0

/*******************************盈家项目定制宏********************************************/

#define SUPPORT_YINJIA_HEATBEAT				1
#define HEART_PACKET_YINJIA_TIME     		  SecTime(20)      //心跳包55s 一次		
/*******************************系统日志记录文件********************************************/
extern bool use_log_debug;
/*******************************更新服务器配置********************************************/
#define URLSIZE					1024
#define FILE_NAME_SIZE	128

#define UPDATESERVER		"ftp://wx.feixuekj.cn/"
#define FWHEADNAME			"SmartHotalHost_fw_V"
#define CONFIGFILE			"SmartHotalHosts.ini"

/*版本信息*/
#define FX_MODE_NAME		"/tmp/sysinfo/model"
#define FX_BOARD_NAME		"/tmp/sysinfo/board_name"

#define FX7620NA			"FX7620N-A"
#define FX7620NB			"FX7620N-B"
#define FX7620NC			"FX7620N-C"
#define FX7688				"fxSmartGw-d"

#define LOCAL_SERVER_TCP_PORT 8100
#define LOCAL_SERVER_UDP_PORT 8200

/*******************************力维门锁配置********************************************/
#define DEVICE_LIWEI_DOOR_SUPPERT 				ENABLE
#define DEVICE_LIWEI_DOOR_QUERY_TIME		  SecTime(10)		//重发命令间隔
#define DEVICE_LIWEI_DOOR_OPEN_CNT				0
/*******************************主机权限管理********************************************/
#define PERMIMNG		DISABLE//是否使能

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
#define PermiMngRequestTime		 SecTime(86400)//间隔24*60*60 1天请求一次
#define PermiMngRequestFaileTime SecTime(3600)
#define PermiMngRequestSendTime	 SecTime(300)
#endif

/*******************************signel factory********************************************/

#define SIGNEL_FACTORY			ENABLE//屏蔽signel factory功能，主机复位，协调器也复位

/*******************************版本号*******************************************/

#define SUPPORT_VERSION_BB		14	//"barrier_breaker"		
#define SUPPORT_VERSION_CC		15	//"chaos_calmer"

#define SUPPORT_VERSION			SUPPORT_VERSION_CC

#ifdef __cplusplus
}

#endif

#endif /* GLOBAL_VAL_H */
