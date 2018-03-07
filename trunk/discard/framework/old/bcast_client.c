/*********************************************************************
 * Filename: bcast_client.c
 * Description:广播客户端代码
 * Author: Cheney.Xu
 * Date: 2014-5-22
 ********************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "bcast_client.h"

#define IP_FOUND_ACK "IP_FOUND_ACK"
#ifdef OPENWRT_TEST
#define IFNAME "wlan0"
#else
#define IFNAME "eth0"
#endif
#define MCAST_PORT 9999

unsigned char MAC_ADDR[12];

char IP_FOUND[50]="IP_FOUND";
char ip[32] = {0};



void read_mac(void)
{
    int sockfd;
    struct ifreq ifreq;

    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket");
        exit(0);
    }

    strcpy(ifreq.ifr_name,IFNAME);

    if(ioctl(sockfd,SIOCGIFHWADDR,&ifreq)<0)
    {
        perror("ioctl");
        exit(0);
    }

    sprintf(MAC_ADDR,"%02x%02x%02x%02x%02x%02x",
            (unsigned char)ifreq.ifr_hwaddr.sa_data[0],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[2],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[3],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char)ifreq.ifr_hwaddr.sa_data[5]);
    printf("MAC_ADDR=%s\n",MAC_ADDR);

    close(sockfd);
}


void* acceptThreadFun_bcast(void *arg)
{
    int ret = -1;
    int sock = -1;
    int j = -1;
    int so_broadcast = 1;
    struct ifreq *ifr;
    struct ifconf ifc;
    struct sockaddr_in broadcast_addr; //广播地址
    struct sockaddr_in from_addr; //服务端地址
    int from_len = sizeof(from_addr);
    int count = -1;
    fd_set readfd; //读文件描述符集合
    char buffer[1024];
    struct timeval timeout;
    timeout.tv_sec = 3; //超时时间3秒
    timeout.tv_usec = 0;

    printf("read_mac.\n");

    read_mac();
    strcat(IP_FOUND, MAC_ADDR);

    printf("acceptThreadFun_bcast.\n");

    //建立数据包套接字
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("create socket failed:");
        //  return -1;
    }

    ifc.ifc_len = sizeof(buffer);
    ifc.ifc_buf = buffer;
    if (ioctl(sock, SIOCGIFCONF, (char *) &ifc) < 0)
    {
        perror("ioctl-conf:");
        //  return -1;
    }

    ifr = ifc.ifc_req;
    for (j = ifc.ifc_len / sizeof(struct ifreq); --j >= 0; ifr++)
    {
        if (!strcmp(ifr->ifr_name, IFNAME))
        {
            if (ioctl(sock, SIOCGIFFLAGS, (char *) ifr) < 0)
            {
                perror("ioctl-get flag failed:");
            }
            break;
        }
    }

    //get the IP of this interface
    if (!ioctl(sock, SIOCGIFADDR, (char *)ifr))
    {
        snprintf(ip, sizeof(ip), "%s",(char *)inet_ntoa(((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr));
        printf("device local ip: %s\n", ip);

        unsigned long mast=inet_addr("0.0.0.255");

        (((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr).s_addr=(((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr).s_addr|mast;

        snprintf(ip, sizeof(ip), "%s",(char *)inet_ntoa(((struct sockaddr_in *)&(ifr->ifr_addr))->sin_addr));

        printf("device mast ip: %s\n", ip);
    }

    memcpy(&broadcast_addr, (char *)&ifr->ifr_addr, sizeof(struct sockaddr_in));
    //设置广播端口号
    printf("\nBroadcast-IP: %s\n", inet_ntoa(broadcast_addr.sin_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(MCAST_PORT);

    //默认的套接字描述符sock是不支持广播，必须设置套接字描述符以支持广播
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));

    //发送多次广播，看网络上是否有服务器存在
    while(1)
    {
        //一共发送10次广播，每次等待2秒是否有回应
        //广播发送服务器地址请求
        timeout.tv_sec = 2;  //
        timeout.tv_usec = 0;

        printf("udp go on\n");

        sleep(2);

        ret = sendto(sock, IP_FOUND, strlen(IP_FOUND), 0, (struct sockaddr*) &broadcast_addr, sizeof(broadcast_addr));
        if (ret < 0)
        {
            continue;
        }

        //文件描述符清0
        FD_ZERO(&readfd);
        //将套接字文件描述符加入到文件描述符集合中
        FD_SET(sock, &readfd);
        //select侦听是否有数据到来
        ret = select(sock + 1, &readfd, NULL, NULL, &timeout);

        printf("\t ret=%d \n", ret);

        switch (ret)
        {
            case -1:
                break;
            case 0:
                //perror("select timeout\n");
                break;
            default:
                //接收到数据
                if (FD_ISSET(sock,&readfd))
                {
                    count = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*) &from_addr, &from_len); //from_addr为服务器端地址
                    printf("\trecvmsg is %s\n", buffer);
                    if (strstr(buffer, IP_FOUND_ACK))
                    {
                        printf("\tfound server IP is %s, Port is %d\n",inet_ntoa(from_addr.sin_addr),htons(from_addr.sin_port));
                    }
                }
                break;
        }
    }
    //  return 0;
}


void startBcast_Thread(void)
{
    pthread_t bcast;

    printf("startBcast_Thread.\n");
    //创建线程来帮忙accept
    int err=pthread_create(&bcast,NULL,acceptThreadFun_bcast,NULL);
    if(err!=0)
    {
        printf("can not create thread \n");
    }

}



