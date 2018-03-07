/**************************************************************************
 * Filename:       sim800l.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    SIM800L模块 GPRS驱动.
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/


/*********************************************************************
* INCLUDES
*/
#include "sim800l.h"
#include "uart.h"
#include "logUtils.h"
#include "globalVal.h"
#include "gpio.h"
/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/
#define SIM800L_CMD_RETRY_MAX	5


#define SIM800L_GPIO_RST	39
#define SIM800L_GPIO_DTR	2

#define POWER_ON	1
#define POWER_OFF	0


/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/

uint8_t sim800lMode = 0;

/*********************************************************************
* LOCAL VARIABLES
*/

static int uartfd = -1;


/*********************************************************************
* LOCAL FUNCTIONS
*/
int sim800l_SendCmd(char *cmd,char*ack,uint16_t waittime);

/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* @fn		   sim800lSendATCommand
*
* @brief	   发送命令
*
* @param	   void
*
* @return	   void
*/

uint8_t* sim800l_CheckAckCmd(uint8_t *ack)
{
	char * stridx = NULL;
	
	stridx = strstr(gUartRxfifo.fifo,ack);
		
	return stridx;
}
/*********************************************************************
* @fn          sim800l_CheckMode
*
* @brief       检测模块是否与MCU进行连接
*
* @param       void
*
* @return      0:成功 -1:失败
*/

int sim800l_CheckMode(void)
{
	int ret = 0;
	uint8_t retryCnt = 0;
	do{
		ret = sim800l_SendCmd("AT\r\n","OK",20);
		msleep(1000);
		if((retryCnt++ > SIM800L_CMD_RETRY_MAX)||(ret < 0))
		{
			ret = -1;
			break;
		}
	}while(ret);

	return ret;
}

/*********************************************************************
* @fn          sim800l_CheckSimCard
*
* @brief       检测sim卡的是否正确的接入
*
* @param       void
*
* @return      0:成功 -1:失败
*/

int sim800l_CheckSimCard(void)
{
	int ret = 0;
	uint8_t retryCnt = 0;
	do{
		ret = sim800l_SendCmd("AT+CPIN?\r\n","READY",20);
		msleep(1000);
		if((retryCnt++ > SIM800L_CMD_RETRY_MAX)||(ret < 0))
		{
			ret = -1;
			break;
		}
	}while(ret);

	return ret;
}

/*********************************************************************
* @fn          sim800l_CheckSimCard
*
* @brief       检测sim卡是否被注册到网络中
*
* @param       void
*
* @return      0:成功 -1:失败
*/
int sim800l_CheckNwkRegStatus(void)
{
	int ret = -1;
	uint8_t retryCnt = 0;
	char *pStr = NULL;
	do{
		sim800l_SendCmd("AT+CREG?\r\n","OK",500);
		msleep(1000);
		if(retryCnt++ > SIM800L_CMD_RETRY_MAX)
		{
			ret = -1;
			break;
		}

		pStr = strstr(gUartRxfifo.fifo,"+CREG:");

		printf("pStr = %s %d\n",pStr,pStr[9]);
		
		if((pStr != 0)&&(strlen(pStr) > 9)&&(pStr[9]=='1'||pStr[9]=='5'))
		{
			ret = 0;
			break;
		}
		
		
	}while(ret);

	return ret;
}

/*********************************************************************
* @fn          sim800l_CheckSimCard
*
* @brief       检测sim卡GPRS是否可以使用
*
* @param       void
*
* @return      0:成功 -1:失败
*/
int sim800l_CheckGprsRegStatus(void)
{
	int ret = -1;
	uint8_t retryCnt = 0;
	char *pStr = NULL;
	do{
		sim800l_SendCmd("AT+CGREG?\r\n","OK",500);
		msleep(1000);
		if(retryCnt++ > SIM800L_CMD_RETRY_MAX)
		{
			ret = -1;
			break;
		}

		pStr = strstr(gUartRxfifo.fifo,"+CGREG:");
		
		if((pStr != 0)&&(strlen(pStr) > 10)&&(pStr[10]=='1'||pStr[10]=='5'))
		{
			ret = 0;
			break;
		}
		else
		{
			sim800l_SendCmd("AT+CIPSHUT\r\n","SHUT OK",20);
		}
	}while(ret);

	return ret;
}


/*********************************************************************
* @fn          sim800l_Setecho
*
* @brief       设置命令回显
*
* @param       status- true:设置回显 false:取消回显
*
* @return      0:成功 -1:失败
*/
int sim800l_Setecho(bool status)
{
	int ret = 0;
	uint8_t retryCnt = 0;
	do{
		if(status)	
			ret = sim800l_SendCmd("ATE1\r\n","OK",20);
		else
			ret = sim800l_SendCmd("ATE0\r\n","OK",20);
		
		msleep(1000);
		if((retryCnt++ > SIM800L_CMD_RETRY_MAX)||(ret < 0))
		{
			ret = -1;
			break;
		}
	}while(ret);

	return ret;
}


/*********************************************************************
* @fn          sim800l_HwStart
*
* @brief       SIM800L硬件启动
*
* @param       void
*
* @return      0:成功 -1:失败
*/
int sim800l_HwStart(void)
{
	int ret = 0;
	if((gpio_export(SIM800L_GPIO_RST) == GPIO_SUCCESS)
		&&(gpio_set_direction(SIM800L_GPIO_RST, GPIO_OUTPUT)==GPIO_SUCCESS))
	{
		ret = gpio_write(SIM800L_GPIO_RST, POWER_ON);
	}

	return (ret > 0?0:-1);	
}

/*********************************************************************
* @fn          sim800l_HwStop
*
* @brief       SIM800L硬件关闭
*
* @param       void
*
* @return      0:成功 -1:失败
*/

int sim800l_HwStop(void)
{
	int ret = 0;
	if((gpio_export(SIM800L_GPIO_RST) == GPIO_SUCCESS)
		&&(gpio_set_direction(SIM800L_GPIO_RST, GPIO_OUTPUT)==GPIO_SUCCESS))
	{
		ret = gpio_write(SIM800L_GPIO_RST, POWER_OFF);
	}

	return (ret > 0?0:-1);	
}

/*********************************************************************
* @fn          sim800l_ExitNet
*
* @brief       SIM800L唤醒，只有在AT+CSCLK=1才生效
*
* @param       void
*
* @return      0:成功 -1:失败
*/

int sim800l_ExitNet(void)
{
	int ret = 0;
	int waittime = 200;
	if((gpio_export(SIM800L_GPIO_DTR) == GPIO_SUCCESS)
		&&(gpio_set_direction(SIM800L_GPIO_DTR, GPIO_OUTPUT)==GPIO_SUCCESS))
	{
		gpio_write(SIM800L_GPIO_DTR, POWER_OFF);
		sleep(2);
		gpio_write(SIM800L_GPIO_DTR, POWER_ON);
	}

	ret = gpio_read(SIM800L_GPIO_DTR);

	while(--waittime)
	{
		msleep(10);
		if(sim800l_CheckAckCmd("OK"))
		{
			sim800lMode = SIM800L_MODE_AT;
			break;
		}
	}
	
	if(waittime == 0)
		ret = -1;

	return ret==0?0:-1;	
}

/*********************************************************************
* @fn          sim800l_Reset
*
* @brief       重启sim800l模块
*
* @param       void
*
* @return      0:成功 -1:失败
*/
int sim800l_Reset(void)
{
	int ret = 0;
	sim800l_HwStop();
	msleep(500);
	sim800l_HwStart();
}

/*********************************************************************
* @fn          sim800l_HwStop
*
* @brief       SIM800L硬件关闭
*
* @param       void
*
* @return      0:连接关闭 1:连接正常 -1:失败
*/

int sim800l_SocketStatus(void)
{
	int ret = -1;
	uint8_t retryCnt = 0;
	char *pStr = NULL;
	do{
		sim800l_SendCmd("AT+CIPSTATUS\r\n","OK",20);
		msleep(1000);
		if(retryCnt++ > SIM800L_CMD_RETRY_MAX)
		{
			ret = -1;
			break;
		}

		if(strstr(gUartRxfifo.fifo,"CLOSED") || strstr(gUartRxfifo.fifo,"ERROR"))
		{
			ret = 0;
			break;
		}
		
		if(strstr(gUartRxfifo.fifo,"CONNECT OK"))
		{
			ret = 1;
			break;
		}
		
	}while(ret);

	return ret;
}

/*********************************************************************
* @fn          sim800l_SocketConnect
*
* @brief       GPRS网络连接
*
* @param       addr - 域名
*			   port - 端口号
*
* @return      0:连接成功 -1:连接失败
*/

int sim800l_SocketConnect(char *addr,char *port)
{
	int ret = 0;
	char url[128] = {0};

	//退出数据透传模式
	sim800l_ExitNet();

	ret = sim800l_SendCmd("AT+CIPCLOSE=1\r\n","CLOSE OK",20); //关闭连接
	msleep(100);
	ret = sim800l_SendCmd("AT+CIPSHUT\r\n","SHUT OK",500);			
	if(sim800l_SendCmd("AT+CGCLASS=\"B\"\r\n","OK",1000))return -1;				//设置GPRS移动台类别为B,支持包交换和数据交换 
	if(sim800l_SendCmd("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n","OK",1000))return -1;	//设置PDP上下文,互联网接协议,接入点等信息
	if(sim800l_SendCmd("AT+CGATT=1\r\n","OK",500))return -1;						//附着GPRS业务
	if(sim800l_SendCmd("AT+CIPCSGP=1,\"CMNET\"\r\n","OK",500))return -1;	 		//设置为GPRS连接模式
	//if(sim800l_SendCmd("AT+CIPHEAD=1\r\n","OK",500))return -1;	 				//设置接收数据显示IP头(方便判断数据来源)
	if(sim800l_SendCmd("AT+CIPMUX=0\r\n","OK",500))return -1;
	if(sim800l_SendCmd("AT+CIPMODE=1\r\n","OK",500))return -1;

	sprintf(url,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",addr,port);
	if(!sim800l_SendCmd(url,"OK",500))
	{
		fxUartRxfifoClear();
		sim800lMode = SIM800L_MODE_NET;
		return 0;
	}

	sim800lMode = SIM800L_MODE_AT;
	return -1;	
}

/*********************************************************************
* @fn          sim800l_SocketClose
*
* @brief       关闭TCP连接
*
* @param       void
*
* @return      0:成功 -1:失败
*/

int sim800l_SocketClose(void)
{
	sim800l_ExitNet();
	
	sim800l_SendCmd("AT+CIPCLOSE=1\r\n","CLOSE OK",20); //关闭连接
	return sim800l_SendCmd("AT+CIPSHUT\r\n","SHUT OK",50);	
}

/*********************************************************************
* @fn		   sim800lSendATCommand
*
* @brief	   发送命令
*
* @param	   void
*
* @return	   void
*/
int sim800l_SendCmd(char *cmd,char*ack,uint16_t waittime)
{
	int ret = 0;
	bool isFind = false;
	if(cmd == NULL)
		return -1;
	
	fxUartRxfifoClear();
#if 0
	do{
		fxUartSend(uartfd,cmd,strlen(cmd));

		if(ack&&waittime)
		{
			while(--waittime)
			{
				msleep(100);
				if(sim800lCheckAckCmd(ack))
				{
					isFind = true;
					break;
				}
			}
			
			if(waittime == 0)
				ret = -1;
		}
		
	}while(ack&&waittime&&(!isFind));
#else
	if(uartfd != -1)
		fxUartSend(uartfd,cmd,strlen(cmd));
	if(ack&&waittime)
	{
		while(--waittime)
		{
			msleep(10);
			if(sim800l_CheckAckCmd(ack))
			{
				break;
			}
		}
		
		if(waittime == 0)
			ret = -1;
	}

#endif
	return ret;
}

/*********************************************************************
* @fn          sim800l_SocketConnect
*
* @brief       GPRS网络连接
*
* @param       addr - 域名
*			   port - 端口号
*
* @return      0:发送成功 -1:发送失败
*/

int sim800l_SendData(uint8_t *buf,uint16_t size)
{
	int ret = 0;
	char ctrlz[2] = {0x1a,'\0'};
	log_debug("sim800l_SendData++\n");
	//sim800l_SendCmd(ctrlz,"SEND OK",50);

	
//	if(sim800l_SendCmd("AT+CIPSEND\r\n",">",500))
//		return -1;
	
	if(uartfd != -1);
		fxUartSend(uartfd, buf, size);

//	ret = sim800l_SendCmd(ctrlz,"SEND OK",500);
	//usleep(1000);
	log_debug("sim800l_SendData--\n");
	return ret;
}

/*********************************************************************
* @fn          sim800l_HwDetection
*
* @brief       sim800L检测
*
* @param       void
*
* @return      0:成功 -1:失败
*/
int sim800l_HwDetection(void)
{
	sim800l_HwStart();
	sim800l_ExitNet();
	
	sleep(5);

	sim800l_SocketClose();
	
	if(sim800l_CheckMode())
	{
		printf("sim800l_CheckMode error\n");
		return -1;
	}
	if(sim800l_CheckSimCard())
	{
		printf("sim800l_CheckSimCard error\n");
		return -1;
	}
	if(sim800l_CheckNwkRegStatus())
	{
		printf("sim800l_CheckNwkRegStatus error\n");
		return -1;
	}
	if(sim800l_CheckGprsRegStatus())
	{	
		printf("sim800l_CheckGprsRegStatus error\n");
		return -1;
	}
	if(sim800l_Setecho(false))
	{
		printf("sim800l_Setecho error\n");
		return -1;
	}
	
	return 0 ;
}

uint8_t sim800l_getMode(void)
{
	return sim800lMode;
}

/*********************************************************************
* @fn          sim800lInit
*
* @brief       初始化接口
*
* @param       
*
* @return      0:初始化成功 -1:初始化失败
*/
int sim800lInit(int fd)
{
	uartfd = fd;
	return sim800l_HwDetection();
}

/*********************************************************************/

