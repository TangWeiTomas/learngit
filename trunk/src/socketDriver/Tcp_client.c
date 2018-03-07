/***********************************************************************************
 * 文 件 名   : Tcp_client.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年7月19日
 * 文件描述   : TCP 连接
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "Types.h"

#include "Tcp_client.h"
#include "Polling.h"
#include "logUtils.h"
#include "Types.h"
#include "globalVal.h"
#include "interface_srpcserver_defs.h"
#include "Zigbee_device_Heartbeat_Manager.h"
#include "fileMng.h"
#include "PackageUtils.h"

#include "PermissionManage.h"

#include "net_utils.h"
#include "Timer_utils.h"
#include <event2/event.h>
#include <event2/event-config.h>
#include <event2/util.h>
#include "indLight.h"

//#include <assert.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "interface_srpcserver.h"
#include <event2/buffer.h>
#include <event2/dns.h> 

/******************************************************************************
 * Constants
 *****************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////
//							基于libevent实现SOCKET通讯  									 //
///////////////////////////////////////////////////////////////////////////////////////////////

static tcp_connection_t *tcp_connection_head = NULL ;
static tcp_server_t tcp_server_head ={NULL,NULL} ;

void start_tcp_client_failed_cb(void * args);
static bool start_tcp_client(tcp_connection_t *connect);

#if PERMIMNG
bool start_tcp_permimng_client(tcp_connection_t *connect);
#endif


int socket_getaddrinfo(tcp_connection_t *connect,const char* server,uint16_t port)
{
	struct evutil_addrinfo ai, *aitop = NULL;
	char strport[MAX_TCP_PACKET_SIZE] = {0};
	struct sockaddr *sa;
	int slen;

	memset(&ai,0,sizeof(ai));
	ai.ai_family = AF_INET;
	ai.ai_socktype = SOCK_STREAM;
	evutil_snprintf(strport, sizeof(strport), "%d", port);

	if(evutil_getaddrinfo(server, strport, &ai, &aitop) !=0 )
	{
		log_debug("evutil_getaddrinfo failed\n");
		return -1;
	}
	
	sa = aitop->ai_addr;
	slen = aitop->ai_addrlen;
	
	memcpy(&connect->addr,sa,slen);
	
	evutil_freeaddrinfo(aitop);
	return 0;
}

/*****************************************************************************
 * 函 数 名  : tcp_read_handler_cb
 * 负 责 人  : Edward
 * 创建日期  : 2016年10月20日
 * 函数功能  : 监听并读取Socket传递的数据
 * 输入参数  : struct bufferevent *bev  event对象
               void *ctx                Socket对象
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
static void
tcp_read_handler_cb(struct bufferevent *bev, void *ctx)
{
	char buf[MaxCacheBufferLength] = {0};	
	hostCmd cmd;
	size_t totalMsgLen = 0;
	uint16_t cmdPacketLen = 0;
	uint16_t packetHeaderPos=0;
	
	tcp_connection_t * connect = ctx;

	log_debug("tcp_read_handler_cb++\n");

	//如果是连接服务的线程则，更新心跳定时
	if(connect->type == SERVER_CONNECT)
	{
		zbSoc_Heartbeat_Client_Report_refresh();
	}
	
	/* This callback is invoked when there is data to read on bev. */
	struct evbuffer *input = bufferevent_get_input(bev);
	
	while((totalMsgLen = evbuffer_get_length(input)) >= MinCmdPacketLength)
	{
		bzero(buf,sizeof(buf));
		evbuffer_copyout(input,buf,totalMsgLen);

		if(lookupFirstPacketHeader(buf,totalMsgLen,&packetHeaderPos) == FindNoHeader)
        {
        	log_debug("CMD NOT FOUND\n");
            evbuffer_drain(input,totalMsgLen);
            break;
        }

		if(packetHeaderPos != 0)
		{
			evbuffer_drain(input,packetHeaderPos);
			if((totalMsgLen = evbuffer_get_length(input)) < MinCmdPacketLength)
			{
				log_debug("length error\n");
				break;
			}
		}
		
		bzero(buf,sizeof(buf));
		//获取长度信息
		if(evbuffer_copyout(input,buf,MinCmdPacketLength)==-1)
		{
			log_debug("evbuffer_copyout ERROR\n");
			break;
		}

		cmdPacketLen = (((buf[CMD_MSG_LEN0_POS]<<8)&0xff00)|(buf[CMD_MSG_LEN1_POS]&0x00ff));
		
		if(cmdPacketLen+4 > totalMsgLen)
		{
			log_debug("[zbSocMsg]: Packet is incomplete,Wait to receive other datas\n");
        	break;
		}
		
		bzero(buf,sizeof(buf));

		if(evbuffer_remove(input,buf,cmdPacketLen+4)==-1)
		{
			log_debug("evbuffer_remove ERROR\n");
			break;
		}
		
		if(checkPacketFCS(&buf[CMD_MSG_LEN0_POS],(cmdPacketLen+3))==true)
		{
			log_debug("==================MSG FROM SOCKET====================\n");
			log_debug("CMD:");
			log_debug_array(buf,cmdPacketLen+4,NULL);
			log_debug("\n");
			memcpy(cmd.data,&buf[CMD_MSG_TP_POS],(cmdPacketLen));
			cmd.size = cmdPacketLen;
			cmd.idx = 0;
			if(connect->tcp_Read_Handler_cb)
				connect->tcp_Read_Handler_cb(&cmd);
		}
	}
	log_debug("tcp_read_handler_cb--\n");
}

static void
tcp_client_event_handler_cb(struct bufferevent *bev, short events, void *ctx)
{
	int err;
	evutil_socket_t s;
	tcp_connection_t *connect = ctx;
	int optval;
	socklen_t optlen = sizeof(optval);

	s = bufferevent_getfd(bev);	
	
	//连接成功
	if(events & BEV_EVENT_CONNECTED)
	{
		log_debug("BEV_EVENT_CONNECTED\n");
		connect->connected = true;
		
		if(connect->type == SERVER_CONNECT)
		{
			//连接检测，当超时时，调用重连
			zbSoc_Heartbeat_Client_Report_start(start_tcp_client_failed_cb,connect);
			//连接成功后5秒发送心跳和注册消息
			zbSoc_Heartbeat_Connected_Report_start();

#ifdef SUPPORT_GPRS_MODULE
			connect->devType = NETWORK_CARD;
#endif
		}
		
#if PERMIMNG
		if(connect->type == PERMISSION_CONNECT)
		{
			//发送权限获取命令
			zbSoc_Heartbeat_Permimng_Report_start();
		}
#endif

		connect->socketFd = s;

		//对sock_cli设置KEEPALIVE和NODELAY
		setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);//使用KEEPALIVE
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &optval, optlen);//禁用NAGLE算法
	}
	else if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		if(events & BEV_EVENT_ERROR)
		{
			err = bufferevent_socket_get_dns_error(bev);
			if(err)
				log_debug("DNS error:%s\n",evutil_gai_strerror(err));
		}
		
		log_debug("BEV_EVENT_ERROR:%s\n",evutil_socket_error_to_string(evutil_socket_geterror(s)));
		
		if(connect->type == SERVER_CONNECT)
		{
			log_debug("Set Reconnect times++\n");
			tu_set_evtimer(&connect->evtimer,SERVER_RECONNECTION_RETRY_TIME,ONCE,start_tcp_client_failed_cb,connect);
			log_debug("Set Reconnect times--\n");
			zbSoc_Heartbeat_Client_Report_stop();
			zbSoc_Heartbeat_Connected_Report_stop();
			roomfairy_registerFlag = false;	
			
#ifdef SUPPORT_GPRS_MODULE
			connect->devType = GPRS_MODULE;
#endif
		}
		
#if PERMIMNG
		if(connect->type == PERMISSION_CONNECT)
		{
			uint64_t times = PermiMng_getRequestIntr();
	
			log_debug("PERMISSION_CONNECT++:%d\n",PERMISSION_CONNECT);
			tu_set_evtimer(&connect->evtimer,times,ONCE,start_tcp_permimng_client_failed_cb,connect);
			log_debug("PERMISSION_CONNECT--:%d\n",PERMISSION_CONNECT);
	
			zbSoc_Heartbeat_Permimng_Report_stop();
			
		}
#endif
		//bufferevent_free(bev);
		if(connect->dns_base)
			evdns_base_free(connect->dns_base, 0);	
		if(connect->bevent)
			bufferevent_free(connect->bevent);	
		
		connect->connected = false;
		connect->socketFd = -1;
		connect->bevent = NULL;
		connect->dns_base = NULL;
	}

}

void start_tcp_client_failed_cb(void * args)
{ 
	ASSERT(args != NULL);

	static uint8_t ledcnt = 0;
	
	tcp_connection_t *connect = args;

	log_debug("start_tcp_client_failed_cb++\n");
	
	if(connect->connected == true)
	{
		log_debug("connect->connected == true\n");
		if(connect->bevent != NULL)
		{
			log_debug("connect->bevent != NULL\n");
			bufferevent_free(connect->bevent);
			connect->bevent = NULL;
			connect->connected = false;
		}
		
		ledcnt = 0;
	}

	if(ledcnt == 0)
	{
		led_SetSysLedStatus(SYS_LED,LED_BLINK);
		//led_SetSysLedStatus(CONNECT_LED,LED_BLINK);
		led_SetSysLedStatus(CONNECT_LED,LED_OFF);
	}
	
	if(ledcnt++ > 10)
	{
		ledcnt = 0;	
	}
	
	start_tcp_client((tcp_connection_t *)args);
	log_debug("start_tcp_client_failed_cb--\n");
}

static bool start_tcp_client_connect(tcp_connection_t *connect)
{
	ASSERT(connect != NULL);
	//struct evdns_base *dns_base = NULL;  

	//log_debug("Address: %s:%d\n",inet_ntoa(connect->addr.sin_addr),ntohs(connect->addr.sin_port));

	connect->dns_base = evdns_base_new(connect->base, 1);  
	if(connect->dns_base == NULL)
	{
		log_debug("evdns_base_new error\n");
		goto error;
	}
	
	connect->bevent = bufferevent_socket_new(connect->base, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS);	
	
	if(connect->bevent == NULL)
	{
		log_debug("bufferevent_socket_new failed\n");
		goto error;
	}

	//bufferevent_priority_set(connect->bevent,0);
	
	bufferevent_setcb(connect->bevent, tcp_read_handler_cb,NULL,tcp_client_event_handler_cb, (void*)connect);	
	if(bufferevent_enable(connect->bevent, EV_READ)==-1)
	{
		log_debug("bufferevent_enable failed\n");
		goto error;
	}

	if(bufferevent_socket_connect_hostname(connect->bevent,connect->dns_base,AF_INET,connect->s_addr,connect->s_port))
	{
		log_debug("bufferevent_socket_connect_hostname failed\n");
		goto error;
	}
	
/*
	if(bufferevent_socket_connect(connect->bevent, (struct sockaddr *)&connect->addr, sizeof(connect->addr))==-1)
	{
		log_debug("bufferevent_socket_connect failed\n");
		goto error;
	}
*/

	log_debug("wait for connecting...\n");
	return true;
	
error:
	if(connect->dns_base)
		evdns_base_free(connect->dns_base, 0);
	
	if(connect->bevent)
		bufferevent_free(connect->bevent);	
	
	connect->connected  = false;
	connect->socketFd = -1;
	
	return false;
}

static bool start_tcp_client(tcp_connection_t *connect)
{
	int ret = -1;
	ASSERT(connect != NULL);

	uint8_t s_addrs[256]  = {0};
	uint16_t s_port = 0;

	//if(SocketInterface_GetMacAddr()==false)
	if(netUtils_getHostMacByUci(roomfairy_WifiMac) != NETUTILS_SUCC)
	{
		log_debug("Get MAC Addr Failed\n");
		goto error;
	}

	log_debug("Wifi Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",roomfairy_WifiMac[0],roomfairy_WifiMac[1],roomfairy_WifiMac[2],roomfairy_WifiMac[3],roomfairy_WifiMac[4],roomfairy_WifiMac[5]);

#if PERMIMNG
	//权限认证没通过
	if(g_PermiMngisPass == false)
	{
		log_debug("g_PermiMngisPass == false\n");
		goto error;
	}
#endif

	//获取服务器地址和端口号
	if(getWifiServerInfofile(s_addrs,&s_port) !=0 )
	{
	  log_debug("Can't get Remote Service Information\n");
	  goto error;
	}
	
	log_debug("Address:%s:%d\n",s_addrs,s_port);

	connect->s_addr = s_addrs;
	connect->s_port = s_port;
	
/*
	if(socket_getaddrinfo(connect,s_addrs,s_port) == -1)
	{
		log_debug("socket_getaddrinfo failed\n");
		goto error;
	}
*/	
	
	if(start_tcp_client_connect(connect) == false)
	{	
		log_debug("start_tcp_client_connect++\n");
		goto error;
	}
	
	return true;
	
error:
	ret = tu_set_evtimer(&connect->evtimer,SERVER_RECONNECTION_RETRY_TIME,ONCE,start_tcp_client_failed_cb,connect);
	if(ret == -1)
	{
		return false;
	}
	return true;
}

bool tcp_client_evInit(struct event_base *base)
{
	tcp_connection_t *connect = NULL;
	tcp_connection_t *srchRec = NULL;
//	tu_evtimer_t evtimer;
	ASSERT(base != NULL);

	connect  = (tcp_connection_t*)malloc(sizeof(tcp_connection_t));
	if(connect == NULL)
	{
		log_debug("malloc failed\n");
		goto error;
	}
	
	connect->next = NULL;
	connect->base = base;
	connect->connected = false;
	connect->socketFd = -1;
	connect->tcp_Read_Handler_cb = SRPC_ProcessIncoming;
	connect->type = SERVER_CONNECT;

	connect->evtimer.base = base;
	connect->evtimer.evtimer = NULL;
	connect->evtimer.in_use= false;
	connect->evtimer.fd = -1;
	
	if(start_tcp_client(connect)==false)
	{
		log_debug("start_tcp_client error\n");
		goto error;
	}

	if(tcp_connection_head == NULL)
	{
		tcp_connection_head = connect;
	}
	else
	{
		srchRec  = tcp_connection_head;
		while(srchRec->next != NULL)
			srchRec = srchRec->next;
		srchRec->next = connect;
	}
	
	return true;
	
error:
	
	if(connect != NULL)
	{
		log_debug("aaa\n");
		free(connect);
	}	
	return false;
}

#if PERMIMNG
void start_tcp_permimng_client_failed_cb(void * args)
{ 
	log_debug("start_tcp_permimng_client_failed_cb++\n");
	tcp_connection_t *connect = args;
	if(connect->connected == true)
	{
		log_debug("connect->connected == true\n");
		if(connect->bevent != NULL)
		{
			log_debug("connect->bevent != NULL\n");
			bufferevent_free(connect->bevent);
			connect->bevent = NULL;
		}
	}
	
	start_tcp_permimng_client((tcp_connection_t *)args);
	log_debug("start_tcp_permimng_client_failed_cb--\n");
}


bool start_tcp_permimng_client(tcp_connection_t *connect)
{
	ASSERT(connect != NULL);

	log_debug("start_tcp_permimng_client++\n");
	int ret = -1;
	
	if(socket_getaddrinfo(connect,PermiMngUrl,PermiMngPort) == -1)
	{
		log_debug("socket_getaddrinfo failed\n");
		goto error;
	}
	
	if(start_tcp_client_connect(connect) == false)
	{	
		log_debug("start_tcp_client_connect++\n");
		goto error;
	}
	
	log_debug("start_tcp_permimng_client--\n");

	return true;
	
error:

	ret = tu_set_evtimer(&connect->evtimer,PermiMngRequestFaileTime,ONCE,start_tcp_permimng_client_failed_cb,connect);

	if(ret == -1)
	{
		return false;
	}
	
	return true;
}



tcp_connection_t* tcp_permimng_client_evInit(struct event_base *base)
{
	tcp_connection_t *connect = NULL;
	tcp_connection_t *srchRec = NULL;
//	tu_evtimer_t evtimer;
	ASSERT(base != NULL);

	connect  = (tcp_connection_t*)malloc(sizeof(tcp_connection_t));
	if(connect == NULL)
	{
		log_debug("malloc failed\n");
		goto error;
	}
	
	connect->next = NULL;
	connect->base = base;
	connect->connected = false;
	connect->socketFd = -1;
	connect->tcp_Read_Handler_cb = SRPC_ProcessIncoming;
	connect->type = PERMISSION_CONNECT ;

	connect->evtimer.base = base;
	connect->evtimer.evtimer = NULL;
	connect->evtimer.in_use= false;
	connect->evtimer.fd = -1;
	
#if 0	
	if(start_tcp_permimng_client(connect)==false)
	{
		log_debug("start_tcp_client error\n");
		goto error;
	}
#endif

	if(tcp_connection_head == NULL)
	{
		tcp_connection_head = connect;
	}
	else
	{
		srchRec  = tcp_connection_head;
		while(srchRec->next != NULL)
			srchRec = srchRec->next;
		srchRec->next = connect;
	}
	
	return connect;
	
error:
	
	if(connect != NULL)
	{
		log_debug("aaa\n");
		free(connect);
	}	
	return NULL;
}

#endif

bool tcp_client_send_msg(uint8_t *cmdMsg,uint16_t cmdMsgLen)
{

    tcp_connection_t *srchRec = tcp_connection_head;

	while(srchRec!=NULL)
	{
		if((true == srchRec->connected) 
#if PERMIMNG
		&& (g_PermiMngisPass == true) 
#endif
		)
		{
			if(srchRec->type != PERMISSION_CONNECT)
			{
				bufferevent_write(srchRec->bevent,cmdMsg,cmdMsgLen);
			}
		}
		srchRec = srchRec->next;
		
	//	usleep(1000);
	}


    return true;
}

//////////////////////////////////////////////////////////////////////////////////////
//						Server AP
/////////////////////////////////////////////////////////////////////////////////////

void tcp_Server_delete_connect(tcp_connection_t *connect)
{
    tcp_connection_t *srchRec=NULL, *prevRec = NULL;

	log_debug("tcp_Server_deleteSocketRec\n");
    // Head of the timer list
    srchRec = tcp_connection_head;

    // Stop when rec found or at the end
    while ((srchRec->socketFd!= connect->socketFd) && (srchRec->next))
    {
        prevRec = srchRec;
        // over to next
        srchRec = srchRec->next;
    }

    if (srchRec->socketFd!= connect->socketFd)
    {
        log_debug("deleteSocketRec: record not found\n");
        return;
    }

    // Does the record exist
    if (srchRec)
    {
        // delete the timer from the list
        if (prevRec == NULL)
        {
            //trying to remove first rec, which is always the listining socket
			prevRec = srchRec->next;
			free(srchRec);
			tcp_connection_head = prevRec;
			return;	
        }

        //remove record from list
        prevRec->next = srchRec->next;
        srchRec->connected = false;
        free(srchRec);
    }
}

static void
tcp_server_event_handler_cb(struct bufferevent *bev, short events, void *ctx)
{
	const char *err;
	evutil_socket_t s;
	tcp_connection_t *connect = ctx;

	//连接成功
	if(events & BEV_EVENT_CONNECTED)
	{
		log_debug("BEV_EVENT_CONNECTED\n");
		connect->connected = true;
		//连接成功后5秒发送心跳和注册消息
	}
	
	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) 
	{
		log_debug("BEV_EVENT_CONNECTED\n");
		s = bufferevent_getfd(bev);	
		err = evutil_socket_error_to_string(evutil_socket_geterror(s));
		log_debug("%s\n",err);
		bufferevent_free(bev);
		tcp_Server_delete_connect(connect);
	}
}

static void 
accept_conn_cb(struct evconnlistener *listener,evutil_socket_t fd, struct sockaddr *address, int socklen,void *ctx)
{  
//	struct sockaddr_in *src;
	tcp_connection_t *connect  = NULL;
	tcp_connection_t *srchRec  = NULL;
	int optval;
	socklen_t optlen = sizeof(optval);
	
	connect = (tcp_connection_t*)malloc(sizeof(tcp_connection_t));
	if(connect==NULL)
	{
		log_debug("malloc failed\n");
		goto error;
	}

	/* We got a new connection! Set up a bufferevent for it. */
	connect->base  = evconnlistener_get_base(listener);
	connect->bevent = bufferevent_socket_new(connect->base, fd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS); 
	if(connect->bevent==NULL)
	{
		log_debug("Error constructing bufferevent!\n");
		goto error;
	}

	connect->socketFd = fd;
	memcpy(&connect->addr,address,socklen);
	connect->tcp_Read_Handler_cb = SRPC_ProcessIncoming;
	connect->connected = true;
	connect->next = NULL;
	connect->type = CLIENT_CONNECT;

	//对sock_cli设置KEEPALIVE和NODELAY
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);//使用KEEPALIVE
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, optlen);//禁用NAGLE算法
	
	bufferevent_setcb(connect->bevent, tcp_read_handler_cb, NULL, tcp_server_event_handler_cb, connect);  
	bufferevent_enable(connect->bevent, EV_READ);
	
	//add list
	if(tcp_connection_head == NULL)
	{
		tcp_connection_head = connect;
	}
	else
	{
		srchRec  = tcp_connection_head;
		while(srchRec->next != NULL)
			srchRec = srchRec->next;
		srchRec->next = connect;
	}

	log_debug("Connect from: %s:%d\n",inet_ntoa(connect->addr.sin_addr),ntohs(connect->addr.sin_port));
	
	return ;
	
error:
	log_debug("accept_conn_cb error\n");

	if(connect!=NULL)
	{
		if(connect->bevent != NULL)
			bufferevent_free(connect->bevent);
		free(connect);
	}

	return;
}


bool tcp_server_evInit(struct event_base *base)
{
	struct evconnlistener *listener = NULL;
	struct sockaddr_in sin;
	ASSERT(base != NULL);

	memset(&sin, 0, sizeof(sin));
	sin.sin_port = htons(LOCAL_SERVER_TCP_PORT);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	
	listener = evconnlistener_new_bind(base, accept_conn_cb, (void *)base,
			LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
			(struct sockaddr*)&sin,
			sizeof(sin));	
	if(!listener)
	{
		log_debug("evconnlistener_new_bind failed\n");
		return false;
	}
	
	tcp_server_head.listener = listener;
	tcp_server_head.base = base;
	
	return true;
}	

void tcp_relase(void)
{
	log_debug("FUN11");
	tcp_connection_t *srchRec =NULL, *NextRec = NULL;
	srchRec = tcp_connection_head;
	log_debug("FUN1");
	while(srchRec!=NULL)
	{
		NextRec = srchRec->next;
		if(srchRec->socketFd != -1)
		{
			log_debug("FUN2");
			close(srchRec->socketFd);
		}
		if(srchRec->evtimer.evtimer)
		{
			log_debug("FUN3");
			event_free(srchRec->evtimer.evtimer);
		}
		free(srchRec);
		tcp_connection_head = NextRec;
		srchRec = NextRec;
	}
	log_debug("FUN4");
	if(tcp_server_head.listener)
	{
		log_debug("FUN5");
		evconnlistener_free(tcp_server_head.listener);
	}
	log_debug("FUN6");
}

