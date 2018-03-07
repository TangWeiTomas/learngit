/***********************************************************************************
 * 文 件 名   : zbSocUart.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : 串口操作
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
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
 * 函 数 名  : zbSocUartPowerOpen
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月19日
 * 函数功能  : 启动协调器电源
 * 输入参数  : void  无
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

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
		//关、开
		if(status == 0 || status == 1)
		{
			return status;
		}
	}

    return -1 ;
}

/*****************************************************************************
 * 函 数 名  : zbSocUartPowerClose
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月19日
 * 函数功能  : 关闭协调器电源
 * 输入参数  : void  无
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

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
		//关、开
		if(status == 0 || status == 1)
		{
			return status;
		}
	}

    return -1 ;
}

 
/*****************************************************************************
 * 函 数 名  : zbSocUartOpen
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月19日
 * 函数功能  : 打开并初始化串口
 * 输入参数  : char *devicePath  串口路径
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
 static int32_t zbSocUartOpen(char *devicePath)
{
    struct termios tio;
    int zbSoc_fd;
    log_debug("zbSocUartOpen\n");

	//打开协调器电源
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
	
    //清除所有正在发生的I/O数据
    tcflush(zbSoc_fd, TCIOFLUSH);
  
    tcsetattr(zbSoc_fd, TCSANOW, &tio);

    return  zbSoc_fd;
}

static void zbSocUartClose(void)
{
	//关闭协调器电源
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

//将协议数据中的内容进行转义
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
 * 函 数 名  : zbSocCmdSend
 * 负 责 人  : Edward
 * 创建日期  : 2016年7月19日
 * 函数功能  : 写串口数据
 * 输入参数  : uint8_t* buf  数据
               uint16_t len  数据长度
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

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
	//将字符进行转义
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

	//命令间隔时间
	usleep(100000);
    
    log_debug("zbSocCmdSend--\n");
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//							基于libevent实现串口读写										 //
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
			//将串口数据放到缓冲区
			if(evbuffer_read(uart_event->buffer,fd,MAX_BUF_SIZE)<=0)
			{
				//read -1 if an error occurred
				return;
			}
				
			//处理缓冲区数据
			while((totalMsgLen = evbuffer_get_length(uart_event->buffer)) >= MinSocCmdPacketLength)
			{
				packetHeaderPos  = 0;
				bzero(buf,sizeof(buf));

				//将缓冲区的数据拷贝到零时缓冲区
				evbuffer_copyout(uart_event->buffer,buf,totalMsgLen);

				//寻找数据头部0XFC标志
				if(lookupSocFirstPacketHeader(buf,totalMsgLen,&packetHeaderPos)==false)
				{
					log_debug("cmd not found\n");
					//清除缓冲区
					evbuffer_drain(uart_event->buffer,totalMsgLen);
					break;
				}

				//丢弃非法包头数据
				if(packetHeaderPos != 0)
				{
					//清除缓冲区前packetHeaderPos位置的数据
					evbuffer_drain(uart_event->buffer,packetHeaderPos);
					if((totalMsgLen = evbuffer_get_length(uart_event->buffer)) < MinSocCmdPacketLength)
					{
						log_debug("length error\n");
						break;
					}
				}
				
				bzero(buf,sizeof(buf));

				//获取长度信息
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

				//将数据移到零时缓冲区，并清除缓冲区的内容
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
	
	//处理多包
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

	        //丢弃非法包头数据
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
	
	u_event->zbSoc_fd = zbSocUartOpen(u_event->deviceName);		//打开串口设备
	
	if(-1 == u_event->zbSoc_fd )
	{
		log_err("Uart open Failed\n");
		return false;
	}
	
#ifdef UART_USE_BUFFER
	//创建event数据接收缓冲区
	u_event->buffer = evbuffer_new();
	if(u_event->buffer == NULL)
	{
		log_err("evbuffer_new Failed\n");
		goto error;
	}
#else
	memset(&zbSocUartCacheBuf,0,sizeof(zbSocUartCacheBuf));
#endif

	//创建一个新event
	u_event->Uart_event = event_new(u_event->base,u_event->zbSoc_fd,EV_READ|EV_PERSIST,zbSocUart_Read_cb,&g_uart_event);

	if(u_event->Uart_event == NULL)
	{
		log_err("event_new Failed\n");
		goto error;
	}

	//设置event优先级
	//event_priority_set(u_event->Uart_event,0);

	//将event添加到调度列表中
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
	g_uart_event.Uart_Handler_cb = zbSocMsg_ProcessIncoming;	//设置数据处理回调函数
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

