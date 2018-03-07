/**************************************************************************
 * Filename:       uart.c
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    串口驱动.
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/


/*********************************************************************
* INCLUDES
*/
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
	
#include <errno.h>
#include <string.h>


#include "logUtils.h"
#include "uart.h"
/*********************************************************************
* MACROS
*/

/*********************************************************************
* CONSTANTS
*/

/*********************************************************************
* TYPEDEFS
*/

/*********************************************************************
* GLOBAL VARIABLES
*/

UartFifo_t gUartRxfifo;    
/*********************************************************************
* LOCAL VARIABLES
*/


/*********************************************************************
* LOCAL FUNCTIONS
*/


/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* @fn          fxUartOpen
*
* @brief       初始化串口
*
* @param       devName - 串口名称
*
* @return      设备描述符，<0:失败，>0成功
*/
int fxUartOpen(char *devName)
{
	struct termios tio;
    int zbSoc_fd;
    log_debug("fxUartOpen\n");

	if(devName == NULL)
	{
		log_err("devName error");
		return -1;
	}
    /* open the device to be non-blocking (read will return immediatly) */
    // zbSoc_Uart_Server.zbSoc_fd = open(devicePath, O_RDWR | O_NOCTTY | O_NONBLOCK);
    // zbSoc_fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
    zbSoc_fd = open(devName, O_RDWR | O_NOCTTY | O_NDELAY);
    if (zbSoc_fd < 0)
    {
        perror(devName);
        return (-1);
    }

	tcflush(zbSoc_fd, TCIOFLUSH);

	bzero(&tio,sizeof(tio));
	
	if(tcgetattr(zbSoc_fd,&tio) !=0 )
	{
		log_debug("tcgetatter failed\n");
		return -1;
	}
	
    //make the access exclusive so other instances will return -1 and exit
    ioctl( zbSoc_fd, TIOCEXCL);
    
	/* c-iflags
	 B115200 : set board rate to 115200
	 CRTSCTS : HW flow control (disabled below)
	 CS8     : 8n1 (8bit,no parity,1 stopbit)
	 CLOCAL  : local connection, no modem contol
	 CREAD   : enable receiving characters*/
	tio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	/* c-iflags
	 ICRNL   : maps 0xD (CR) to 0x10 (LR), we do not want this.
	 IGNPAR  : ignore bits with parity erros, I guess it is
	 better to ignStateore an erronious bit then interprit it incorrectly. */
	tio.c_iflag = IGNPAR & ~ICRNL;
	tio.c_oflag = 0;
	tio.c_lflag = 0;

//  cfsetispeed(&tio,B57600);     
//  cfsetospeed(&tio,B57600);  
//    
//  tio.c_cflag |= CLOCAL | CREAD;
//  tio.c_cflag &= ~CRTSCTS; 
//  tio.c_cflag &= ~CSIZE;
//  tio.c_cflag |=  CS8  ;
//	tio.c_cflag &= ~PARENB; 
//	tio.c_iflag &= ~INPCK;
//	tio.c_cflag &= ~CSTOPB;
//	tio.c_iflag |= IGNPAR & ~ICRNL;
//	 
//	tio.c_oflag &= ~OPOST;
//	tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	
    //清除所有正在发生的I/O数据
    tcflush(zbSoc_fd, TCIOFLUSH);
  
    tcsetattr(zbSoc_fd, TCSANOW, &tio);

    return  zbSoc_fd;
}

/*********************************************************************
* @fn          fxUartClose
*
* @brief       初始化串口
*
* @param       fd - 设备描述符，
*
* @return      void
*/

void fxUartClose(int fd)
{
	if(fd != -1)
	{
		close(fd);
	}
}

/*********************************************************************
* @fn          fxUartSend
*
* @brief       发送串口出具
*
* @param       fd - 设备描述符，
*			   buf - 待发送数据缓冲区
*			   size - 数据大小
*
* @return      0:成功 -1:失败
*/
int fxUartSend(int fd,uint8_t *buf,uint16_t size)
{
	int remain = 0;
	int offset = 0;
	int ret = 0;
	int sub = 0;
	uint8_t *pBuf = NULL;
	
	if(fd == -1 || buf == NULL || size == 0)
	{
		log_err("args is error\n");
		return -1;
	}

	pBuf = buf;
	remain = size;

	while(remain > 0)
	{
		sub = (remain >= 8?8:remain);
		ret = write(fd,pBuf+offset,sub);

		tcflush(fd, TCOFLUSH);
		usleep(5000);

        remain -= 8;
        offset += 8;
	}

	return 0;
}

/*********************************************************************
* @fn          fxUartRead
*
* @brief       发送串口出具
*
* @param       fd - 设备描述符，
*			   buf - 待发送数据缓冲区
*			   size - 数据大小
*
* @return      0 > :成功 -1:失败 -2:串口打开错误
*/

int fxUartRead(int fd,uint8_t *buf,uint16_t size)
{
	int byteRead = 0;
	int byteCnt = 0;
	
	if(fd == -1 || buf == NULL || size <= 0)
		return -1;

	while(1)
	{
		byteRead = read(fd,&buf[byteCnt],(size - byteCnt));
		if(byteRead <= 0)
			break;
		byteCnt +=byteRead;
		usleep(3000);
	}

	return byteCnt;
}

/*********************************************************************
* @fn          fxUartRxfifoReset
*
* @brief       发送串口出具
*
* @param       fd - 设备描述符，
*			   buf - 待发送数据缓冲区
*			   size - 数据大小
*
* @return      0 > :成功 -1:失败 -2:串口打开错误
*/

void fxUartRxfifoClear(void)
{
	memset(&gUartRxfifo,0,sizeof(gUartRxfifo));
}
/*********************************************************************/

