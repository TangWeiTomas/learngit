/**************************************************************************************************
 * Filename:       localServerHandle.c
 * Author:             zxb      
 * E-Mail:          zxb@yystart.com 
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/ 
 * 
 * Version:         1.00  (2014-12-06,14:30)    :   Create the file.
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

#include "cJSON.h"
#include "hal_defs.h"
#include "hal_types.h"

#include "interface_srpcserver.h"
#include "comMsgPrivate.h"
//#include "cmdMsgSend.h"

#include "zbSocCmd.h"
#include "errorCode.h"
#include "localCmdMng.h"
#include "localTimerHandle.h"
#include "globalVal.h"
#include "localServerHandle.h"
#include "socket_server.h"


/************************************************************************
* 函数名 :local_Server_DataInit(void)
* 描述   :  初始化本地Server数据
* 输入   ：
* 输出   ：无
* 返回   ：
************************************************************************/
void local_Server_DataInit(void)
{
    
    //初始化接收的数据包
    memset(&localRecvData,0,sizeof(localRecvData));
    
}

/***************************************************************************************************
 * @fn      local_Server_RxCB
 *
 * @brief   Callback for Rx'ing SRPC messages.
 *
 * @return  Status
 ***************************************************************************************************/
void local_Server_RxCB(int clientFd)
{
    char buffer[256];
    int byteToRead;
    int byteRead;
    int rtn;
    hostCmd cmd;

    printf("local_Server_RxCB++[%x]\n", clientFd);

    rtn = ioctl(clientFd, FIONREAD, &byteToRead);

    if (rtn != 0)
    {
        printf("local_Server_RxCB: Socket error\n");
    }

    if(byteToRead<=0) return;
    
    byteRead = read(clientFd, &localRecvData.buffer[localRecvData.bufferNum], (MaxCacheBufferLength-localRecvData.bufferNum));
    
    if (byteRead < 0)
    {
        printf("[local_Server_RxCB]:Error reading from socket\n");
    }
    else if (byteRead == 0)
    {
        //出现连接已经断开
        printf("[local_Server_RxCB]:The Server Have close\n");
    }
    else
    {
       localRecvData.bufferNum += byteRead;
    }

    while (localRecvData.bufferNum >= MinCmdPacketLength)
    {
        uint16 packetHeaderPos=0;
        uint16 cmdPacketLen;
        uint8 cmdPacketFCS;
        bzero(&cmd, sizeof(cmd));
        
        if(lookupFirstPacketHeader(localRecvData.buffer,localRecvData.bufferNum,&packetHeaderPos) == FindNoHeader)
        {
            local_Server_DataInit();
            break;
        }
        
        //丢弃非法包头数据
        if(packetHeaderPos != 0)
        {
            memmove(localRecvData.buffer,&localRecvData.buffer[packetHeaderPos],(localRecvData.bufferNum-packetHeaderPos));
            localRecvData.bufferNum -= packetHeaderPos;
        }
        
        cmdPacketLen = (((localRecvData.buffer[CMD_MSG_LEN0_POS]<<8)&0xff00)|(localRecvData.buffer[CMD_MSG_LEN1_POS]&0x00ff));
    
        printf("[localServer]: Message cmd length = %d recvData.bufferNum=%d\n",cmdPacketLen,localRecvData.bufferNum);
    
        // 2byte(TP)+6byte(Mac)+1byte(Dir)+2byte(D0,D1)
        if((cmdPacketLen+4) > localRecvData.bufferNum)
        {
            printf("[localServer]: Packet length error,Wait to receive other datas\n");
            break;
        }
    
        if(checkPacketFCS(&localRecvData.buffer[CMD_MSG_LEN0_POS],cmdPacketLen+3)==true)
        {
            memcpy(cmd.data,&localRecvData.buffer[CMD_MSG_TP_POS],cmdPacketLen);
            cmd.size = cmdPacketLen;
            cmd.idx = 0;

            printf("[localServer]: SRPC_ProcessIncoming 11111111111111\n");
            pthread_mutex_lock(&m_db_mutex);
            SRPC_ProcessIncoming(&cmd);            
            pthread_mutex_unlock(&m_db_mutex);
            
        }
        else
        {
            printf("[localServer]:Drop the error fcs data packet.\n");
        }
    
        if(localRecvData.bufferNum>=(cmdPacketLen+4))
        {
            memmove(localRecvData.buffer,&localRecvData.buffer[cmdPacketLen+4],(localRecvData.bufferNum-(cmdPacketLen+4)));
            localRecvData.bufferNum -= (cmdPacketLen+4);
        }
        else
        {
            local_Server_DataInit();
        }
    }

    printf("local_Server_RxCB--\n");

    return;
}

/***************************************************************************************************
 * @fn      local_Server_ConnectCB
 *
 * @brief   Callback for connecting SRPC clients.
 *
 * @return  Status
 ***************************************************************************************************/
void local_Server_ConnectCB(int clientFd)
{

    printf("local_Server_ConnectCB clientFd=%d\n",clientFd);

}

/***************************************************************************************************
 * @fn      local_Server_Init
 *
 * @brief   initialises the RPC interface and waitsfor a client to connect.
 * @param
 *
 * @return  Status
 ***************************************************************************************************/
void local_Server_Init(void)
{

    local_Server_DataInit();

    if (socketSeverInit(LOCAL_SERVER_TCP_PORT) == -1)
    {
        //exit if the server does not start
        exit(-1);
    }

    serverSocketConfig(local_Server_RxCB, local_Server_ConnectCB);
    
}

