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

/****************Zigbee Soc****************************************************/
//int serialPortFd = 0;


/****************WIFI Soc***************************************************/
uint8 roomfairy_WifiMac[6];
bool roomfairy_registerFlag=false;


//socketRecord_t *socketRecordHead = NULL;
//socketRecord_t *Remote_Socket = NULL;

char RemoteServerIPAddr[16]= {0};
uint16 RemoteServerPort=0;

//Local Socket Server
dataBuffer_t  localRecvData;


/***************Timer Task*************************************************************/
//CTimer *SocketHeartTimerID;
//CTimer *zbSocHeartTimerID;


/****************************************************************************/
pthread_mutex_t m_db_mutex= PTHREAD_MUTEX_INITIALIZER;












