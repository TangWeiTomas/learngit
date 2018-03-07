/**************************************************************************************************
* Filename:       socket_server.h
* Author:             zxb      
* E-Mail:          zxb@yystart.com 
* Description:    Handle socket server.
*
*  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/ 
* 
* Version:         1.00  (2014-11-24,19:40)    :   Create the file.
*                  
*
*************************************************************************/

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * TYPEDEFS
 */

typedef void (*socketServerCb_t)(int clientFd);

/*********************************************************************
 * INCLUDES
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include "hal_types.h"

/*********************************************************************
 * CONSTANTS
 */
//#define SOCKET_SERVER_PORT 1234
#define MAX_CLIENTS         50

#ifdef OPENWRT_TEST
#define ServerFileName      "/etc/user"
#else
#define ServerFileName      "./user"
#endif

#if 0
#define REMOTE_SERV_IP          "192.168.0.130"
#define REMOTE_SERV_PORT        10000
#else
#define REMOTE_SERV_IP          "192.168.0.100"
#define REMOTE_SERV_PORT        6000
#endif

typedef struct {
    void *next;
    int socketFd;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    int connection_flag;
} socketRecord_t;

#define MinCmdPacketLength  14 //FLG(1)+LEN(2)+TP(2)+MAC(6)+D0D1(2)+...+FCS(1)

#define FindFullPacket		0
#define FindBreakPacket		1
#define FindErrorPacket		2
#define FindPacketHeader	3
#define FindNoHeader		4


/*
 * serverSocketInit - initialises the server.
 */
int32 socketSeverInit(uint32 port);

/*
 * serverSocketConfig - initialises the server.
 */
int32 serverSocketConfig(socketServerCb_t rxCb, socketServerCb_t connectCb);

/*
 * getClientFds -  get clients fd's.
 */
void socketSeverGetClientFds(int *fds, int maxFds);

/* 
 * getClientFds - get clients fd's. 
 */
uint32 socketSeverGetNumClients(void);

/*
 * socketSeverPoll - services the Socket events.
 */
void socketSeverPoll(int clinetFd, int revent);

/*
 * socketSeverSendAllclients - Send a buffer to all clients.
 */
int32 socketSeverSendAllclients(uint8* buf, uint32 len);

/*
 * socketSeverSend - Send a buffer to a clients.
 */
int32 socketSeverSend(uint8* buf, uint32 len, int32 fdClient);

/*
 * socketSeverClose - Closes the client connections.
 */
void socketSeverClose(void);


uint8 lookupFirstPacketHeader(uint8* data,uint16 length,uint16* startPos);

bool checkPacketFCS(uint8* data,uint16 length);

#ifdef __cplusplus
}
#endif

#endif /* SOCKET_SERVER_H */
