#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>  

#include "libsocketudp.h"

#include "logUtils.h"

/*
#define NDEBUG 0

#define clean_errno() (errno==0 ? "None":strerror(errno))
#define log_err(M,...)	fprintf(stderr,"[Error][%s:%d] %s" M,__FUNCTION__,__LINE__,clean_errno(),##__VA_ARGS__);

#ifndef NDEBUG
#define ASSERT(expr) NULL
#else
#define ASSERT(expr)                                          \
	 do {													  \
	  if (!(expr)) {										  \
		fprintf(stderr, 									  \
				"Assertion failed in %s on line %d: %s\n",	  \
				__FILE__,									  \
				__LINE__,									  \
            #expr);                                       	  \
		abort();											  \
	  } 													  \
	 } while (0)
#endif
*/
/*****************************************************************************
 * 函 数 名  : udp_new
 * 负 责 人  : Edward
 * 创建日期  : 2016年11月8日
 * 函数功能  : 创建UDP监听服务器
 * 输入参数  : int port  监听端口号
 * 输出参数  : 无
 * 返 回 值  : udp_t
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
udp_t *udp_new (int port)
{
	udp_t *self = NULL;
	int opt = 1;
	int fcntls = 0;

	
	do{
	    self = (udp_t *) malloc (sizeof (udp_t));
		if(self == NULL)
		{
			log_err("malloc error\n");
			break;
		}

		bzero(self,sizeof(udp_t));
		
		self->port = port;

		self->handle = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	    if (self->handle == -1)
	    {
			log_err("socket error\n");
			break;
	    }

	    if(setsockopt(self->handle, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt)) == -1)
	    {
			log_err("setsockopt error\n");
			break;
	    }
		
	    //设置UDP地址
		self->address.sin_family=AF_INET;
		self->address.sin_port = htons (self->port);
		self->address.sin_addr.s_addr = htonl (INADDR_ANY);		

		//设置广播地址
		self->broadcast.sin_family = AF_INET;
		self->broadcast.sin_port = htons (self->port);
		//self->address.sin_addr.s_addr = htonl (INADDR_ANY);		
		self->broadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
		//inet_aton ("255.255.255.255", &self->broadcast.sin_addr);

		if (bind (self->handle, (const struct sockaddr*)(&self->address), sizeof (self->address)) == -1)
		{
			log_err("bind error\n");
			break;
		}

		
		//设置为非阻塞模式

		fcntls = fcntl(self->handle,F_GETFL,0);
		if(fcntls < 0)
		{
			log_err("setsockopt error\n");
			break;
		}
		
		fcntls |= O_NONBLOCK;

		if(fcntl(self->handle,F_SETFL,fcntls) < 0)
		{
			log_err("setsockopt error\n");
			break;
		}


		log_debug("UDP Server Address: %s:%d\n",inet_ntoa(self->address.sin_addr),ntohs(self->address.sin_port));
		log_debug("UDP Broadcast Address: %s:%d\n",inet_ntoa(self->broadcast.sin_addr),ntohs(self->broadcast.sin_port));

    	return self;
	}while(0);

    free(self);
    return NULL;
}
/*****************************************************************************
 * 函 数 名  : udp_destroy
 * 负 责 人  : Edward
 * 创建日期  : 2016年11月8日
 * 函数功能  : 释放UDP句柄
 * 输入参数  : udp_t **self_p 句柄的指针
 * 输出参数  : 无
 * 返 回 值  : udp_t
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/

void udp_destroy (udp_t **self_p)
{
    ASSERT (self_p);
    if (*self_p) {
        udp_t *self = *self_p;
        close (self->handle);
        free (self);
        *self_p = NULL;
    }
}

int udp_handle (udp_t *self)
{
    ASSERT (self);
    return self->handle;
}

/*****************************************************************************
 * 函 数 名  : udp_send
 * 负 责 人  : Edward
 * 创建日期  : 2016年11月8日
 * 函数功能  : 广播发送udp数据包
 * 输入参数  : udp_t *self    UDP描述符
               char *buffer   发送数据缓冲区
               size_t length  发送数据长度
 * 输出参数  : 无
 * 返 回 值  : 失败返回-1,成功返回发送的字节数
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
ssize_t udp_send (udp_t *self, char *buffer, size_t length)
{
	ssize_t scnt = -1;
    scnt = sendto (self->handle, buffer, length, 0, (const struct sockaddr *)&(self->broadcast), sizeof(self->broadcast));
    if(scnt == -1)
    {
		log_err("sendto Error\n"); 
		return -1 ;
    }
    
    return scnt;
}

/*****************************************************************************
 * 函 数 名  : udp_recv
 * 负 责 人  : Edward
 * 创建日期  : 2016年11月8日
 * 函数功能  : 接收UDP包
 * 输入参数  : udp_t *self    UDP描述符
               char *buffer   接收缓冲区
               size_t length  缓存区长度
 * 输出参数  : 无
 * 返 回 值  : 失败返回-1,成功返回接收的字节数
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
ssize_t udp_recv (udp_t *self, char *buffer, size_t length)
{

	socklen_t si_len = sizeof (struct sockaddr_in);
	
    int  size = recvfrom (self->handle, buffer, length, 0, (struct sockaddr *)&(self->recvfroms), &si_len);

    if (size == -1 && errno != EAGAIN)
    {
		log_err("recvfrom error\n");
		return -1;
    }
	
	printf("size = %d\n",size);
    	
	printf ("ReadFrom %s:%d\n",inet_ntoa (self->recvfroms.sin_addr), ntohs (self->recvfroms.sin_port));
	
    return size;
}

