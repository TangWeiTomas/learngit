/***********************************************************************************
 * �� �� ��   : hwSyslog.c
 * �� �� ��   : Edward
 * ��������   : 2016��11��30��
 * �ļ�����   : �����豸����������Ϣ����־������
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <curl.h>

#include "cJSON.h"
#include "logUtils.h"
#include "globalVal.h"
#include "mt_zbSocCmd.h"

#include "hwSyslog.h"


#define URL	"http://139.224.68.72:8080/FXMobiServer/gateway/addInfo"

static int hwSyslog_httpPost(char *str,uint16_t len)
{
	
	CURL *curl = NULL;
	CURLcode res ;
	struct curl_slist *headers=NULL;
	struct curl_slist *plist = NULL;

	ASSERT(str != NULL);
	
	curl_global_init(CURL_GLOBAL_ALL);
	
	curl = curl_easy_init();
	if(curl)
	{
	/* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. 
     */ 
     
    curl_easy_setopt(curl, CURLOPT_URL, URL);

	//�������ӳ�ʱ
	//curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L ); //����Ļ��ӡ�������ӹ��̺ͷ���http����
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, 10 );//��������ʱ��ʱ���ã����10��������δ�����ֱ꣬���˳�
	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1); // ����3��Ϊ�ض�������
	//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //���ص�ͷ������Location(һ��ֱ�������urlû�ҵ�)�����������Location��Ӧ������ 
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);//���Ҵ�������ֹ����̫��
	curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 3 );//���ӳ�ʱ�������ֵ�������̫�̿��ܵ����������󲻵��ͶϿ���

	//����http���͵���������ΪJSON
	plist = curl_slist_append(headers,"Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);

    /* Now specify the POST data  ����json����*/ 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str);
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  
  curl_global_cleanup();
  
  return (res!=CURLE_OK)?-1:0;
}


//{
//    "reportDate": "2012-12-30 12:30:21",
//    "mac": "213",
//    "ieee": "12323",
//    "zigbee": "123123",
//    "panId": "fff",
//    "action": "1w1w"
//}

char * hwSyslog_cJson(char *date,char *mac,char *ieee,uint16_t panid,uint8_t channel ,uint8_t action)
{
	cJSON *root = NULL;
	char *out = NULL;
	
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"reportDate",date);
	cJSON_AddStringToObject(root,"mac",mac);
	cJSON_AddStringToObject(root,"ieee",ieee);
	cJSON_AddNumberToObject(root,"zigbee",channel);
	cJSON_AddNumberToObject(root,"panId",panid);
	cJSON_AddNumberToObject(root,"action",action);

	out=cJSON_Print(root);	
	cJSON_Delete(root);	
	
	log_debug("cJSON: %s\n",out);
	return out;
}

void hwSyslog_cJSONfree(char *out)
{
	if(out != NULL) free(out);
}

int hwSyslog(uint8_t ieee[8],uint8_t action)
{
	ASSERT(ieee != NULL);

	char *logs = NULL;
	int ret = -1;
	uint8_t ieees[32]		={0};
	uint8_t rootMacs[20]	={0};
	uint16_t panid = 0;
	uint8_t channel = 0;
	
	log_debug("hwSyslog++\n");

	panid   = mt_zdo_getCoorPaind();
	channel = mt_zdo_getCoorChannel();
	
	if(panid == 0 || channel== 0)
	{

		log_debug("PANID= %x,channel=%x\n",panid,channel);
		//return ret;
	}
	
	sprintf(ieees,"%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",ieee[0],ieee[1],ieee[2],ieee[3],ieee[4],ieee[5],ieee[6],ieee[7]);	
												
	sprintf(rootMacs,"%02x:%02x:%02x:%02x:%02x:%02x",roomfairy_WifiMac[0],roomfairy_WifiMac[1],roomfairy_WifiMac[2],roomfairy_WifiMac[3],roomfairy_WifiMac[4],roomfairy_WifiMac[5]);
	
	//log_debug("Mac:%s  IEEE:%s\n",rootMacs,ieees);

	//����JSON
	logs = hwSyslog_cJson(getlocaltime(),rootMacs,ieees,panid,channel,action);	
						
	//��������
	ret = hwSyslog_httpPost(logs,strlen(logs));

	//�ͷ�
	hwSyslog_cJSONfree(logs);
	
	log_debug("hwSyslog--\n");
	return ret;
}



