/**************************************************************************
 * Filename:       sim800lEvent.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    sim800l events
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/


/*********************************************************************
* INCLUDES
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include<unistd.h>
#include <sys/epoll.h>  
#include <sys/types.h>    
#include <netinet/in.h>  
#include <poll.h>  
#include <fcntl.h> 
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include <event2/event.h>

#include "uart.h"
#include "comParse.h"
#include "interface_srpcserver.h"
#include "interface_srpcserver_defs.h"
#include "Types.h"
#include "logUtils.h"
#include "Tcp_client.h"
#include "queue.h"
#include "sim800l.h"
/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/

#define MAX_PROTOCO_ANALYSIS_FIFO_SIZE 	1024
/*********************************************************************
* TYPEDEFS
*/
typedef enum
{
	SIM800L_CMD_MODE 	   = 0x00,
	SIM800L_SERIALNET_MODE = 0x01,
}sim800lMode_t;

typedef struct 
{
	int uart_fd ;
	char *deviceName;
	struct event *event;
	struct event_base *base;
	void (* uartProcessMsgCb)(hostCmd *cmd);
}sim800lUart_t;

typedef struct 
{
	uint8_t fifo[MAX_PROTOCO_ANALYSIS_FIFO_SIZE];
	uint16_t byteCnt;
}ProtocoAanalysis_t;

typedef struct CmdList_t
{
	hostCmd cmds;
	void* next;
}CmdList_t;


/*********************************************************************
* GLOBAL VARIABLES
*/

//串口缓冲区
extern UartFifo_t gUartRxfifo;   

/*********************************************************************
* LOCAL VARIABLES
*/
char *AT = "AT+CGREG?\r\n";
sim800lUart_t sim800lUart;
static pthread_t sim800lMsgProcessPthread = 0;
ProtocoAanalysis_t ProtocoAanalysis;

//数据处理
CmdList_t *CmdListHead = NULL;
pthread_mutex_t lock;
bool isReciveData = false;

tcp_connection_t *nwkconnect = NULL;

/*********************************************************************
* LOCAL FUNCTIONS
*/

/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* @fn		   sim800lEvt_ProtocoAanalysis
*
* @brief	   网关协议分析
*
* @param	   
*
* @return	   void
*/
void sim800lEvt_ProtocoAanalysis(uint8_t *buf,uint16_t size,void*args)
{
	hostCmd cmd;
	size_t totalMsgLen = 0;
	uint16_t cmdPacketLen = 0;
	uint16_t packetHeaderPos=0;
	
	sim800lUart_t *sim800lUarts = (sim800lUart_t*)args;

	log_debug("sim800lEvt_ProtocoAanalysis++1\n");


	//如果是连接服务的线程则，更新心跳定时
	zbSoc_Heartbeat_Client_Report_refresh();
	
	if(size > (MaxCacheBufferLength-ProtocoAanalysis.byteCnt))
	{
		memcpy(&ProtocoAanalysis.fifo[ProtocoAanalysis.byteCnt],buf,MaxCacheBufferLength-ProtocoAanalysis.byteCnt);
		ProtocoAanalysis.byteCnt += MaxCacheBufferLength-ProtocoAanalysis.byteCnt;
	}
	else
	{
		memcpy(&ProtocoAanalysis.fifo[ProtocoAanalysis.byteCnt],buf,size);
		ProtocoAanalysis.byteCnt += size;
	}
	
	log_debug("sim800lEvt_ProtocoAanalysis++2\n");
	log_debug_array(ProtocoAanalysis.fifo,ProtocoAanalysis.byteCnt,NULL);
	log_debug("\n");
	
	while(ProtocoAanalysis.byteCnt >= MinCmdPacketLength)
	{
		
		if(lookupFirstPacketHeader(ProtocoAanalysis.fifo,ProtocoAanalysis.byteCnt,&packetHeaderPos) == FindNoHeader)
        {
        	log_debug("CMD NOT FOUND\n");
           	memset(&ProtocoAanalysis,0,sizeof(ProtocoAanalysis));
            break;
        }

		if(packetHeaderPos != 0)
		{
		   memmove(ProtocoAanalysis.fifo,&ProtocoAanalysis.fifo[packetHeaderPos],(ProtocoAanalysis.byteCnt-packetHeaderPos));
	       ProtocoAanalysis.byteCnt -= packetHeaderPos;
		   if(  ProtocoAanalysis.byteCnt  < MinCmdPacketLength)
		   		break;
		}
		
		cmdPacketLen = (((ProtocoAanalysis.fifo[CMD_MSG_LEN0_POS]<<8)&0xff00)|(ProtocoAanalysis.fifo[CMD_MSG_LEN1_POS]&0x00ff));
		
		if(cmdPacketLen+4 >  ProtocoAanalysis.byteCnt)
		{
			log_debug("[zbSocMsg]: Packet is incomplete,Wait to receive other datas\n");
        	break;
		}

		if(checkPacketFCS(&ProtocoAanalysis.fifo[CMD_MSG_LEN0_POS],(cmdPacketLen+3))==true)
		{
			log_debug("==================MSG FROM SOCKET====================\n");
			log_debug("CMD:");
			log_debug_array(buf,cmdPacketLen+4,NULL);
			log_debug("\n");
			
			CmdList_t *srchRec = NULL;
			CmdList_t *pCmds = (CmdList_t*)malloc(sizeof(CmdList_t));
			
			
			if(pCmds != NULL)
			{
				memcpy(pCmds->cmds.data,&buf[CMD_MSG_TP_POS],(cmdPacketLen));
				pCmds->cmds.size = cmdPacketLen;
				pCmds->cmds.idx = 0;
				pCmds->next = NULL;

				pthread_mutex_lock(&lock);
				if(CmdListHead == NULL)
				{
					CmdListHead = pCmds;
				}
				else
				{
					srchRec  = CmdListHead;
					while(srchRec->next != NULL)
						srchRec = srchRec->next;
					srchRec->next = pCmds;
				}
				
				isReciveData  = true;
				pthread_mutex_unlock(&lock);
			}
		}

        if(ProtocoAanalysis.byteCnt >= (cmdPacketLen+MinCmdPacketLength))
        {
            memmove(ProtocoAanalysis.fifo,&ProtocoAanalysis.fifo[cmdPacketLen+MinCmdPacketLength],(ProtocoAanalysis.byteCnt -(cmdPacketLen+MinCmdPacketLength)));
            ProtocoAanalysis.byteCnt -= (cmdPacketLen+MinCmdPacketLength);
        }
        else
        {
            memset(&ProtocoAanalysis,0,sizeof(ProtocoAanalysis));
        }
	}	
}

/*********************************************************************
* @fn          sim800lEvt_event_callback_fn
*
* @brief       接收SIM800L串口数据
*
* @param       arg - NULL
*
* @return      void*
*/
void sim800lEvt_event_callback_fn(evutil_socket_t fd, short event, void *args)
{
	int ret = 0;
	char *pStr = NULL;
	
	log_debug("sim800l_event_callback_fn++\n");

	ret = fxUartRead(fd,&gUartRxfifo.fifo[gUartRxfifo.byteCnt], (UART_MAX_FIFO_SIZE - gUartRxfifo.byteCnt));
	if(ret < 0)
		return;
	
	gUartRxfifo.byteCnt += ret;

	log_debug("byteCnt = %d %d: %s\n",gUartRxfifo.byteCnt,ret,gUartRxfifo.fifo);
	
	if((pStr =  strstr(gUartRxfifo.fifo,"CLOSED"))||(pStr = strstr(gUartRxfifo.fifo,"+PDP: DEACT")))//连接关闭
	{
		if(nwkconnect->type == SERVER_CONNECT)
		{
			log_debug("Set Reconnect times++\n");
			tu_set_evtimer(&nwkconnect->evtimer,SERVER_RECONNECTION_RETRY_TIME,ONCE,start_tcp_client_failed_cb,nwkconnect);
			log_debug("Set Reconnect times--\n");
			zbSoc_Heartbeat_Client_Report_stop();
			zbSoc_Heartbeat_Connected_Report_stop();
			roomfairy_registerFlag = false;		
		}
		
		nwkconnect->devType == NETWORK_CARD;
		sim800lMode = SIM800L_MODE_AT;
		nwkconnect->connected = false;
		//connect->socketFd = -1;
		nwkconnect->bevent = NULL;
		nwkconnect->dns_base = NULL;
	}
	else if((pStr =  strstr(gUartRxfifo.fifo,"CONNECT")))
	{
		sim800lMode = SIM800L_MODE_NET;
		gUartRxfifo.byteCnt = 0;
	}
	else if((pStr =  strstr(gUartRxfifo.fifo,"+IPD,")))//用户数据
	{
		char *data = NULL;
		//获取数据长度
		int msglen = strtol(&pStr[5],&data,10);
		log_debug("msglen = %d\n",msglen);
		//log_debug_array(&data[1],msglen,NULL);
		sim800lEvt_ProtocoAanalysis(&data[1],msglen,args);
		//fxUartRxfifoClear();
		gUartRxfifo.byteCnt = 0;
	}else if(sim800lMode == SIM800L_MODE_NET)
	{
		sim800lEvt_ProtocoAanalysis(gUartRxfifo.fifo,gUartRxfifo.byteCnt,args);
		gUartRxfifo.byteCnt = 0;
	}

	if(gUartRxfifo.byteCnt >= UART_MAX_FIFO_SIZE)
		gUartRxfifo.byteCnt = 0;
	
	log_debug("sim800l_event_callback_fn--\n");
}

/*********************************************************************
* @fn          sim800lEvt_ProcessMsgCb
*
* @brief       sim800l串口接收数据线程回调函数
*
* @param       arg - NULL
*
* @return      void*
*/
void *sim800lEvt_ProcessMsgCb(void *arg)  
{
	sim800lUart_t *sim800lEvents =(sim800lUart_t*)arg;

	do{
		sim800lEvents->base= event_base_new();
		if(sim800lEvents->base == NULL)
		{
			log_debug("event_base_new error\n");
			break ;
		}
	
		sim800lEvents->event = event_new(sim800lEvents->base,sim800lEvents->uart_fd,EV_READ|EV_PERSIST,sim800lEvt_event_callback_fn,&sim800lEvents);
		if(sim800lEvents->event == NULL)
		{
			log_debug("event_new faild\n");
			break;
		}

		if(event_add(sim800lEvents->event,NULL)==-1)
		{
			log_debug("event_add Failed\n");
			break;
		}
		
		event_base_dispatch(sim800lEvents->base);
		
	}while(0);

	if(sim800lEvents->uart_fd != -1)
	{
		fxUartClose(sim800lEvents->uart_fd);
		sim800lEvents->uart_fd = -1;
	}
	
	if(sim800lEvents->event)
	{
		event_del(sim800lEvents->event);
		sim800lEvents->event = NULL;
	}
	
	pthread_detach(pthread_self());
}

/*********************************************************************
* @fn          sim800lEvt_ProcessCmds
*
* @brief       处理命令请求
*
* @param       args - 线程参数
*
* @return      void
*/

void *sim800lEvt_ProcessCmds(void *arg) 
{
	CmdList_t *srchRec = NULL,*NextRec=NULL;
	while(1)
	{
		if(isReciveData || CmdListHead!=NULL )
		{
			pthread_mutex_lock(&lock);
			
			srchRec = CmdListHead;
			while(srchRec != NULL)
			{
				NextRec = srchRec->next;
					
				SRPC_ProcessIncoming(&(srchRec->cmds));
				
				free(srchRec);
				CmdListHead = NextRec;
				srchRec = NextRec;
			}
			
			isReciveData = false;
			
			pthread_mutex_unlock(&lock);
		}
	
		msleep(100);
	}
	pthread_detach(pthread_self());
}

/*********************************************************************
* @fn          sim800l_evtInit
*
* @brief       sim800l任务初始化
*
* @param       device - 串口路径名称
*
* @return      0:success -1:falild
*/
int sim800lEvt_evtInit(char *device)
{
	int ret = 0;

	pthread_t processcmds ;
	
	sim800lUart.deviceName = strdup(device);

	sim800lUart.uartProcessMsgCb = SRPC_ProcessIncoming;

	
	sim800lUart.uart_fd = fxUartOpen(sim800lUart.deviceName);
	if(sim800lUart.uart_fd < 0)
	{
		log_debug("fxUartOpen faild\n");
		return -1;
	}
	
	fxUartRxfifoClear();

	pthread_mutex_init(&lock,NULL);
	
	//数据接收线程
	ret = pthread_create(&sim800lMsgProcessPthread,NULL,sim800lEvt_ProcessMsgCb,&sim800lUart);
	if(ret!=0)
		return -1;

	//数据处理线程
	ret = pthread_create(&processcmds,NULL,sim800lEvt_ProcessCmds,NULL);
	if(ret != 0)
		return -1;
#if 1	
	sim800lInit(sim800lUart.uart_fd);
#endif	
	return ret;
}

/*********************************************************************
* @fn          sim800lEvt_Connect
*
* @brief       连接服务器
*
* @param       connect - 连接结构体
*
* @return      0:success -1:falild
*/

int sim800lEvt_Connect(tcp_connection_t *connect)
{
	int ret = 0;
	char sPort[8] = {0};
	sprintf(sPort,"%d",connect->s_port);
	ret = sim800l_SocketConnect(connect->s_addr,sPort);

	
	//connect->devType = GPRS_MODULE;

	if(!ret) //连接成功
	{
		connect->connected = true;
		if(connect->type == SERVER_CONNECT)
		{
			//连接检测，当超时时，调用重连
			zbSoc_Heartbeat_Client_Report_start(start_tcp_client_failed_cb,connect);
			//连接成功后5秒发送心跳和注册消息
			zbSoc_Heartbeat_Connected_Report_start();

			connect->devType = GPRS_MODULE;
		}
		
		connect->socketFd = sim800lUart.uart_fd;
	}
	else //连接失败
	{
		if(connect->type == SERVER_CONNECT)
		{
			log_debug("Set Reconnect times++\n");
			ret = tu_set_evtimer(&connect->evtimer,SERVER_RECONNECTION_RETRY_TIME,ONCE,start_tcp_client_failed_cb,connect);
			log_debug("Set Reconnect times--\n");
			zbSoc_Heartbeat_Client_Report_stop();
			zbSoc_Heartbeat_Connected_Report_stop();
			roomfairy_registerFlag = false;		

			//切换连接方式
			connect->devType = NETWORK_CARD;
		}
		
		connect->connected = false;
		//connect->socketFd = -1;
		connect->bevent = NULL;
		connect->dns_base = NULL;
	}
	
	nwkconnect = connect;

	return ret;
}

/*********************************************************************/

