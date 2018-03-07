#include "PermissionManage.h"

#include <event2/event.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Tcp_client.h"

#include "globalVal.h"

#if PERMIMNG 

typedef struct PermiMng_s{
	uint8_t PermiType;		//授权类型
	time_t endSecondsData;	//结束的秒数
}PermiMng_t;


typedef enum{
	MOUNTH=	0X00,
	YEAR=	0X01,
	FOREVER=0X02
}PermiMng_mode;

//权限检测
bool g_PermiMngisPass = false;

static PermiMng_t permisstion;
static tcp_connection_t *PermiConnect = NULL;


//权限请求检测间隔时间
uint64_t g_PermiMngRequestInterval = PermiMngRequestTime;

bool g_PermiMngRequestScuess = false;

int PermiMng_rand_str(char str[],uint16_t number)
{
	char strs[]="00123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int i;
	char ss[2];
	//printf("%c %c\n",str[1],str[62]);
	srand((unsigned int)time((time_t *)NULL));
	for(i=1;i<=number;i++)
	{
		sprintf(ss,"%c",str[(rand()%62)+1]);
		strcat(str,ss);
	}
}

//配置数据校验
uint8_t PermiMng_SetFCS(uint8_t *buf,uint8_t bufsize,uint8_t datalen)
{
    uint8_t fcs=0;
    
	if((datalen+1) < bufsize)
	{
		uint16_t cnt;
        for(cnt=0; cnt<datalen; cnt++)
        {
            fcs ^= buf[cnt];
        }

        buf[datalen] = fcs;
        
        return true;
	}

    return false;
}
//配置数据校验
bool PermiMng_checkPacketFCS(uint8_t* data,uint16_t length)
{
    uint16_t cnt;
    uint8_t result=0;
    //先查找包头
    for(cnt=0; cnt<length; cnt++)
    {
        result ^= data[cnt];
    }

    //校验结果为0，则表明校验正确
    if(result==0)
    {
        return true;
    }

    return false;
}

//加密解密程序  
void  PermiMng_encrypt(uint8_t* message, const char *key)  
{  
    int i;  
    int len = strlen(key);  
    log_debug("Key:%d\n",len);

	int msgsize = strlen(message);
	int msgcnt = 0;
	for(msgcnt = 0;msgcnt<msgsize;msgcnt++)
	{
//		log_debug("%x\n",message[msgcnt]);
		 for(i = 0; i < len; i++) {  
            message[msgcnt] = message[msgcnt] ^ (int)key[i];  
        }  
//        log_debug("--%x\n",message[msgcnt]);
	}
    
//    while(*message) {  
//    	log_debug("%x\n",*message);
//        //对message的每一个字符和key进行按位异或  
//        for(i = 0; i < len; i++) {  
//            *message = (*message) ^ (int)key[i];  
//        }  
//        message++;  
//        log_debug("--%x\n",*message);
//    }  
}

//从配置文件中读取权限
bool PermiMng_Config(void)
{
	int fd = -1;
	int i = 0;
	time_t timep;
	int bufsize = 0;
	uint8_t encryptbuf[PERMI_BUF_SIZE] = {0};
	
	if (isFileExist(PermiMngFilePath))//isexist
	{
		if(-1 == readBinaryFile(PermiMngFilePath,encryptbuf,PERMI_BUF_SIZE))
			return false;

		//解密
		PermiMng_encrypt(encryptbuf,PermiMngKey);
		
		bufsize = strlen(encryptbuf);
			
		if(!PermiMng_checkPacketFCS(encryptbuf,bufsize))
		{
			
			log_err("PermiMng_checkPacketFCS failed\n");
			return false;
		}
		log_debug("%s\n",encryptbuf);
		bufsize = strlen(encryptbuf);
		log_debug("%d\n",bufsize);
		encryptbuf[bufsize-1] = '\0';
		log_debug("%s\n",encryptbuf);
		permisstion.PermiType = encryptbuf[0]-'0';
		
		permisstion.endSecondsData =(time_t)strtoul(&encryptbuf[1],NULL,10);
		
		log_debug("%ld\n",permisstion.endSecondsData);
		log_debug("permisstion.PermiType:%d\n",permisstion.PermiType);
		
		//年月授权方式
		if(permisstion.PermiType == MOUNTH || permisstion.PermiType == YEAR)
		{
			time(&timep);
			
			log_debug("current:%ld,overdue:%ld\n",timep,permisstion.endSecondsData);
			
			if (timep > permisstion.endSecondsData)//权限过期
			{
				log_debug("g_PermiMngisPass=false");
				g_PermiMngisPass = false;
				return false;
			}
			else
			{
				log_debug("g_PermiMngisPass=true");
				g_PermiMngisPass = true;
			}
		}
		
		//永久权限检测
		else if(permisstion.PermiType == FOREVER)
		{
			g_PermiMngisPass = true;
		}
		//其他
		else
		{
			g_PermiMngisPass = false;
			return false;
		}
		
		return true;
	}

	g_PermiMngisPass = true;

	return false;
}


void PermiMng_WriteConfig(uint8_t permiType,time_t permiData)
{
	time_t timep;
	int i=0;
	uint8_t encryptbuf[PERMI_BUF_SIZE] = {0};
	
	//encryptbuf[0] = permiType;
	sprintf(encryptbuf,"%d%ld",permiType,permiData);
	
	PermiMng_SetFCS(encryptbuf,PERMI_BUF_SIZE,strlen(encryptbuf));

	PermiMng_encrypt(encryptbuf,PermiMngKey);

	if(-1==writeBinaryFile(PermiMngFilePath,encryptbuf,strlen(encryptbuf)))
		log_debug("writeBinaryFile ERROR\n");
}

int PermiMng_SendMsg(uint8_t* cmdMsg,uint16_t cmdMsgLen)
{
	ASSERT(cmdMsg != NULL);
	
	if(PermiConnect == NULL)
		return -1;
		
	if(PermiConnect->connected == true)
		bufferevent_write(PermiConnect->bevent,cmdMsg,cmdMsgLen);
}

//获取请求间隔
uint64_t PermiMng_getRequestIntr(void)
{
	uint64_t times = 0;
	//未请求成功
	if(g_PermiMngRequestScuess == false)
	{
		log_debug("g_PermiMngRequestScuess == false\n");
		times = PermiMngRequestSendTime;
	}
	else
	{
		//请求成功，权限通过
		if(g_PermiMngisPass == true)
		{
			log_debug("g_PermiMngRequestScuess = True && g_PermiMngisPass == true\n");
			times = g_PermiMngRequestInterval;
		}
		else //请求成功，权限检测未通过
		{
			log_debug("g_PermiMngRequestScuess = True && g_PermiMngisPass == false\n");
			times = PermiMngRequestFaileTime;
		}
	}
	return times;
}

int PermiMng_close(void)
{
	log_debug("PermiMng_close++\n");

	uint64_t times = 0;
	
	if(PermiConnect == NULL)
		return -1;

	tcp_connection_t *connect = PermiConnect;
	if(connect->connected == true)
	{
		log_debug("connect->connected == true\n");
		if(connect->bevent != NULL)
		{
			log_debug("connect->bevent != NULL\n");
			bufferevent_free(connect->bevent);
			connect->bevent = NULL;
			connect->connected = false;
		}
	}
	
	PermiMng_StartTimer(PermiMng_getRequestIntr());
	
	log_debug("PermiMng_close--\n");
}

int PermiMng_StartTimer(uint64_t times)
{
	int ret = -1;
	if(PermiConnect == NULL)
		return -1;

	log_debug("PermiMng_StartTimer++\n");
	
	ret = tu_set_evtimer(&PermiConnect->evtimer,times,ONCE,start_tcp_permimng_client_failed_cb,PermiConnect);

	if(ret == -1)
		return -1;
		
	log_debug("PermiMng_StartTimer=--\n");

	return ret;
}

void PermiMng_Init(struct event_base *base)
{
	ASSERT(base != NULL);

	uint64_t times = 0;
	
	log_debug("PermiMng_Init++\n");

	if(!PermiMng_Config())
	{
		g_PermiMngRequestScuess = false;
	}
	else
	{
		g_PermiMngRequestScuess = true;
	}
	
	PermiConnect = tcp_permimng_client_evInit(base);

	if(PermiMng_StartTimer(PermiMng_getRequestIntr()) < 0)
		log_err("PermiMng_StartTimer failed\n");

	log_debug("PermiMng_Init--\n");
}

#endif
