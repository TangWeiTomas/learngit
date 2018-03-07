/*******************************************************************************
 Filename:       tcp_client.h
 Revised:        $Date$
 Revision:       $Revision$

 Description:   Client communication via TCP ports


 Copyright 2013 Texas Instruments Incorporated. All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License").  You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless used solely and exclusively in conjunction with
 a Texas Instruments radio frequency device, which is integrated into
 your product.  Other than for the foregoing purpose, you may not use,
 reproduce, copy, prepare derivative works of, modify, distribute, perform,
 display or sell this Software and/or its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,l
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.
*******************************************************************************/
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "comParse.h"
#include <event2/util.h>
#include "PackageUtils.h"
#include "globalVal.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
#define SERVER_RECONNECTION_RETRY_TIME 5000
#define MAX_TCP_PACKET_SIZE 1024

/******************************************************************************
 * Types
 *****************************************************************************/
typedef enum{
	SERVER_CONNECT = 0, //连接服务器
	CLIENT_CONNECT = 1, 	//客户端连接
	PERMISSION_CONNECT = 2	
}Client_Type_t;

#ifdef SUPPORT_GPRS_MODULE

typedef enum
{
	NETWORK_CARD = 0x00,
	GPRS_MODULE  = 0x01,
}deviceTypes_t;

#endif

typedef struct tcp_connection_s
{
	int socketFd;
	bool connected;
#ifdef  SUPPORT_GPRS_MODULE	
	deviceTypes_t devType;
#endif
	tu_evtimer_t evtimer;
	struct sockaddr_in addr;
	uint8_t *s_addr;
	uint16_t s_port;
	struct bufferevent *bevent;
	struct evdns_base *dns_base; 
	struct event_base *base;
	Client_Type_t type;
	void (* tcp_Read_Handler_cb)(hostCmd *cmd);
	void *next;
}tcp_connection_t;

typedef struct tcp_server_s
{
	struct event_base *base;
	struct evconnlistener *listener;
}tcp_server_t;

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/
bool tcp_client_send_msg(uint8_t *cmdMsg,uint16_t cmdMsgLen);
bool tcp_client_evInit(struct event_base *base);
bool tcp_server_evInit(struct event_base *base);


#if PERMIMNG
tcp_connection_t* tcp_permimng_client_evInit(struct event_base *base);
void start_tcp_permimng_client_failed_cb(void * args);
#endif

void start_tcp_client_failed_cb(void * args);

#endif /* TCP_CLIENT_H */

