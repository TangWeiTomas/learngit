/***********************************************************************************
 * �� �� ��   : libsocketudp.h
 * �� �� ��   : Edward
 * ��������   : 2016��11��4��
 * �ļ�����   : udp���߰�
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/
#ifndef _LIB_SOCKET_UDP_H_
#define _LIB_SOCKET_UDP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  

//  UDP instance

typedef struct {
    int handle;                 //  Socket for send/recv
    int port;                   //  UDP port we work on
    //  Own address
    struct sockaddr_in address;
    //  Broadcast address
    struct sockaddr_in broadcast;
    //revice addr
    struct sockaddr_in recvfroms;
    
} udp_t;

udp_t *udp_new (int port);
void udp_destroy (udp_t **self_p);
int udp_handle (udp_t *self);
ssize_t udp_send (udp_t *self, char *buffer, size_t length);
ssize_t udp_recv (udp_t *self, char *buffer, size_t length);

#ifdef __cplusplus
}
#endif
#endif
