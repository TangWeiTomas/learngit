/**************************************************************************************************
 * Filename:       cmdMsgHandle.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-02,16:40)    :   Create the file.
 *
 *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <pthread.h>
#include <sys/msg.h>

#include "Types.h"
#include "cJSON.h"
#include "hal_defs.h"
#include "hal_types.h"

#include "comMsgPrivate.h"
//#include "cmdMsgSend.h"
#include "globalVal.h"

/************************************************************************
* 函数名 :HeartPacketSend(void)
* 描述   :   发送心跳包
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void HeartPacketSend(void)
{
    uint8 allDevNum;
    uint8 buf[1000];
    uint16 bufLen;

    LOG_DEBUG("HeartPacketSend++ \n");

    zigbeeDev_getAllRegisterOnlineDevices(&allDevNum,buf,&bufLen);
    LOG_DEBUG("allDevNum=%d.\n",allDevNum);
    zigbeeDev_heartPacketInd(allDevNum,buf,bufLen);

    LOG_DEBUG("HeartPacketSend-- \n");

    return;
}

