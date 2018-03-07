/***********************************************************************************
 * 文 件 名   : libsocketudp.h
 * 负 责 人   : Edward
 * 创建日期   : 2016年11月4日
 * 文件描述   : udp工具包
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
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
