/***********************************************************************************
 * �� �� ��   : zbSocUart.c
 * �� �� ��   : Edward
 * ��������   : 2016��7��19��
 * �ļ�����   : ���ڲ���
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/


#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

#include "zbSocUart.h"
#include "comParse.h"
#include "zbSocPrivate.h"
#include "globalVal.h"
#include "zbSocCmd.h"
#include "logUtils.h"
#include "zbSocCmd.h"

#include <event2/event.h>
#include <event2/event-config.h>
#include <event2/util.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#define MAX_BUF_SIZE	512

/*******************************************************************************
 * Constants
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
 
uart_event_t g_uart_event;

#ifndef UART_USE_BUFFER
typedef struct
{
	uint8_t buf[MaxCacheBufferLength];
	uint16_t bufcount;
}cmdDataBuf_t;

cmdDataBuf_t zbSocUartCacheBuf;

#endif

/*******************************************************************************
 * Functions
 ******************************************************************************/
/*****************************************************************************
 * �� �� ��  : zbSocUartPowerOpen
 * �� �� ��  : Edward
 * ��������  : 2016��7��19��
 * ��������  : ����Э������Դ
 * �������  : void  ��
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static int8_t zbSocUartPowerOpen(void)
{
	pid_t mthread = 0;
	int status = 0;
#ifdef OPENWRT_TEST
		char *cmd = "/bin/zbPower on";
#else
		char *cmd = "./zbPower on";
#endif

	mthread = system(cmd);
	
	if( 0 > mthread)
		return -1;
		
	if(WIFEXITED(mthread))
	{
		status = WEXITSTATUS(mthread);
		//�ء���
		if(status == 0 || status == 1)
		{
			return status;
		}
	}

    return -1 ;
}

/*****************************************************************************
 * �� �� ��  : zbSocUartPowerClose
 * �� �� ��  : Edward
 * ��������  : 2016��7��19��
 * ��������  : �ر�Э������Դ
 * �������  : void  ��
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
static int8_t zbSocUartPowerClose(void)
{
	pid_t mthread = 0;
	int status = 0;
#ifdef OPENWRT_TEST
		char *cmd = "/bin/zbPower off";
#else
		char *cmd = "./zbPower off";
#endif

	mthread = system(cmd);
	
	if( 0 > mthread)
		return -1;
		
	if(WIFEXITED(mthread))
	{
		status = WEXITSTATUS(mthread);
		//�ء���
		if(status == 0 || status == 1)
		{
			return status;
		}
	}

    return -1 ;
}

 
/*****************************************************************************
 * �� �� ��  : zbSocUartOpen
 * �� �� ��  : Edward
 * ��������  : 2016��7��19��
 * ��������  : �򿪲���ʼ������
 * �������  : char *devicePath  ����·��
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
 static int32_t zbSocUartOpen(char *devicePath)
{
    struct termios tio;
    int zbSoc_fd;
    log_debug("zbSocUartOpen\n");

	//��Э������Դ
	zbSocUartPowerOpen();
    
    /* open the device to be non-blocking (read will return immediatly) */
    // zbSoc_Uart_Server.zbSoc_fd = open(devicePath, O_RDWR | O_NOCTTY | O_NONBLOCK);
    // zbSoc_fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
    zbSoc_fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
    if (zbSoc_fd < 0)
    {
        perror(devicePath);
        log_debug("%s open failed\n", devicePath);
        return (-1);
    }

	tcflush(zbSoc_fd, TCIOFLUSH);

	bzero(&tio,sizeof(tio));
	
	if(tcgetattr(zbSoc_fd,&tio) !=0 )
	{
		log_debug("tcgetatter failed\n");
		return -1;
	}
	
    //make the access exclusive so other instances will return -1 and exit
    ioctl( zbSoc_fd, TIOCEXCL);
    
	/* c-iflags
	 B115200 : set board rate to 115200
	 CRTSCTS : HW flow control (disabled below)
	 CS8     : 8n1 (8bit,no parity,1 stopbit)
	 CLOCAL  : local connection, no modem contol
	 CREAD   : enable receiving characters*/
	tio.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
	/* c-iflags
	 ICRNL   : maps 0xD (CR) to 0x10 (LR), we do not want this.
	 IGNPAR  : ignore bits with parity erros, I guess it is
	 better to ignStateore an erronious bit then interprit it incorrectly. */
	tio.c_iflag = IGNPAR & ~ICRNL;
	tio.c_oflag = 0;
	tio.c_lflag = 0;

//    cfsetispeed(&tio,B57600);     
//    cfsetospeed(&tio,B57600);  
//    
//    tio.c_cflag |= CLOCAL | CREAD;
//    tio.c_cflag &= ~CRTSCTS; 
//    tio.c_cflag &= ~CSIZE;
//    tio.c_cflag |=  CS8  ;
//	tio.c_cflag &= ~PARENB; 
//	tio.c_iflag &= ~INPCK;
//	tio.c_cflag &= ~CSTOPB;
//	tio.c_iflag |= IGNPAR & ~ICRNL;
//	 
//	tio.c_oflag &= ~OPOST;
//	tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
    //����������ڷ�����I/O����
    tcflush(zbSoc_fd, TCIOFLUSH);
  
    tcsetattr(zbSoc_fd, TCSANOW, &tio);

    return  zbSoc_fd;
}

static void zbSocUartClose(void)
{
	//�ر�Э������Դ
	zbSocUartPowerClose();
	if(g_uart_event.zbSoc_fd != -1)
	{
		close(g_uart_event.zbSoc_fd);
		g_uart_event.zbSoc_fd = -1;
	}	
}

#if DISPOSE_ESC

#define ESC 0xFF
#define STX 0xFE

//��Э�������е����ݽ���ת��
uint32_t DisposeESC(uint32_t nLength, uint8_t *pOut, const uint8_t *pIn)
{
    uint32_t i = 0;
    uint16_t temp;
    uint32_t out_length = 0;
    for (i = 0; i < nLength; i++)
    {
        if (*pIn == ESC || *pIn  == STX)  //need to  escape data processing
        {
            temp = *pIn | 0x100;
            *pOut = ESC;
            out_length++;
            pOut++;
            *pOut = (uint8_t)((temp - ESC) & 0XFF);
        }
        else
        {
            *pOut = *pIn;
        }
        out_length++;
        pOut++;
        pIn++;
    }
    
    return out_length;
}

#endif
/*****************************************************************************
 * �� �� ��  : zbSocCmdSend
 * �� �� ��  : Edward
 * ��������  : 2016��7��19��
 * ��������  : д��������
 * �������  : uint8_t* buf  ����
               uint16_t len  ���ݳ���
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
void zbSocCmdSend(uint8_t* buf, uint16_t len)
{
    int remain = 0;
    int offset = 0;
    int rtn = 0;
	char *pbuf = NULL;
	
    log_debug("zbSocCmdSend++\n");
    
	log_debug("CMD:");
    log_debug_array(buf,len,NULL);

	if(g_uart_event.zbSoc_fd == -1)
	{	
		log_debug("zbSoc_fd failed\n");
		return ;
	}

#if DISPOSE_ESC 
	char buffer[MaxPacketLength] = {0};

	buffer[0] = 0xFE;
	//���ַ�����ת��
	remain = DisposeESC((len-1),&buffer[1],&buf[1]) + 1;
	pbuf = buffer;

	log_debug("CMD:");
	log_debug_array(pbuf,remain,NULL);
#else
	pbuf = buf;
	remain = len;
#endif

    while (remain > 0)
    {
        int sub = (remain >= 8 ? 8 : remain);

		rtn = write(g_uart_event.zbSoc_fd, pbuf + offset, sub);

        tcflush(g_uart_event.zbSoc_fd, TCOFLUSH);
		usleep(5000);

        remain -= 8;
        offset += 8;

		if(rtn < 0)
		{
			log_debug("Send Error errno = %d :%s\n",errno,strerror(errno));
		}
    }
	
    if(rtn < 0 )
    {
        log_debug("Send Error errno = %d :%s\n",errno,strerror(errno));
    }

	//������ʱ��
	usleep(100000);
    
    log_debug("zbSocCmdSend--\n");
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							����libeventʵ�ִ��ڶ�д										 //
///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef UART_USE_BUFFER
static void zbSocUart_Read_cb(evutil_socket_t fd, short event, void * args)
{
		uint8_t buf[MaxCacheBufferLength] = {0};	
		hostCmd cmd;
		size_t totalMsgLen = 0;
		size_t cmdPacketLen = 0;

		uint16_t packetHeaderPos=0;
		uart_event_t *uart_event = args;
		while(1)
		{
			//���������ݷŵ�������
			if(evbuffer_read(uart_event->buffer,fd,MAX_BUF_SIZE)<=0)
			{
				//read -1 if an error occurred
				return;
			}
				
			//������������
			while((totalMsgLen = evbuffer_get_length(uart_event->buffer)) >= MinSocCmdPacketLength)
			{
				packetHeaderPos  = 0;
				bzero(buf,sizeof(buf));

				//�������������ݿ�������ʱ������
				evbuffer_copyout(uart_event->buffer,buf,totalMsgLen);

				//Ѱ������ͷ��0XFC��־
				if(lookupSocFirstPacketHeader(buf,totalMsgLen,&packetHeaderPos)==false)
				{
					log_debug("cmd not found\n");
					//���������
					evbuffer_drain(uart_event->buffer,totalMsgLen);
					break;
				}

				//�����Ƿ���ͷ����
				if(packetHeaderPos != 0)
				{
					//���������ǰpacketHeaderPosλ�õ�����
					evbuffer_drain(uart_event->buffer,packetHeaderPos);
					if((totalMsgLen = evbuffer_get_length(uart_event->buffer)) < MinSocCmdPacketLength)
					{
						log_debug("length error\n");
						break;
					}
				}
				
				bzero(buf,sizeof(buf));

				//��ȡ������Ϣ
				if(evbuffer_copyout(uart_event->buffer,buf,MinSocCmdPacketLength)==-1)
				{
					log_debug("evbuffer_copyout ERROR\n");
					break;
				}
				
				cmdPacketLen = buf[SOC_MSG_LEN_POS];
				
				if(cmdPacketLen+MinSocCmdPacketLength > totalMsgLen)
				{
					log_debug("[zbSocMsg]: Packet is incomplete,Wait to receive other datas\n");
	            	break;
				}
				
				bzero(buf,sizeof(buf));

				//�������Ƶ���ʱ�������������������������
				if(evbuffer_remove(uart_event->buffer,buf,cmdPacketLen+MinSocCmdPacketLength)==-1)
				{
					log_debug("evbuffer_remove ERROR\n");
					break;
				}
				
				if(checkPacketFCS(&buf[SOC_MSG_LEN_POS],(cmdPacketLen+4))==true)
				{
					log_debug("==================MSG FROM MT====================\n");
					log_debug("CMD:");
					log_debug_array(buf,cmdPacketLen+MinSocCmdPacketLength,NULL);
					log_debug("\n");
					memcpy(cmd.data,&buf[SOC_MSG_CMD0_POS],(cmdPacketLen+2));
					cmd.size = cmdPacketLen+2;
					cmd.idx = 0;
					if(uart_event->Uart_Handler_cb)
						uart_event->Uart_Handler_cb(&cmd);
				}
			}
			usleep(2000);
		}
}
#else
static void zbSocUart_Read_cb(evutil_socket_t fd, short event, void * args)
{
	int byteRead = 0;
    uint16_t packetHeaderPos=0;
    uint8_t cmdPacketLen;
    hostCmd cmd;

	uart_event_t *uart_event = args;
	
	log_debug("zbSocUart_Read_cb++\n");
	
	//������
	while(1)
	{
	    byteRead = read(fd, &zbSocUartCacheBuf.buf[zbSocUartCacheBuf.bufcount], (MaxCacheBufferLength-zbSocUartCacheBuf.bufcount));

	    if(byteRead<=0)
		{
			log_debug("no data to read\n");
			break;
	    }

	    zbSocUartCacheBuf.bufcount += byteRead;
		
	    while (zbSocUartCacheBuf.bufcount >= MinSocCmdPacketLength)
	    {

	        if(lookupSocFirstPacketHeader(zbSocUartCacheBuf.buf,zbSocUartCacheBuf.bufcount,&packetHeaderPos)==false)
	        {
	            memset(&zbSocUartCacheBuf,0,sizeof(zbSocUartCacheBuf));
	            log_debug("lookupSocFirstPacketHeader failed\n");
	            return;
	        }

	        //�����Ƿ���ͷ����
	        if(packetHeaderPos != 0)
	        {
	            memmove(zbSocUartCacheBuf.buf,&zbSocUartCacheBuf.buf[packetHeaderPos],(zbSocUartCacheBuf.bufcount-packetHeaderPos));
	            zbSocUartCacheBuf.bufcount -= packetHeaderPos;
	        }

	        cmdPacketLen = zbSocUartCacheBuf.buf[SOC_MSG_LEN_POS];

	        // 1byte(FLAG)+1byte(Len)+2byte(Cmd)+...+1byte(FCS)
	        if((cmdPacketLen+MinSocCmdPacketLength) > zbSocUartCacheBuf.bufcount)
	        {
	            log_debug(" Packet is incomplete,Wait to receive other datas\n");
	           
	            break;
	        }

	        //1byte(Len)+2byte(Cmd)+1byte[FCS]
	        if(checkPacketFCS(&zbSocUartCacheBuf.buf[SOC_MSG_LEN_POS],(cmdPacketLen+4))==true)
	        {
	        	memset(&cmd,0,sizeof(cmd));
	            memcpy(cmd.data,&zbSocUartCacheBuf.buf[SOC_MSG_CMD0_POS],(cmdPacketLen+2));
				log_debug("==================MSG FROM MT====================\n");
				log_debug("CMD:");
				log_debug_array(cmd.data,cmdPacketLen+2,NULL);
				log_debug("\n");
				cmd.size = cmdPacketLen+2;
				cmd.idx = 0;
				if(uart_event->Uart_Handler_cb)
					uart_event->Uart_Handler_cb(&cmd);
	        }
	        else
	        {
	            log_debug("Drop the error fcs data packet.\n");
	        }

	        if(zbSocUartCacheBuf.bufcount >= (cmdPacketLen+MinSocCmdPacketLength))
	        {
	            memmove(zbSocUartCacheBuf.buf,&zbSocUartCacheBuf.buf[cmdPacketLen+MinSocCmdPacketLength],(zbSocUartCacheBuf.bufcount -(cmdPacketLen+MinSocCmdPacketLength)));
	            zbSocUartCacheBuf.bufcount -= (cmdPacketLen+MinSocCmdPacketLength);
	        }
	        else
	        {
	            memset(&zbSocUartCacheBuf,0,sizeof(zbSocUartCacheBuf));
	        }
	    }	
	    usleep(3000);
    }
    log_debug("zbSocUart_Read_cb++\n");
}

#endif
bool zbSocUart_event(uart_event_t *u_event)
{

	if(u_event == NULL)
		return false;
	
	u_event->zbSoc_fd = zbSocUartOpen(u_event->deviceName);		//�򿪴����豸
	
	if(-1 == u_event->zbSoc_fd )
	{
		log_err("Uart open Failed\n");
		return false;
	}
	
#ifdef UART_USE_BUFFER
	//����event���ݽ��ջ�����
	u_event->buffer = evbuffer_new();
	if(u_event->buffer == NULL)
	{
		log_err("evbuffer_new Failed\n");
		goto error;
	}
#else
	memset(&zbSocUartCacheBuf,0,sizeof(zbSocUartCacheBuf));
#endif

	//����һ����event
	u_event->Uart_event = event_new(u_event->base,u_event->zbSoc_fd,EV_READ|EV_PERSIST,zbSocUart_Read_cb,&g_uart_event);

	if(u_event->Uart_event == NULL)
	{
		log_err("event_new Failed\n");
		goto error;
	}

	//����event���ȼ�
	//event_priority_set(u_event->Uart_event,0);

	//��event��ӵ������б���
	if(event_add(u_event->Uart_event,NULL)==-1)
	{
		log_err("event_add Failed\n");
		goto error;
	}

	return true;
	
error:
	if(u_event->zbSoc_fd != -1)
	{
		close(u_event->zbSoc_fd);
		u_event->zbSoc_fd = -1;
	}
	
	if(u_event->Uart_event)
	{
		event_del(u_event->Uart_event);
		u_event->Uart_event = NULL;
	}
#ifdef UART_USE_BUFFER
	if(u_event->buffer)
	{	
		evbuffer_free(u_event->buffer);
		u_event->buffer = NULL;
	}
#endif
	return false;
}

void zbSocUart_event_relase(void)
{

	zbSocUartClose();
	
	if(g_uart_event.Uart_event)
	{
		event_del(g_uart_event.Uart_event);
		g_uart_event.Uart_event = NULL;
	}
#ifdef UART_USE_BUFFER	
	if(g_uart_event.buffer)
	{	
		evbuffer_free(g_uart_event.buffer);
		g_uart_event.buffer = NULL;
	}
#endif
}

bool zbSocUart_evInit(struct event_base *base,const char *device)
{
	ASSERT(device != NULL && device != NULL);

	if(base == NULL || device == NULL)
		return false;

	memset(&g_uart_event,0,sizeof(uart_event_t));

	g_uart_event.base = base;
	g_uart_event.Uart_Handler_cb = zbSocMsg_ProcessIncoming;	//�������ݴ���ص�����
	g_uart_event.timeout = 0;
	
	g_uart_event.deviceName = malloc(strlen((char*)device));
	if(NULL == g_uart_event.deviceName)
	{
		log_err("malloc failed\n");
		return false;
	}

	memcpy(g_uart_event.deviceName,device,strlen(device));

	log_debug("device : %s\n",g_uart_event.deviceName);

	if(zbSocUart_event(&g_uart_event) == false)
	{
		free(g_uart_event.deviceName);
		return false;
	}
	
	return  true;
}

void zbSocUart_evRelase(void)
{
	if(g_uart_event.zbSoc_fd != -1)
	{
		close(g_uart_event.zbSoc_fd);
		g_uart_event.zbSoc_fd = -1;
	}
	
	if(g_uart_event.Uart_event)
	{
		event_del(g_uart_event.Uart_event);
		g_uart_event.Uart_event = NULL;
	}

#ifdef UART_USE_BUFFER
	if(g_uart_event.buffer)
	{	
		evbuffer_free(g_uart_event.buffer);
		g_uart_event.buffer = NULL;
	}
#endif

	if(g_uart_event.deviceName)
	{
		free(g_uart_event.deviceName);
		g_uart_event.deviceName = NULL;
	}
}

void zbSocUart_timeout_Set(int cnt)
{
	if(cnt >= 0)
	{
		g_uart_event.timeout = cnt;
	}
}

int zbSocUart_timeout_get(void)
{
	return g_uart_event.timeout;
}

