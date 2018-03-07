/***********************************************************************************
 * 文 件 名   : net_utils.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年12月1日
 * 文件描述   : 网络相关的工具包
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/sockios.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/rtnetlink.h>    //for rtnetlink
#include <net/if.h> 			//for IF_NAMESIZ, route_info
#include <ifaddrs.h>


#include "net_utils.h"
#include "logUtils.h"
#include "config_operate.h"

#define BUFSIZE 8192

struct route_info
{
	u_int dstAddr;
	u_int srcAddr;
	u_int gateWay;
	char ifName[IF_NAMESIZE];
};


//通过获取网关是否被分配到IP来判断是否连接到路由器
int netUtils_isConnect(const char *eth)
{
	int sock_fd;	
	struct  sockaddr_in my_addr;	
	struct ifreq ifr;	
	char ipaddr[32]="\0";

	/* Get socket file descriptor */	
	if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)	
	{		
		perror("socket");		
		return NETUTILS_FAILE;	
	}	

	/* Get IP Address */
	strncpy(ifr.ifr_name, eth, IF_NAMESIZE);
	ifr.ifr_name[IFNAMSIZ-1]='\0';
	if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
	{
		log_debug("%s is Unconnected\n",eth);	
		close(sock_fd);
		return NETUTILS_FAILE;
	}	

	memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
	strcpy(ipaddr, inet_ntoa(my_addr.sin_addr));
	
	close(sock_fd);
	
	return NETUTILS_SUCC;
}

//判断指定的interface是否启动
int netUtils_isDetect(const char* eth)
{
	int skfd = 0;
	struct ifreq ifr;

	ASSERT(eth != NULL);

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0) 
	{
	        printf("%s:%d Open socket error\n", __FILE__, __LINE__);
	        return NETUTILS_FAILE;
	}

	strcpy(ifr.ifr_name, eth);

	if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 ) 
	{
	        printf("%s:%d IOCTL error\n", __FILE__, __LINE__);
	        printf("Maybe ethernet inferface %s is not valid\n", ifr.ifr_name);
	        close(skfd);
	        return NETUTILS_FAILE;
	}

	if(ifr.ifr_flags & IFF_RUNNING) 
	{
		return NETUTILS_UP; //up
	} 
	else 
	{
	    return NETUTILS_DOWN;//down
	}
}

int netUtils_getHostMacByUci(char resultMac[6])
{
	char *mac = NULL;
	char *delim=":";
	char *p;
	int cnt = 0;
	
	mac = network_mac_get();	
	if(mac == NULL)
	{
		return NETUTILS_FAILE;
	}

	p = strtok(mac,delim);
	while(p != NULL)
	{
		if(cnt > 6)
		{
			network_mac_free(mac);
			return NETUTILS_FAILE;
		}
		
		resultMac[cnt++]=(strtol(p,NULL,16)&0xff);
		p = strtok(NULL,delim);
	}
	
	//log_debug("Wifi Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",resultMac[0],resultMac[1],resultMac[2],resultMac[3],resultMac[4],resultMac[5]);

	network_mac_free(mac);
	
	return NETUTILS_SUCC;
}


int netUtils_getHostMacByIoctl(const char *device,char resultMac[6])
{
    struct ifreq req;
    int socketFd = 0;
    
	ASSERT(device != NULL);

#if 0
	//if((socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP))<0)
	if((socketFd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        log_debug("ERROR: Can't create socket\n");
        return NETUTILS_FAILE;
    }
	
	memset(&req,0,sizeof(req));
    strncpy(req.ifr_name,device,sizeof(req.ifr_name)-1); //将设备名作为输入参数传入
    
    if(ioctl(socketFd,SIOCGIFHWADDR,&req) < 0) //执行取MAC地址操作
    {
        log_err("ERROR: Socket ioctl fail \n");
        close(socketFd);
        return NETUTILS_FAILE;
    }

	bzero(roomfairy_WifiMac,sizeof(roomfairy_WifiMac));
    memcpy(roomfairy_WifiMac,req.ifr_hwaddr.sa_data,ETH_ALEN); //取输出的MAC地址

    log_debug("Wifi Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",roomfairy_WifiMac[0],roomfairy_WifiMac[1],roomfairy_WifiMac[2],roomfairy_WifiMac[3],roomfairy_WifiMac[4],roomfairy_WifiMac[5]);

	if(roomfairy_WifiMac[0]==0 &&roomfairy_WifiMac[1]==0&&roomfairy_WifiMac[2]==0&&roomfairy_WifiMac[3]==0 &&roomfairy_WifiMac[4]==0 &&roomfairy_WifiMac[5]==0)
	{
		log_err("ERROR: error Wifi Mac address\n");
		close(socketFd);
		return NETUTILS_FAILE;
	}

	close(socketFd);
#endif
	do
	{
		socketFd = socket(AF_INET, SOCK_DGRAM, 0);
		if(socketFd < 0)
		{
			 log_debug("ERROR: Can't create socket\n");
			 break;
		}

		memset(&req,0,sizeof(req));
	    strncpy(req.ifr_name,device,sizeof(req.ifr_name)-1); //将设备名作为输入参数传入
	    
	    if(ioctl(socketFd,SIOCGIFHWADDR,&req) < 0) //执行取MAC地址操作
	    {
			log_err("ERROR: Socket ioctl fail \n");
			break;
	    }
		
		bzero(resultMac,sizeof(resultMac));
		memcpy(resultMac,req.ifr_hwaddr.sa_data,ETH_ALEN); //取输出的MAC地址

		close(socketFd);
		return NETUTILS_SUCC;
		
	}while(0);

	if(socketFd) close(socketFd);

	return NETUTILS_FAILE;//NETUTILS_SUCC;
}



//获取指定ifname的IP地址
static int netUtils_getIfNameAddrs(char *ifname,char *host)
 {     
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	
	if (getifaddrs(&ifaddr) == -1) 
	{
	   log_err("getifaddrs error \n");
	   return -1;
	}

	memset(host,0,sizeof(host));

	/* Walk through linked list, maintaining head pointer so we
	  can free list later */

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
	   if (ifa->ifa_addr == NULL)
		   continue;

	   family = ifa->ifa_addr->sa_family;

	   /* Display interface name and family (including symbolic
		  form of the latter for the common families) */
/*
	   printf("%s  address family: %d%s\n",
			   ifa->ifa_name, family,
			   (family == AF_PACKET) ? " (AF_PACKET)" :
			   (family == AF_INET) ?   " (AF_INET)" :
			   (family == AF_INET6) ?  " (AF_INET6)" : "");
*/
	   /* For an AF_INET* interface address, display the address */
	   if(strcmp(ifa->ifa_name,ifname)!=0)
		   continue;
	   
	   if (family == AF_INET) 
	   {
		   s = getnameinfo(ifa->ifa_addr,(family == AF_INET) ? sizeof(struct sockaddr_in) :sizeof(struct sockaddr_in6),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				   
		   if (s != 0) 
		   {
				log_err("getnameinfo() failed: %s\n", gai_strerror(s));
				return -1;
		   }
		   
		   log_debug("address: <%s>\n", host);
	   }
	}

	freeifaddrs(ifaddr); 
	
	return 0;
 }
 

static int netUtils_readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)
{
    struct nlmsghdr *nlHdr;
    int readLen = 0, msgLen = 0;
    do{
         //收到内核的应答       
        if((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0)
        {
            log_err("SOCK READ: ");
            return -1;
        }
        
        nlHdr = (struct nlmsghdr *)bufPtr;
         //检查header是否有效
        if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))
        {
            log_err("Error in recieved packet");
            return -1;
        }
        
        if(nlHdr->nlmsg_type == NLMSG_DONE)
        {
            break;
        }
        else
        {
            bufPtr += readLen;
            msgLen += readLen;
        }
        
        if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)
        {
            break;
        }
    } while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));
    return msgLen;
}
 
 //分析返回的路由信息
static void netUtils_parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo,char *gateway, char *ifName)
{
    struct rtmsg *rtMsg;
    struct rtattr *rtAttr;
    int rtLen;
   
    struct in_addr dst;
    struct in_addr gate;
 
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);
    // If the route is not for AF_INET or does not belong to main routing table
    //then return.
    if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
        return;
 
    rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
    rtLen = RTM_PAYLOAD(nlHdr);
    for(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen)){
        switch(rtAttr->rta_type) {
        case RTA_OIF:
            if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
            break;
        case RTA_GATEWAY:
            rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);
            break;
        case RTA_PREFSRC:
            rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);
            break;
        case RTA_DST:
            rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);
            break;
        }
    }
    
    dst.s_addr = rtInfo->dstAddr;

    if (strstr((char *)inet_ntoa(dst), "0.0.0.0"))
    {
        sprintf(ifName, "%s", rtInfo->ifName);
        //printf("oif:%s",rtInfo->ifName);
        gate.s_addr = rtInfo->gateWay;
        sprintf(gateway, "%s", (char *)inet_ntoa(gate));
        //printf("%s\n",gateway);
        gate.s_addr = rtInfo->srcAddr;
        //printf("src:%s\n",(char *)inet_ntoa(gate));
        gate.s_addr = rtInfo->dstAddr;
        //printf("dst:%s\n",(char *)inet_ntoa(gate));
    }
   
    return;
}

//获取路由器的网关信息/判断是否连接到路由器
static int netUtils_getGateway(char *gateway, char *ifName)
{
    struct nlmsghdr *nlMsg;
    struct rtmsg *rtMsg;
    struct route_info *rtInfo;
    char msgBuf[BUFSIZE];
 
    int sock, len, msgSeq = 0;
 
    if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
    {
        log_err("Socket Creation: ");
        return -1;
    }
    
    memset(msgBuf, 0, BUFSIZE);
 
    nlMsg = (struct nlmsghdr *)msgBuf;
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);
 
    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
    nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .
 
    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
    nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.
    nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.
 
    if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0){
        log_err("Write To Socket Failed");
        close(sock);
        return -1;
    }
 
    if((len = netUtils_readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {
        log_err("Read From Socket Failed");
        close(sock);
        return -1;
    }
    
    rtInfo = (struct route_info *)malloc(sizeof(struct route_info));
    if(rtInfo == NULL)
    {
		log_err("malloc failed");
		close(sock);
		return -1;
    }

    for(;NLMSG_OK(nlMsg,len);nlMsg = NLMSG_NEXT(nlMsg,len)){
        memset(rtInfo, 0, sizeof(struct route_info));
        netUtils_parseRoutes(nlMsg, rtInfo, gateway, ifName);
    }

    free(rtInfo);
    close(sock);

    return 0;
}
 

//获取网关在当前局域网的IP地址
int netUtils_getHostAddrs(char *host)
{
	char buff[256], ifName[12];	   

	//获取网关路由
	if(netUtils_getGateway(buff, ifName))
	{
		log_err("get_gateway error\n");
		return NETUTILS_FAILE;
	}

	//获取主机的IP地址
	if(netUtils_getIfNameAddrs(ifName,host))
	{
		log_err("getInterfaceAddrs error\n");
		return NETUTILS_FAILE;
	}

	return NETUTILS_SUCC;
}


