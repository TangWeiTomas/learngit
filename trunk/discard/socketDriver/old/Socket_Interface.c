#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <poll.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <stdbool.h>

#include "comParse.h"
#include "Tcp_client.h"

#include "logUtils.h"
#include "Socket_Interface.h"
#include "globalVal.h"
//#include "logUtils.h"
#include "Polling.h"
#include "interface_srpcserver.h"
#include "Timer_utils.h"
#include "config_operate.h"


#include <arpa/inet.h>  //for in_addr
#include <linux/rtnetlink.h>    //for rtnetlink
#include <net/if.h> //for IF_NAMESIZ, route_info
#include <stdio.h>
#include <stdlib.h> //for malloc(), free()
#include <string.h> //for strstr(), memset()
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/

 #if 0
 
#define BUFSIZE 8192

struct route_info
{
	u_int dstAddr;
	u_int srcAddr;
	u_int gateWay;
	char ifName[IF_NAMESIZE];
};

#endif
/*****************************************************************************
 * 函 数 名  : SocketInterface_GetMacAddr
 * 负 责 人  : Edward
 * 创建日期  : 2016年5月1日
 * 函数功能  : 获取本机的MAC地址
 * 输入参数  : void  无
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

*****************************************************************************/
bool SocketInterface_GetMacAddr(void)
{
//    bool rtn = fsalse;
#if 0		
#ifdef OPENWRT_TEST
//	char *device = "wlan0";
	char *device = "br-lan";
#else
	char *device="eth0"; //eth0是网卡设备名
#endif
    
    struct ifreq req;
    int socketFd;

	if((socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP))<0)
    {
        log_debug("ERROR: Can't create socket\n");
        return false;
    }
	
	memset(&req,0,sizeof(req));
    strncpy(req.ifr_name,device,sizeof(req.ifr_name)-1); //将设备名作为输入参数传入
    
    if(ioctl(socketFd,SIOCGIFHWADDR,&req)<0) //执行取MAC地址操作
    {
        log_err("ERROR: Socket ioctl fail \n");
        close(socketFd);
        return false;
    }

	bzero(roomfairy_WifiMac,sizeof(roomfairy_WifiMac));
    memcpy(roomfairy_WifiMac,req.ifr_hwaddr.sa_data,ETH_ALEN); //取输出的MAC地址

    log_debug("Wifi Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",roomfairy_WifiMac[0],roomfairy_WifiMac[1],roomfairy_WifiMac[2],roomfairy_WifiMac[3],roomfairy_WifiMac[4],roomfairy_WifiMac[5]);

	if(roomfairy_WifiMac[0]==0 &&roomfairy_WifiMac[1]==0&&roomfairy_WifiMac[2]==0&&roomfairy_WifiMac[3]==0 &&roomfairy_WifiMac[4]==0 &&roomfairy_WifiMac[5]==0)
	{
		log_err("ERROR: error Wifi Mac address\n");
		close(socketFd);
		return false;
	}

	close(socketFd);

	return true;
	
#else

	char *mac = NULL;
	char *delim=":";
	char *p;
	int cnt = 0;
	
	mac = network_mac_get();	
	if(mac == NULL)
	{
		return false;
	}

	p = strtok(mac,delim);
	while(p != NULL)
	{
		if(cnt > 6)
		{
			network_mac_free(mac);
			return false;
		}
		
		roomfairy_WifiMac[cnt++]=strtol(p,NULL,16);
		p = strtok(NULL,delim);
	}
	
	log_debug("Wifi Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",roomfairy_WifiMac[0],roomfairy_WifiMac[1],roomfairy_WifiMac[2],roomfairy_WifiMac[3],roomfairy_WifiMac[4],roomfairy_WifiMac[5]);

	network_mac_free(mac);
	return true;

#endif
}

#if 0
static int getInterfaceAddrs(char *ifname,char *host)
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
 

static int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)
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
static void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo,char *gateway, char *ifName)
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



static int get_gateway(char *gateway, char *ifName)
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
 
    if((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {
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
        parseRoutes(nlMsg, rtInfo, gateway, ifName);
    }

    free(rtInfo);
    close(sock);

    return 0;
}
 

bool SocketInterface_Getgateway(char *host)
{
	char buff[256], ifName[12];	   
	//获取网关路由
	if(get_gateway(buff, ifName))
	{
		log_err("get_gateway error");
		return false;
	}

	//获取主机的IP地址
	if(getInterfaceAddrs(ifName,host))
	{
		log_err("getInterfaceAddrs error");
		return false;
	}

	return true;
}
#endif 

