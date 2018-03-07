#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
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
 * �� �� ��  : udp_new
 * �� �� ��  : Edward
 * ��������  : 2016��11��8��
 * ��������  : ����UDP����������
 * �������  : int port  �����˿ں�
 * �������  : ��
 * �� �� ֵ  : udp_t
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
udp_t *udp_new (int port)
{
	udp_t *self = NULL;
	int opt = 1;
	
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

	    //����UDP��ַ
		self->address.sin_family=AF_INET;
		self->address.sin_port = htons (self->port);
		self->address.sin_addr.s_addr = htonl (INADDR_ANY);		

		//���ù㲥��ַ
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

		log_debug("UDP Server Address: %s:%d\n",inet_ntoa(self->address.sin_addr),ntohs(self->address.sin_port));
		log_debug("UDP Broadcast Address: %s:%d\n",inet_ntoa(self->broadcast.sin_addr),ntohs(self->broadcast.sin_port));

    	return self;
	}while(0);

    free(self);
    return NULL;
}
/*****************************************************************************
 * �� �� ��  : udp_destroy
 * �� �� ��  : Edward
 * ��������  : 2016��11��8��
 * ��������  : �ͷ�UDP���
 * �������  : udp_t **self_p �����ָ��
 * �������  : ��
 * �� �� ֵ  : udp_t
 * ���ù�ϵ  : 
 * ��    ��  : 

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
 * �� �� ��  : udp_send
 * �� �� ��  : Edward
 * ��������  : 2016��11��8��
 * ��������  : �㲥����udp���ݰ�
 * �������  : udp_t *self    UDP������
               char *buffer   �������ݻ�����
               size_t length  �������ݳ���
 * �������  : ��
 * �� �� ֵ  : ʧ�ܷ���-1,�ɹ����ط��͵��ֽ���
 * ���ù�ϵ  : 
 * ��    ��  : 

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
 * �� �� ��  : udp_recv
 * �� �� ��  : Edward
 * ��������  : 2016��11��8��
 * ��������  : ����UDP��
 * �������  : udp_t *self    UDP������
               char *buffer   ���ջ�����
               size_t length  ����������
 * �������  : ��
 * �� �� ֵ  : ʧ�ܷ���-1,�ɹ����ؽ��յ��ֽ���
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
ssize_t udp_recv (udp_t *self, char *buffer, size_t length)
{

	socklen_t si_len = sizeof (struct sockaddr_in);
	
    int  size = recvfrom (self->handle, buffer, length, 0, (struct sockaddr *)&(self->recvfroms), &si_len);

    if (size == -1)
    {
		log_err("recvfrom error\n");
		return -1;
    }

	printf("size = %d\n",size);
    	
	printf ("ReadFrom %s:%d\n",inet_ntoa (self->recvfroms.sin_addr), ntohs (self->recvfroms.sin_port));
	
    return size;
}

