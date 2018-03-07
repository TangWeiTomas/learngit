/**************************************************************************************************
 * Filename:       globalVal.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    定义全局变量的文件.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00    新建文件
 *
 */

/*********************************************************************
 * INCLUDES
 */

#include "globalVal.h"
/*********************************************************************
 * MACROS
 */

/*******************************WIFI Soc**************************************************/
uint8_t roomfairy_WifiMac[6];

/*******************************主机注册标记**********************************************/
bool roomfairy_registerFlag;

/*******************************标记用户是否使用智能系统**********************************/
bool usr_use_smartHotal_system = false;

/*******************************是否启动日志记录**********************************/
//bool use_log_msg	= false;	//设备日志(记录在/etc/config/msg.log)
bool use_log_debug  = false;	//调试日志
//bool use_log_syslog = false;	//syslog日志


