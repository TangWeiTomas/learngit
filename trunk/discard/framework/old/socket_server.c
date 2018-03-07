/**************************************************************************************************
* Filename:       socket_server.c
* Author:             zxb      
* E-Mail:          zxb@yystart.com 
* Description:    Handle socket server.
*
*  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/ 
* 
* Version:         1.00  (2014-11-24,19:40)    :   Create the file.
*                  
*
*************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include <netinet/if_ether.h>
#include <net/if.h>

//Application
//#include "interface_srpcserver.h"
#include "socket_server.h"
#include "cJSON.h"

//New Add
#include "interface_srpcserver.h"
#include "comMsgPrivate.h"
#include "globalVal.h"
//#include "cmdMsgSend.h"
#include "LogUtils.h"
/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * DEFINE
 */


/*********************************************************************
 * TYPEDEFS
 */


socketServerCb_t socketServerRxCb;
socketServerCb_t socketServerConnectCb;


dataBuffer_t  recvData;



/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */
static void deleteSocketRec(int rmSocketFd);
static int createSocketRec(void);

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/


/************************************************************************
* 函数名 :socketDataInit(void)
* 描述   :   Socket 初始化
* 输入   ：
* 输出   ：无
* 返回   ：无
************************************************************************/
void socketDataInit(void)
{
    
    //初始化接收的数据包
    memset(&recvData,0,sizeof(recvData));

    
}

/*********************************************************************
 * @fn      createSocketRec
 *
 * @brief   create a socket and add a rec fto the list.
 *
 * @param   table
 * @param   rmTimer
 *
 * @return  new clint fd
 */
int createSocketRec(void)
{
	int tr = 1;
	socketRecord_t *srchRec;

	socketRecord_t *newSocket = malloc(sizeof(socketRecord_t));

	//open a new client connection with the listening socket (at head of list)
	newSocket->clilen = sizeof(newSocket->cli_addr);

	//Head is always the listening socket
	newSocket->socketFd = accept(socketRecordHead->socketFd,
			(struct sockaddr *) &(newSocket->cli_addr), &(newSocket->clilen));

	//printf("connected\n");

	if (newSocket->socketFd < 0) printf("ERROR on accept");

	// Set the socket option SO_REUSEADDR to reduce the chance of a
	// "Address Already in Use" error on the bind
	setsockopt(newSocket->socketFd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int));
	// Set the fd to none blocking
	fcntl(newSocket->socketFd, F_SETFL, O_NONBLOCK);

	printf("New Client Connected fd:%d - IP:%s\n", newSocket->socketFd, inet_ntoa(newSocket->cli_addr.sin_addr));

	newSocket->next = NULL;

	//find the end of the list and add the record
	srchRec = socketRecordHead;
	// Stop at the last record
	while (srchRec->next)
		srchRec = srchRec->next;

	// Add to the list
	srchRec->next = newSocket;

	return (newSocket->socketFd);
}

/*********************************************************************
 * @fn      deleteSocketRec
 *
 * @brief   Delete a rec from list.
 *
 * @param   table
 * @param   rmTimer
 *
 * @return  none
 */
 
void deleteSocketRec(int rmSocketFd)
{
	socketRecord_t *srchRec, *prevRec = NULL;

	// Head of the timer list
	srchRec = socketRecordHead;

	// Stop when rec found or at the end
	while ((srchRec->socketFd != rmSocketFd) && (srchRec->next))
	{
		prevRec = srchRec;
		// over to next
		srchRec = srchRec->next;
	}

	if (srchRec->socketFd != rmSocketFd)
	{
		printf("deleteSocketRec: record not found\n");
		return;
	}

	// Does the record exist
	if (srchRec)
	{
		// delete the timer from the list
		if (prevRec == NULL)
		{
			//trying to remove first rec, which is always the listining socket
			printf("deleteSocketRec: removing first rec, which is always the listining socket\n");
			return;
		}

		//remove record from list
		prevRec->next = srchRec->next;

		close(srchRec->socketFd);
		free(srchRec);
	}
}

void active_keepalive(int sockfd)
{
    int optval;
    socklen_t optlen = sizeof(optval);

    /* check the status for the keepalive option */
    if (getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) 
    {
        perror("getsockopt SO_KEEPALIVE failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("SO_KEEPALIVE is %s\n", optval ? "ON" : "OFF");

    /* set the option active */
    optval = 1;
    optlen = sizeof(optval);
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) 
    {
        printf("setsockopt SO_KEEPALIVE failed，reason: %m\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("SO_KEEPALIVE on socket\n");

    /* check the status again */
    if (getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) 
    {
        perror("getsockopt SO_KEEPALIVE again failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));
}

void set_keepalive_params(int sockfd, int idle, int count, int intvl)
{
    int keepalive_time = idle;
    int keepalive_probes = count;
    int keepalive_intvl = intvl;
    
    if (setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(int)) < 0) {
        perror("TCP_KEEPIDLE failed");
        return;
    }
    
    if (setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(int)) < 0) {
        perror("TCP_KEEPCNT failed");
        return;
    }

    if (setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &keepalive_intvl, sizeof(int)) < 0) {
        perror("TCP_KEEPINTVL failed");
        return;
    }

    return;
}	

void do_register(int remote_sockfd)
{
    /* 开启keepalive 选项*/
    active_keepalive(remote_sockfd);
    
    /* 设置keepalive 相关参数 */
    set_keepalive_params(remote_sockfd, 60, 5, 3);

}

void do_connet_server()
{
    socketDataInit();

    /* socket: create the socket */
    Remote_Socket->socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if(Remote_Socket->socketFd < 0) error("ERROR opening socket");
    
    struct sockaddr_in serveraddr;
    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr=inet_addr((const char*)RemoteServerIPAddr);//REMOTE_SERV_IP RemoteServerIPAddr
    serveraddr.sin_port=htons(RemoteServerPort);//REMOTE_SERV_PORT RemoteServerIPAddr

    do{
        /* connect: create a connection with the server */
        if (connect(Remote_Socket->socketFd, &serveraddr, sizeof(serveraddr)) != 0)
        {
            perror("connect to server error!\n");
            Remote_Socket->connection_flag=0;
            sleep(6);
        }
        else 
        {
            epInfo_t epInfo;
            Remote_Socket->connection_flag=1;
            memset(&epInfo,0,sizeof(epInfo));
            
            printf("do_connet_server 33333333333\n");
            
            pthread_mutex_lock(&m_db_mutex);

            if(devListGetDeviceByNaEp(epInfo.nwkAddr,epInfo.endpoint) == NULL)
            {
                printf("zigbeeDev_RoomfairyRegisterInd.\n");                
                memset(&epInfo,0,sizeof(epInfo));
                devListAddDevice(&epInfo);
                sleep(1);
                zigbeeDev_RoomfairyRegisterInd();
            }
            else
            {
                sleep(2);
                HeartPacketSend();
            }
            
            pthread_mutex_unlock(&m_db_mutex);

            break;
        }
    }while(1);
    
}

/************************************************************************
* 函数名 :do_connet_error(void)
* 描述   :   Socket连接出错处理
* 输入   ：
* 输出   ：无
* 返回   ：无
************************************************************************/
void do_connet_error(void)
{
    close(Remote_Socket->socketFd);
    Remote_Socket->connection_flag=0;
    do_connet_server();
    do_register(Remote_Socket->socketFd);
}

/************************************************************************
* 函数名 :lookupFirstPacketHeader(UINT8* data,UINT16 length,UINT16* startPos) 
* 描述   :   检测是否有完整数据包
* 输入   ：
* 输出   ：无
* 返回   ：int  返回:-1 数据包长度不对，0未找到包头，1找到包头
			startPos 是0x9f的位置 
************************************************************************/
uint8 lookupFirstPacketHeader(uint8* data,uint16 length,uint16* startPos)
{
    uint16 cnt;
    *startPos = 0xffff;

    //先查找包头
    for(cnt=0;cnt<length;cnt++)
    {
        if(data[cnt]==CMD_MSG_FLAG)
        {
            *startPos = cnt;
            return FindPacketHeader;
        }
    }

    return FindNoHeader;

}

/************************************************************************
* 函数名 :checkPacketFCS(UINT8* data,UINT16 length) 
* 描述   :   检测数据包校验是否正确
* 输入   ：
* 输出   ：无
* 返回   ：false 数据包校验不正确true:数据包校验正确
************************************************************************/
bool checkPacketFCS(uint8* data,uint16 length)
{
    uint16 cnt;
    uint8 result=0;
	LOG_PRINT("checkPacketFCS\n");
    //先查找包头
    for(cnt=0;cnt<length;cnt++)
    {
        result ^= data[cnt];
		printf("%x ",data[cnt]);
    }

    //校验结果为0，则表明校验正确
    if(result==0)
    {   
        return true;
    }
    
    return false;
}

/**
  ******************************************************************************
  * @fun     remoteClientMsgHandle 
  * @author  FSSmarthome
  * @in		 remote_sockfd 
  * @brief   Server Main process
  ******************************************************************************/
void remoteClientMsgHandle(int remote_sockfd)
{
    fd_set rset;
    struct timeval tv;
    int maxfd,nready;

    hostCmd cmd;
    int byteToRead;	
    int byteRead;	
    int rtn;
    
    int errcode;

    printf("\n Start To Remote Client Pthread... \n");

    //Socket 初始化
    socketDataInit();
    
    while (1)
    {
        FD_ZERO(&rset);

        // timeout setting
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        maxfd = remote_sockfd;
        FD_SET(remote_sockfd, &rset);
        nready = select(maxfd + 1, &rset, NULL, NULL, &tv); //select返回表示检测到可读事件

        if (nready == -1){
            perror("select error");
            //break;
        }  

        if (nready == 0){
            printf("select time out !\n");            
            socketDataInit();
            errno = 0;
            continue;
        }

        if (FD_ISSET(remote_sockfd, &rset))
        {
            errno = 0;
            printf("\n******Start To Read Socket...\n");

            //查看下缓冲区有多少数据需要读取
            rtn = ioctl(remote_sockfd, FIONREAD, &byteToRead);
            
            printf("byteToRead = %d.\n",byteToRead);

            if (rtn != 0)
            {
                printf("[remoteClient]: Socket error\n");
            }

            if (byteToRead < 0)
            { 
                printf("[remoteClient]: Error reading from socket\n");
                errcode = errno;
                if(errcode == ETIMEDOUT)
                {   //出现网络故障
                    printf("******The Network failure ! %d : %d \n", errno, ETIMEDOUT);
                    do_connet_error();                    
                    continue;
                }
                printf("[remoteClient]: Error No ETIMEDOUT\n");
            }
            else if (byteToRead  == 0) 
            {
                printf("[remoteClient]: The Server Have close\n");
                do_connet_error();
                continue;
            }
            else
            {
                byteRead = read(remote_sockfd, &recvData.buffer[recvData.bufferNum], (MaxCacheBufferLength-recvData.bufferNum));
                
                if (byteRead < 0)
                {
                    printf("[remoteClientMsgHandle]: error reading from socket\n");
                    errcode = errno;
                    if(errcode == ETIMEDOUT)
                    {   
                        //出现网络故障
                        printf("******The Network failure ! %d : %d \n", errno, ETIMEDOUT);
                        do_connet_error();
                    }
                }
                else if (byteRead == 0)
                {            
                    //出现连接已经断开
                    printf("The Server Have close\n");
                    do_connet_error();
                }
                else
                {
                   recvData.bufferNum += byteRead;
                }
            }
            
            while (recvData.bufferNum >= MinCmdPacketLength)
            {
                uint16 packetHeaderPos=0;
                uint16 cmdPacketLen;
                uint8 cmdPacketFCS;
                bzero(&cmd, sizeof(cmd));
                
                if(lookupFirstPacketHeader(recvData.buffer,recvData.bufferNum,&packetHeaderPos) == FindNoHeader)
                {
                    socketDataInit();
                    break;
                }
                
                //丢弃非法包头数据
                if(packetHeaderPos != 0)
                {
                    memmove(recvData.buffer,&recvData.buffer[packetHeaderPos],(recvData.bufferNum-packetHeaderPos));
                    recvData.bufferNum -= packetHeaderPos;
                }
                
                cmdPacketLen = (((recvData.buffer[CMD_MSG_LEN0_POS]<<8)&0xff00)|(recvData.buffer[CMD_MSG_LEN1_POS]&0x00ff));

                printf("[remoteClient]: Message cmd length = %d recvData.bufferNum=%d\n",cmdPacketLen,recvData.bufferNum);

                // 2byte(TP)+6byte(Mac)+1byte(Dir)+2byte(D0,D1)
                if((cmdPacketLen+4) > recvData.bufferNum)
                {
                    printf("[remoteClient]: Packet length error,Wait to receive other datas\n");
                    break;
                }

                if(checkPacketFCS(&recvData.buffer[CMD_MSG_LEN0_POS],cmdPacketLen+3)==true)
                {
                    memcpy(cmd.data,&recvData.buffer[CMD_MSG_TP_POS],cmdPacketLen);
                    cmd.size = cmdPacketLen;
                    cmd.idx = 0;
                    
                    printf("remoteClientMsgHandle  SRPC_ProcessIncoming 4444444444\n");
                    
                    pthread_mutex_lock(&m_db_mutex);
                    SRPC_ProcessIncoming(&cmd);
                    pthread_mutex_unlock(&m_db_mutex);
                }
                else
                {
                    printf("[remoteClient]:Drop the error fcs data packet.\n");
                }

                if(recvData.bufferNum>=(cmdPacketLen+4))
                {
                    memmove(recvData.buffer,&recvData.buffer[cmdPacketLen+4],(recvData.bufferNum-(cmdPacketLen+4)));
                    recvData.bufferNum -= (cmdPacketLen+4);
                }
                else
                {
                    socketDataInit();
                }
            }
        }
    }
    
    close(remote_sockfd);
    
    Remote_Socket->connection_flag=0;

}

/**
  ******************************************************************************
  * @fun     acceptThreadFun_server 
  * @author  FSSmarthome
  * @in		 void * 
  * @brief   远程连接线程处理函数
  ******************************************************************************/
void* acceptThreadFun_server(void *arg)
{
    do_connet_server();
    do_register(Remote_Socket->socketFd);
    remoteClientMsgHandle(Remote_Socket->socketFd);
}

/***************************************************************************************************
 * @fn      serverSocketInit
 *
 * @brief   initialises the server.
 * @param   
 *
 * @return  Status
 */
int32 socketSeverInit(uint32 port)
{
    struct sockaddr_in serv_addr;
    int stat, tr = 1;
    int errVal;

    if (socketRecordHead == NULL)
    {
        // New record
        socketRecord_t *lsSocket = malloc(sizeof(socketRecord_t));
#ifndef UNGET_WIFIMAC        
        char *device="eth0"; //eth0是网卡设备名
        struct ifreq req;
#endif
        lsSocket->socketFd = socket(AF_INET, SOCK_STREAM, 0);
        if (lsSocket->socketFd < 0)
        {
            printf("ERROR opening socket");
			free(lsSocket);
            return -1;
        }

#ifndef UNGET_WIFIMAC
        strcpy(req.ifr_name,device); //将设备名作为输入参数传入
        errVal=ioctl(lsSocket->socketFd,SIOCGIFHWADDR,&req); //执行取MAC地址操作
        if(errVal != -1)
        {
            memcpy(roomfairy_WifiMac,req.ifr_hwaddr.sa_data,ETH_ALEN); //取输出的MAC地址
            printf("Wifi Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",roomfairy_WifiMac[0],roomfairy_WifiMac[1],roomfairy_WifiMac[2],roomfairy_WifiMac[3],roomfairy_WifiMac[4],roomfairy_WifiMac[5]);
        }
#endif

        // Set the socket option SO_REUSEADDR to reduce the chance of a
        // "Address Already in Use" error on the bind
        setsockopt(lsSocket->socketFd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int));
        // Set the fd to none blocking
        fcntl(lsSocket->socketFd, F_SETFL, O_NONBLOCK);

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        stat = bind(lsSocket->socketFd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
        
        if (stat < 0)
        {
            printf("ERROR on binding: %s\n", strerror(errno));
            return -1;
        }

        //will have 5 pending open client requests
        listen(lsSocket->socketFd, 5);

        lsSocket->next = NULL;
        //Head is always the listening socket
        socketRecordHead = lsSocket;

        //远程socket创建        
        Remote_Socket = malloc(sizeof(socketRecord_t));
        Remote_Socket->next = NULL;
#if 0       
        if((host=gethostbyname("your.server"))==NULL) 
        { 
            perror("gethostbyname error"); 
            exit(1); 
        } 
#endif
        //读取当前服务器IP地址和端口号
        getWifiServerInfofile(RemoteServerIPAddr,&RemoteServerPort);
        
        printf("RemoteServerIPAddr %s ,RemoteServerPort %d \n",RemoteServerIPAddr,RemoteServerPort);
        
        pthread_t remote_client;
        
        int err=pthread_create(&remote_client,NULL,acceptThreadFun_server,NULL);    //创建线程来帮忙accept
        
        if(err!=0) printf("can not create thread : %s\n",strerror(errno));

    }

    //printf("waiting for socket new connection\n");

    return 0;
}

/***************************************************************************************************
 * @fn      serverSocketConfig
 *
 * @brief   register the Rx Callback.
 * @param   
 *
 * @return  Status
 */
int32 serverSocketConfig(socketServerCb_t rxCb, socketServerCb_t connectCb)
{
    socketServerRxCb = rxCb;
    socketServerConnectCb = connectCb;

    return 0;
}
/*********************************************************************
 * @fn      socketSeverGetClientFds()
 *
 * @brief   get clients fd's.
 *
 * @param   none
 *
 * @return  list of Timerfd's
 */
void socketSeverGetClientFds(int *fds, int maxFds)
{
    uint32 recordCnt = 0;
    socketRecord_t *srchRec;

    // Head of the timer list
    srchRec = socketRecordHead;

    // Stop when at the end or max is reached
    while ((srchRec) && (recordCnt < maxFds))
    {
        //printf("getClientFds: adding fd%d, to idx:%d \n", srchRec->socketFd, recordCnt);
        fds[recordCnt++] = srchRec->socketFd;

        srchRec = srchRec->next;
    }

    return;
}

/*********************************************************************
 * @fn      socketSeverGetNumClients()
 *
 * @brief   get clients fd's.
 *
 * @param   none
 *
 * @return  list of Timerfd's
 */
uint32 socketSeverGetNumClients(void)
{
    uint32 recordCnt = 0;
    socketRecord_t *srchRec;

    //printf("socketSeverGetNumClients++\n", recordCnt);

    // Head of the timer list
    srchRec = socketRecordHead;

    if (srchRec == NULL)
    {
        //printf("socketSeverGetNumClients: socketRecordHead NULL\n");
        return -1;
    }

    // Stop when rec found or at the end
    while (srchRec)
    {
        //printf("socketSeverGetNumClients: recordCnt=%d\n", recordCnt);
        srchRec = srchRec->next;
        recordCnt++;
    }

    //printf("socketSeverGetNumClients %d\n", recordCnt);
    return (recordCnt);
}

/*********************************************************************
 * @fn      socketSeverPoll()
 *
 * @brief   services the Socket events.
 *
 * @param   clinetFd - Fd to services
 * @param   revent - event to services
 *
 * @return  none
 */
void socketSeverPoll(int clinetFd, int revent)
{
    //printf("pollSocket++\n");
    //is this a new connection on the listening socket
    if (clinetFd == socketRecordHead->socketFd)
    {
        int newClientFd = createSocketRec();

        if (socketServerConnectCb)
        {
            socketServerConnectCb(newClientFd);
        }
    }
    else
    {
        //this is a client socket is it a input or shutdown event
        if (revent & POLLIN)
        {
            //uint32 pakcetCnt = 0;
            //its a Rx event
            //printf("got Rx on fd %d, pakcetCnt=%d\n", clinetFd, pakcetCnt++);
            if (socketServerRxCb)
            {
                socketServerRxCb(clinetFd);
            }
        }

        if (revent & POLLRDHUP)
        {
            //its a shut down close the socket
            printf("Client fd:%d disconnected\n", clinetFd);
            //remove the record and close the socket
            deleteSocketRec(clinetFd);
        }
    }

    return;
}

/***************************************************************************************************
 * @fn      socketSeverSend
 *
 * @brief   Send a buffer to a clients.
 * @param   uint8* srpcMsg - message to be sent
 *          int32 fdClient - Client fd
 *
 * @return  Status
 */
int32 socketSeverSend(uint8* buf, uint32 len, int32 fdClient)
{
    int32 rtn;

    //printf("socketSeverSend++: writing to socket fd %d\n", fdClient);
#if 0
    if (fdClient)
    {
        rtn = write(fdClient, buf, len);
        if (rtn < 0)
        {
            printf("ERROR writing to socket %d\n", fdClient);
            return rtn;
        }
    }
#endif
    if(Remote_Socket->connection_flag == 1)
    {
        rtn = write(Remote_Socket->socketFd, buf, len);
        if (rtn < 0)
        {
            printf("ERROR writing to socket %d\n", Remote_Socket->socketFd);
            return rtn;
        }
    }

    //printf("socketSeverSend--\n");
    return 0;
}

/***************************************************************************************************
 * @fn      socketSeverSendAllclients
 *
 * @brief   Send a buffer to all clients.
 * @param   uint8* srpcMsg - message to be sent
 *
 * @return  Status
 */
int32 socketSeverSendAllclients(uint8* buf, uint32 len)
{
    int rtn;
    socketRecord_t *srchRec;

    // first client socket
    srchRec = socketRecordHead->next;

    // Stop when at the end or max is reached
    while (srchRec)
    {
        //printf("SRPC_Send: client %d\n", cnt++);
        rtn = write(srchRec->socketFd, buf, len);
        if (rtn < 0)
        {
            printf("[SendAllclients]:ERROR writing to socket %d\n", srchRec->socketFd);
            printf("[SendAllclients]:closing client socket\n");
            //remove the record and close the socket
            deleteSocketRec(srchRec->socketFd);

            return rtn;
        }
        srchRec = srchRec->next;
    }

    if(Remote_Socket->connection_flag == 1)
    {
        rtn = write(Remote_Socket->socketFd, buf, len);
    }

    return 0;
}

/***************************************************************************************************
 * @fn      socketSeverClose
 *
 * @brief   Closes the client connections.
 *
 * @return  Status
 */
void socketSeverClose(void)
{
	int fds[MAX_CLIENTS], idx = 0;

	socketSeverGetClientFds(fds, MAX_CLIENTS);

	while (socketSeverGetNumClients() > 1)
	{
		printf("socketSeverClose: Closing socket fd:%d\n", fds[idx]);
		deleteSocketRec(fds[idx++]);
	}

	//Now remove the listening socket
	if (fds[0])
	{
		printf("socketSeverClose: Closing the listening socket\n");
		close(fds[0]);
	}
}





