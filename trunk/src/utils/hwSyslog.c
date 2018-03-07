/***********************************************************************************
 * 文 件 名   : hwSyslog.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年11月30日
 * 文件描述   : 用于设备掉线推送消息到日志服务器
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <curl.h>
#include <string.h>
#include <pthread.h>

#include "cJSON.h"
#include "logUtils.h"
#include "globalVal.h"
#include "mt_zbSocCmd.h"

#include "hwSyslog.h"

typedef struct
{
	uint16_t size;
	char *msg;
}hwSyslog_t;

//#define URL	"http://139.224.68.72:8080/FXMobiServer/gateway/addInfo"
#define URL "http://platform.feixuekj.cn/api/gateway/devicelog/add"

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
		
	/* 
		First set the URL that is about to receive our POST. This URL can
       	just as well be a https:// URL if that is what should receive the
       	data. 
    */ 
     
    curl_easy_setopt(curl, CURLOPT_URL, URL);

	//设置连接超时
	//curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L ); //在屏幕打印请求连接过程和返回http数据
	curl_easy_setopt( curl, CURLOPT_TIMEOUT, 10 );//接收数据时超时设置，如果10秒内数据未接收完，直接退出
	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1); // 以下3个为重定向设置
	//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //返回的头部中有Location(一般直接请求的url没找到)，则继续请求Location对应的数据 
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);//查找次数，防止查找太深
	curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, 3 );//连接超时，这个数值如果设置太短可能导致数据请求不到就断开了

	//设置http发送的数据类型为JSON
	plist = curl_slist_append(headers,"Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);

    /* Now specify the POST data  发送json数据*/ 
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
//    "action": "0/1"
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


void *hwSyslog_pthread(void *args)
{
	int ret = 0;
	hwSyslog_t *logs = (hwSyslog_t*)args;
	log_debug("hwSyslog_pthread++\n");

	//发送数据
	ret = hwSyslog_httpPost(logs->msg,logs->size);
	
	if(ret != 0)
		log_debug("hwSyslog_httpPost failed\n");

	//释放内存
	hwSyslog_cJSONfree(logs->msg);
	free(logs);
	
	log_debug("hwSyslog_pthread--\n");
	pthread_detach(pthread_self());
}

int hwSyslog(uint8_t ieee[8],uint8_t action)
{
	ASSERT(ieee != NULL);

	//char *logs = NULL;
	hwSyslog_t *logs ;
	pthread_t thread;
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

	do
	{
		logs = (hwSyslog_t*)malloc(sizeof(hwSyslog_t));

		if(logs == NULL)
		{
			log_debug("malloc failed\n");
			break;
		}
		
		//生成JSON
		logs->msg = hwSyslog_cJson(getlocaltime(),rootMacs,ieees,panid,channel,action);	
		logs->size = strlen(logs->msg);
		
		ret = pthread_create(&thread,NULL,hwSyslog_pthread,logs);
		if(ret != 0)
		{
			log_debug("pthread_create failed\n");
			hwSyslog_cJSONfree(logs->msg);
			free(logs);
			break;
		}
	}while(0);

	//释放
	//hwSyslog_cJSONfree(logs->msg);
	
	log_debug("hwSyslog--\n");
	return ret;
}



