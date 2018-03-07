#include "libsocketudp.h"
#include <stdio.h>
#include<string.h>
#include<unistd.h>


void showHex(char*buffer,size_t len)
{
	size_t i = 0;
	for(i=0;i<len;i++)
		printf("%x ",(buffer[i]&0xff));
	printf("\n");
}

void CreatData(char*buffer,size_t len)
{
	size_t i = 0;
	for(i=0;i<len;i++)
		buffer[i] = i;
}

int main(int argc,char**argv)
{

	char buffer[1024] = {0xFC,0x00 ,0x0B ,0x00 ,0x01, 0x20, 0x20, 0x20, 0x20, 0x20, 0x79, 0x01, 0xFF, 0x01, 0xAC};
	size_t len= 0 ;
	int size = 0;
	int port = 0;
	if(argc != 2)
	{
		printf("usage:%s <port>",argv[0]);
		return -1;
	}
	port = atoi(argv[1]);
	
	udp_t *udphandle = udp_new (port);

	if(udphandle == NULL)
		return -1;

	//CreatData(buffer,sizeof(buffer));
#define TEST 0
	while(1)
	{
	#if TEST
		memset(buffer,0,sizeof(buffer));
		len = udp_recv(udphandle,buffer,sizeof(buffer));
		if(len!=-1)
		{
			showHex(buffer,len);
	#endif
			udp_send(udphandle,buffer,15);
	#if TEST

		}	
	#endif
		sleep(1);
	}
	
	udp_destroy(&udphandle);
}
