/***********************************************************************************
 *      : config_operate.c
 *      : Edward
 *    : 20161110
 * ?   : /etc/config ??wirelessgateway?
 * ??   : Copyright (c) 2008-2016   xx xx xx xx ??
 *        : 
 * ??   : 
***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "logUtils.h"
#include "config_operate.h"
#include "wifi_uci.h"

//wireless
#define W_PACKAGE			"wireless"

#define W_SECTION_AP 		"ap"
#define W_SECTION_STA 		"sta"
#define W_SECTION_RADIO		"radio0"

#define W_OPTION_SSID 		"ssid"
#define W_OPTION_MODE 		"mode"
#define W_OPTION_ENCRY 		"encryption"
#define W_OPTION_KEY 		"key"
#define W_OPTION_DISABLE 	"disabled"
#define W_OPTION_CHANNEL	"channel"

//gateway
#define G_PACKAGE			"gateway"
//airkiss 
#define G_SECTION_AIRKISS	"airkiss"
#define G_OPTION_CONFIG		"config"
//server
#define G_SECTION_SERVER	"server"
#define G_OPTION_ADDR		"addr"
#define G_OPTION_PORT		"port"
//wifi 
#define G_SECTION_WIRLESS	"wifi"
#define G_OPTION_WIRELESS	"wireless"
//banding
#define G_SECTION_BIND	"binding"
#define G_OPTION_BIND	"bind"
//network
#define N_PACKAGE		"network"
#define N_SECTION_LAN   "lan"
#define N_OPTION_MAC	"macaddr"

int wireless_ap_disable(bool st)
{
	int ret = 0;
	if(st)
	{
		ret = wifi_uci_set(W_PACKAGE,W_SECTION_AP,W_OPTION_DISABLE,"1");
	}
	else
	{
		ret = wifi_uci_set(W_PACKAGE,W_SECTION_AP,W_OPTION_DISABLE,"0");
	}

	return ret;
}

int wireless_ap_setPasswd(char *encry,char *passwd)
{
	int ret = 0;
	if(passwd == NULL)
	{	
		ret = wifi_uci_set(W_PACKAGE,W_SECTION_AP,W_OPTION_ENCRY,"none");
		wifi_uci_del(W_PACKAGE,W_SECTION_AP,W_OPTION_KEY);
	}
	else
	{
		if(strlen(passwd) < 8)
			return -1;

		if(!strcmp(encry,"none"))
			return -1;
		
		ret = wifi_uci_set(W_PACKAGE,W_SECTION_AP,W_OPTION_ENCRY,encry);
		if(ret)
		{
			return -1;
		}
		
		ret = wifi_uci_set(W_PACKAGE,W_SECTION_AP,W_OPTION_KEY,passwd);
	}

	return ret;
}

int wireless_ap_setSSID(char *ssid)
{
	int ret = 0;

	ASSERT(ssid!=NULL);

	ret = wifi_uci_set(W_PACKAGE,W_SECTION_AP,W_OPTION_SSID,ssid);
	
	return ret;
}

int wireless_ap_setMode(char *mode)
{
	ASSERT(mode != NULL);

	return wifi_uci_set(W_PACKAGE,W_SECTION_AP,W_OPTION_MODE,mode);
}

//wifi station??
//wfInfo_t *wfInfo

void wireless_sta_config(char *ssid,char *encry,char *passwd,char *channel)
{
	
	char cmd[128] = "\0";
	
	if(isFileExist("/etc/config/wireless.ap-only"))//
	{
		snprintf(cmd,128,"cp /etc/config/wireless.ap-only /etc/config/wireless");
		system(cmd);
	}
	else
	{
		snprintf(cmd,128,"cp /etc/config/wireless /etc/config/wireless.ap-only");
		system(cmd);
	}

	snprintf(cmd,128,"uci set wireless.sta=wifi-iface\0");
	system(cmd);
	snprintf(cmd,128,"uci set wireless.sta.device=radio0\0");
	system(cmd);
	snprintf(cmd,128,"uci set wireless.sta.ifname=apcli0\0");
	system(cmd);
	snprintf(cmd,128,"uci set wireless.sta.network=wwan\0");
	system(cmd);
	snprintf(cmd,128,"uci set wireless.sta.mode=sta\0");
	system(cmd);
	snprintf(cmd,128,"uci set wireless.sta.ssid=%s\0",ssid);
	system(cmd);

	if(channel != NULL)
	{
		snprintf(cmd,128,"uci set wireless.radio0.channel=%s\0",channel);
		system(cmd);
	}
	
	if(encry !=NULL && passwd != NULL)
	{
		snprintf(cmd,128,"uci set wireless.sta.encryption=%s\0",encry);
		system(cmd);
		snprintf(cmd,128,"uci set wireless.sta.key=%s\0",passwd);
		system(cmd);
	}
	else
	{
		snprintf(cmd,128,"uci set wireless.sta.encryption=none");
		system(cmd);
	}

	snprintf(cmd,128,"uci commit wireless\0");
	system(cmd);
}

int wireless_radio_setChannel(char *ch)
{
	ASSERT(ch!=NULL);

	return wifi_uci_set(W_PACKAGE,W_SECTION_RADIO,W_OPTION_CHANNEL,ch);
}

int gateway_airkiss_set(bool st)
{
	int ret = 0;
	if(st)
	{
		ret = wifi_uci_set(G_PACKAGE,G_SECTION_AIRKISS,G_OPTION_CONFIG,"true");
	}
	else
	{
		ret = wifi_uci_set(G_PACKAGE,G_SECTION_AIRKISS,G_OPTION_CONFIG,"false");
	}

	return ret;
}

char* gateway_airkiss_get(void)
{
	return wifi_uci_get(G_PACKAGE,G_SECTION_AIRKISS,G_OPTION_CONFIG);
}

void gateway_airkiss_free(char *obj)
{
	ASSERT(obj!=NULL);
	free(obj);
}

char* gateway_server_getAddr(void)
{
	return wifi_uci_get(G_PACKAGE,G_SECTION_SERVER,G_OPTION_ADDR);
}

char* gateway_server_getPort(void)
{
	return wifi_uci_get(G_PACKAGE,G_SECTION_SERVER,G_OPTION_PORT);
}


int gateway_server_setAddr(char *addr)
{
	ASSERT(addr != NULL);
	
	return wifi_uci_set(G_PACKAGE,G_SECTION_SERVER,G_OPTION_ADDR,addr);
}

int gateway_server_setPort(char* port)
{
	ASSERT(port != NULL);
	return wifi_uci_set(G_PACKAGE,G_SECTION_SERVER,G_OPTION_PORT,port);
}

void gateway_server_free(char *obj)
{
	ASSERT(obj!=NULL);
	free(obj);
}

int gateway_server_config(char *addr,char *port)
{
	ASSERT(addr != NULL && port != NULL);
	
	int ret = 0;
	char *uciServer = NULL;
	char *uciPort = NULL;
	
	do
	{
		uciServer = gateway_server_getAddr();
		if(uciServer == NULL)
		{
		//	log_err("get server address failed\n");
			break;
		}
	
		uciPort = gateway_server_getPort();
		if(uciPort == NULL)
		{
		//	log_err("get server port failed\n");
			break;
		}
		
		ret = gateway_server_setAddr(addr);
		if(ret != 0)
		{
		//	log_err("set server addr failed\n");
			break;
		}
		ret = gateway_server_setPort(port);
		if(ret != 0)
		{
		//	log_err("set server port failed\n");
			break;
		}
		gateway_server_free(uciServer);
		gateway_server_free(uciPort);
		return 0;
	}while(0);
	
	//reback config
	gateway_server_setAddr(uciServer);
	gateway_server_setPort(uciPort);
	
	if(uciServer != NULL)
		gateway_server_free(uciServer);
	if(uciPort != NULL)
		gateway_server_free(uciPort);
	
	return -1;
}



int gateway_wireless_set(bool st)
{
	int ret = 0;
	if(st)
	{
		ret = wifi_uci_set(G_PACKAGE,G_SECTION_WIRLESS,G_OPTION_WIRELESS,"true");
	}
	else
	{
		ret = wifi_uci_set(G_PACKAGE,G_SECTION_WIRLESS,G_OPTION_WIRELESS,"false");
	}

	return ret;
}

char* gateway_wireless_get(void)
{
	return wifi_uci_get(G_PACKAGE,G_SECTION_WIRLESS,G_OPTION_WIRELESS);
}


void gateway_wireless_free(char *obj)
{
	ASSERT(obj!=NULL);
	free(obj);
}

//ж?
int gateway_Binding_set(bool st)
{
	int ret = 0;
	if(st)
	{
		ret = wifi_uci_set(G_PACKAGE,G_SECTION_BIND,G_OPTION_BIND,"true");
	}
	else
	{
		ret = wifi_uci_set(G_PACKAGE,G_SECTION_BIND,G_OPTION_BIND,"false");
	}

	return ret;
}

char* gateway_Binding_get(void)
{
	return wifi_uci_get(G_PACKAGE,G_SECTION_BIND,G_OPTION_BIND);
}


void gateway_Binding_free(char *obj)
{
	ASSERT(obj!=NULL);
	free(obj);
}

char* network_mac_get(void)
{
	return wifi_uci_get(N_PACKAGE,N_SECTION_LAN,N_OPTION_MAC);
}


void network_mac_free(char *obj)
{
	ASSERT(obj!=NULL);
	free(obj);
}


