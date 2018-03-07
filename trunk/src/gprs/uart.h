/**************************************************************************
 * Filename:       uart.h
 * Author:         edward
 * E-Mail:         ouxiangping@feixuekj.cn
 * Description:    SIM800L 串口驱动
 *
 *  Copyright (C) 2016 FeiXue Company - http://www.feixuekj.cn/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *************************************************************************/

#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "stdint.h"
#include "stdio.h"

/*********************************************************************
 * CONSTANTS
 */
 
#define UART_MAX_FIFO_SIZE	1024
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
	uint8_t fifo[UART_MAX_FIFO_SIZE];
	uint16_t byteCnt;
	uint16_t ridx;
	uint16_t widx;
}UartFifo_t;

/*********************************************************************
 * VARIABLES
 */
 
 extern UartFifo_t gUartRxfifo;    
/*********************************************************************
 * FUNCTIONS
 */
extern int fxUartOpen(char *devName);
extern void fxUartClose(int fd);
extern  int fxUartSend(int fd,uint8_t *buf,uint16_t size);
extern  int fxUartRead(int fd,uint8_t *buf,uint16_t size);
extern void fxUartRxfifoClear(void);
/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __ENCRYPT_H__ */

