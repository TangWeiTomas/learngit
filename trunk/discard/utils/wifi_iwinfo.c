#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "logUtils.h"
#include "iwinfo.h"
#include "wifi_iwinfo.h"

static char * wifi_formatEncryption(struct iwinfo_crypto_entry *c)
{
	static char buf[512];

	if (!c)
	{
		snprintf(buf, sizeof(buf), "unknown");
	}
	else if (c->enabled)
	{
		/* WEP */
		if (c->auth_algs && !c->wpa_version)
		{
			snprintf(buf, sizeof(buf), "wep-open");
		}

		/* WPA */
		else if (c->wpa_version)
		{
			switch (c->wpa_version) {
				case 3:
					snprintf(buf, sizeof(buf), "psk2");
					break;
				case 2:
					snprintf(buf, sizeof(buf), "psk2");
					break;
				case 1:
					snprintf(buf, sizeof(buf), "psk");
					break;
			}
		}
		else
		{
			snprintf(buf, sizeof(buf), "none");
		}
	}
	else
	{
		snprintf(buf, sizeof(buf), "none");
	}

	return buf;
}

static int wifi_iwinfoGetWifiBySSID(const struct iwinfo_ops *iw, wfInfo_t *wfInfo)
{
	ASSERT(iw!=NULL && wfInfo != NULL);

	int i, x, len;	
	char buf[IWINFO_BUFSIZE];	
	struct iwinfo_scanlist_entry *e;

	log_debug("wifi_iwinfoGetWifiBySSID++\n");

	if (iw->scanlist(wfInfo->device, buf, &len))	
	{		
		log_err("Scanning not possible\n\n");		
		return -1;	
	}	
	else if (len <= 0)	
	{
		log_err("No scan results\n\n");		
		return -1;	
	}
	
	for (i = 0, x = 1; i < len; i += sizeof(struct iwinfo_scanlist_entry), x++)	
	{		
		e = (struct iwinfo_scanlist_entry *) &buf[i];	

		if(strlen(wfInfo->ssid)!=strlen(e->ssid))
		{
			continue;
		}
		
		if(memcmp(e->ssid,wfInfo->ssid,strlen(e->ssid))==0)
		{
			if( e->channel <= 0)
			{
				return -1;
			}
			
			wfInfo->channel = e->channel;
			wfInfo->encry = strdup(wifi_formatEncryption(&e->crypto));

			if(memcmp("unknown",wfInfo->encry,strlen(wfInfo->encry))==0)
			{
				return -1;
			}

			log_debug("wifi_iwinfoGetWifiBySSID--1\n");
	
			return 0;
		}
	}
	
	log_debug("wifi_iwinfoGetWifiBySSID--2\n");
	
	return -1;
}

wfInfo_t *wifi_iwinfoScan(const char*device,const char*ssid,const char*passwd)
{
	ASSERT(device != NULL && ssid != NULL);
	
	const struct iwinfo_ops *iw;
	int ret = -1;
	wfInfo_t *wfInfo = NULL;
	log_debug("airkiss_iwinfoScanwifi++\n");

	do{
		wfInfo = malloc(sizeof(wfInfo_t));
		if(wfInfo == NULL)
		{
			log_err("malloc error\n");
			break;
		}		

		wfInfo->device = NULL;
		wfInfo->ssid = NULL;
		wfInfo->pwd = NULL;
		wfInfo->encry = NULL;
		wfInfo->channel = 0;

		wfInfo->device = strdup(device);
		wfInfo->ssid= strdup(ssid);
		wfInfo->pwd= strdup(passwd);
		
		iw = iwinfo_backend(wfInfo->device);
		if(!iw)
		{
			log_err("No such wireless device: %s\n", wfInfo->device);
			break;
		}
		
		ret = wifi_iwinfoGetWifiBySSID(iw, wfInfo);
		if(ret == -1)
		{
			log_err("wifi_iwinfoGetWifiBySSID error\n");
			break;
		}

		iwinfo_finish();

		return wfInfo;
	}while(0);

	if(!iw)
		iwinfo_finish();

	wifi_iwinfoDestroy(wfInfo);
	return NULL;
}


void wifi_iwinfoDestroy(wfInfo_t *wfInfo)
{
	ASSERT(wfInfo != NULL);
	log_debug("wifi_iwinfoDestroy++1\n");
	if(wfInfo->device != NULL) free(wfInfo->device);
	log_debug("wifi_iwinfoDestroy++2\n");
	if(wfInfo->ssid != NULL) free(wfInfo->ssid);
	log_debug("wifi_iwinfoDestroy++3\n");
	if(wfInfo->pwd != NULL) free(wfInfo->pwd);
	log_debug("wifi_iwinfoDestroy++4\n");
	if(wfInfo->encry != NULL) free(wfInfo->encry);
	log_debug("wifi_iwinfoDestroy++5\n");
	if(wfInfo) free(wfInfo);
	log_debug("wifi_iwinfoDestroy++6\n");
}

