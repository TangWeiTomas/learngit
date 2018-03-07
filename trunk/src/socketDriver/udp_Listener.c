#include "udp_Listener.h"
#include "string.h"
#include <event2/event.h>
#include "interface_srpcserver_defs.h"
#include <sys/socket.h>
#include "comParse.h"
#include "libsocketudp.h"
#include "Timer_utils.h"


#define UDP_FIFO_SIZE	1024

typedef struct
{
	uint8_t buf[UDP_FIFO_SIZE];
	uint16_t bufsize;
}buf_t;

typedef struct 
{
	udp_t *udp_handler;
	struct event *udp_event;
	struct event_base *base;
	void (* udp_eventMsg_cb)(hostCmd *cmd, void*args );
}udpInfo_t;

static buf_t udpBuffer;

#define UDP_ATTRIBUTE	(EV_READ|EV_PERSIST)

static udpInfo_t *udpInfo = NULL;

static tu_evtimer_t udpTimers;
static struct event_base *gbase = NULL;

static void udpBroadcast_BroadcastMsg(hostCmd *cmd,char *ip,int port)
{
	memset(cmd,0,sizeof(hostCmd));
	cmd->idx = 0;
	
	makeMsgHeader(cmd,CMD_MSG_DIR_CFM);
	//D0 D1 Opcode
	cmdSet16bitVal(cmd,0xFF01);
	//D2-D9 IP
	cmdSet8bitVal(cmd,strlen(ip));
	cmdSetStringVal(cmd,ip,strlen(ip));
	//D10 端口号
	cmdSet16bitVal(cmd,port);
	makeMsgEnder(cmd);
}

//消息处理
static void udpBroadcast_eventMsg_cb(hostCmd *cmd,void *args)
{
	ASSERT(cmd!=NULL && args !=NULL);

	char host[NI_MAXHOST] = {0};
	uint8_t _wifiMac[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	hostCmd SendCmds;
	//udpListener_t *udp = args;
	udpInfo_t *udp = args;

	uint16_t _usrtp = 0;
	uint8_t _mac[7] = {0};
	uint8_t _dir = 0;
	uint16_t _cmds = 0;
	
	cmdGet16bitVal(cmd,&_usrtp);
	cmdGetStringVal(cmd,_mac,6);
	cmdGet8bitVal(cmd, &_dir);
	cmdGet16bitVal(cmd,&_cmds);

	log_debug("udpBroadcast_eventMsg_cb++\n");

	//屏蔽自身发送的反馈及上报命令
	if(_dir == CMD_MSG_DIR_CFM || _dir == CMD_MSG_DIR_IND)
		return;
	
	if(_cmds != 0XFF01)
	{
		log_debug("opcode = 0x%x\n",_cmds);
		return;
	}

	/*当MAC地址为本机的MAC或者全为0XFF*/
	if((strncmp(_mac,roomfairy_WifiMac,6))&&(strncmp(_mac,_wifiMac,6)))
	{
		log_debug("mac match failed %x%x%x%x%x%x %x%x%x%x%x%x \n",_mac[0],_mac[1],_mac[2],_mac[3],_mac[4],_mac[5],\
		roomfairy_WifiMac[0],roomfairy_WifiMac[1],roomfairy_WifiMac[2],roomfairy_WifiMac[3],roomfairy_WifiMac[4],roomfairy_WifiMac[5]);
		return ;
	}

	//获取网关IP
//	if(!SocketInterface_Getgateway(host))//false
	if(netUtils_getHostAddrs(host))//false
	{
		log_err("SocketInterface_Getgateway failed\n");
		return;
	}

	/*
	//过来本机发送的UDP广播包
	if(strcmp(inet_ntoa(udp->udp_handler->recvfroms.sin_addr),host)==0)
	{
		log_debug("revice from self\n");
		return;
	}
	*/
	
	udpBroadcast_BroadcastMsg(&SendCmds,host,LOCAL_SERVER_TCP_PORT);
	
	log_debug("%d\n",SendCmds.idx);

	if(udp_send(udp->udp_handler,SendCmds.data, SendCmds.idx)==-1)
	{
		log_err("udp_send error\n");
	}
	
	log_debug("udpBroadcast_eventMsg_cb--\n");
	
}

//消息接收
static void udpBroadcast_eventReceive_cb(evutil_socket_t udpsocketfd, short event, void *args)
{
	ASSERT(args != NULL);
//	udpListener_t *udp = args;
	int byteRead = 0;
    uint16_t packetHeaderPos=0;
    uint8_t cmdPacketLen;
    hostCmd cmd;
    udpInfo_t *udp = (udpInfo_t *)args;
    int len = sizeof(struct sockaddr_in);

    log_debug("udpBroadcast_eventReceive_cb++\n");
    
	byteRead = udp_recv(udp->udp_handler,&udpBuffer.buf[udpBuffer.bufsize], (MaxCacheBufferLength-udpBuffer.bufsize));
	
    if(byteRead<=0)
	{
		log_debug("Read Failed\n");

		//重启
		udpBroadcast_Restart(udp);
		return;
    }

    udpBuffer.bufsize += byteRead;
	
    while (udpBuffer.bufsize >= MinCmdPacketLength)
    {

        bzero(&cmd, sizeof(cmd));

        if(lookupFirstPacketHeader(udpBuffer.buf,udpBuffer.bufsize,&packetHeaderPos) == FindNoHeader)
        {
            memset(&udpBuffer,0,sizeof(udpBuffer));
            break;
        }
		
        //丢弃非法包头数据
        if(packetHeaderPos != 0)
        {
            memmove(udpBuffer.buf,&udpBuffer.buf[packetHeaderPos],(udpBuffer.bufsize-packetHeaderPos));
            udpBuffer.bufsize -= packetHeaderPos;
        }

        cmdPacketLen = (((udpBuffer.buf[CMD_MSG_LEN0_POS]<<8)&0xff00)|(udpBuffer.buf[CMD_MSG_LEN1_POS]&0x00ff));

        log_debug("Message cmd length = %d recvData.bufferNum=%d\n",cmdPacketLen,udpBuffer.bufsize);

        // 2byte(TP)+6byte(Mac)+1byte(Dir)+2byte(D0,D1)
        if((cmdPacketLen+4) > udpBuffer.bufsize)
        {
            log_debug("[localServer]: Packet length error,Wait to receive other datas\n");
            break;
        }

        if(checkPacketFCS(&udpBuffer.buf[CMD_MSG_LEN0_POS],cmdPacketLen+3)==true)
        {
            memcpy(cmd.data,&udpBuffer.buf[CMD_MSG_TP_POS],cmdPacketLen);
            cmd.size = cmdPacketLen;
            cmd.idx = 0;
           	udp->udp_eventMsg_cb(&cmd,udp);
        }
        else
        {
            log_debug("Drop the error fcs data packet.\n");
        }

        if(udpBuffer.bufsize>=(cmdPacketLen+4))
        {
            memmove(udpBuffer.buf,&udpBuffer.buf[cmdPacketLen+4],(udpBuffer.bufsize-(cmdPacketLen+4)));
            udpBuffer.bufsize -= (cmdPacketLen+4);
        }
        else
        {
           memset(&udpBuffer,0,sizeof(udpBuffer));
        }
    }	
	   

    log_debug("udpBroadcast_eventReceive_cb--\n");
	
}

static int8_t udpBroadcast_SetEvent(udpInfo_t *udp)
{
	ASSERT( udp!= NULL );

	int rtn = -1;

	do
	{
		udp->udp_event = event_new(udp->base,udp->udp_handler->handle,UDP_ATTRIBUTE,udpBroadcast_eventReceive_cb,udp);
		if(udp->udp_event == NULL)
		{
			log_err("event_new error\n");
			break;
		}

		rtn = event_add(udp->udp_event,NULL);
		if(rtn == -1)
		{
			log_err("event_add error\n");
			break;
		}

		return 0;
	}while(0);
	
	return -1;
}


int8_t udpBroadcast_Init(struct event_base *base)
{
	ASSERT(base != NULL);

	do{
		if(udpInfo == NULL)
		{
			udpInfo = (udpInfo_t*)malloc(sizeof(udpInfo_t));
			if(udpInfo == NULL)
			{
				log_err("malloc error\n");
				break;
			}
		}
		
		udpInfo->base = base;
		udpInfo->udp_handler = udp_new(LOCAL_SERVER_UDP_PORT);
		
		if(udpInfo->udp_handler == NULL)
		{
			log_err("udp_new error\n");
			break;
		}

		if(-1 == udpBroadcast_SetEvent(udpInfo))
		{
			log_err("udplisten_AddEvent error\n");
			break;
		}

		udpInfo->udp_eventMsg_cb = udpBroadcast_eventMsg_cb;
		return 0;
	}while(0);

	udpBroadcast_relase();
	return -1;	
}


static void udpBroadcast_Restartcb(void * args)
{
	udpBroadcast_Init((struct event_base*)args);
}

void udpBroadcast_Restart(udpInfo_t *udpInfo)
{
	gbase =  udpInfo->base;
	if(tu_set_evtimer(&udpTimers,5000,ONCE,udpBroadcast_Restartcb,gbase) < 0)
		log_err("tu_set_evtimer error\n");
	udpBroadcast_relase();
}

void udpBroadcast_relase(void)
{
	if(udpInfo != NULL)
	{
		if(udpInfo->udp_handler != NULL)
			udp_destroy(&(udpInfo->udp_handler));
		if(udpInfo->udp_event != NULL)
			event_free(udpInfo->udp_event);
		free(udpInfo);
		udpInfo=NULL;
	}	
}
