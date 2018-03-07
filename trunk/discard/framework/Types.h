/*******************************************************************************
  Filename:       types.h
  Revised:        $Date$
  Revision:       $Revision$

  Description:   This file contains type definitions for the HA Gateway Demo.
*******************************************************************************/
#ifndef TYPES_H
#define TYPES_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <sys/signalfd.h>
//#include <assert.h>
#include <stddef.h>

/******************************************************************************
 * Constants
 *****************************************************************************/

#define INITIAL_CONFIRMATION_TIMEOUT 5000

#define MAX_DB_FILENAMR_LEN 255

#define ENABLE 							0X01
#define DISABLE							0X00
#define	DATASEGMENT						32
#define EVENT_TYPE_NODE_ACTION			0x01
#define EVENT_TYPE_SCENEID_ACTION		0x00

/******************************************************************************
 * Constants - Clusters and their associated attributes
 *****************************************************************************/

/******************************************************************************
 * Constants - 协议定义
 *****************************************************************************/
#define MinCmdPacketLength  14 //FLG(1)+LEN(2)+TP(2)+MAC(6)+D0D1(2)+...+FCS(1)

#define FindFullPacket      0
#define FindBreakPacket     1
#define FindErrorPacket     2
#define FindPacketHeader    3
#define FindNoHeader        4

#define ONE_SEC_TIME     1000
#define SecTime(n)     ((n)*ONE_SEC_TIME)

#define ONE_MIN_TIME     1000*60
#define MinTime(n)     ((n)*ONE_MIN_TIME)

//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER) 

#define container_of(ptr, type, member) ({	    \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})


/******************************************************************************
 * Types			设备类型定义
 *****************************************************************************/
	
typedef enum
{
	ONCE = false,
	CIRCLE	= true
} TimerType_e;

#endif /* TYPES_H */

