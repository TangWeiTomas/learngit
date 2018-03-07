#ifndef __ZB_SOC_UART_H__
#define __ZB_SOC_UART_H__

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Types.h"

#include "comParse.h"
#include "globalVal.h"
#include "Timer_utils.h"


/******************************************************************************
 * Types
 *****************************************************************************/
typedef struct uart_event_s
{
	int zbSoc_fd ;
	int timeout;
	char *deviceName;
	struct event *Uart_event;
	struct event_base *base;
	
#ifdef UART_USE_BUFFER
	struct evbuffer *buffer;
#endif

	void (* Uart_Handler_cb)(hostCmd *cmd);
//	int32_t (*uart_Hardware_init_cb)(char*device);
//	void (*uart_Hardware_close_cb)(void);
//	int8_t (*zbpower_on)(void);			//控制协调器电源开启
//	int8_t (*zbpower_off)(void);			//控制协调器电源关闭
}uart_event_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern uart_event_t g_uart_event;

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/
// ZLL Soc API
//configuration API's
bool zbSocUart_evInit(struct event_base *base,const char *device);
void zbSocUart_evRelase(void);
void zbSocCmdSend(uint8_t* buf, uint16_t len);
bool zbSocUart_event(uart_event_t *u_event);
void zbSocUart_event_relase(void);
void zbSocUart_timeout_Set(int cnt);

int zbSocUart_timeout_get(void);


#ifdef __cplusplus
}
#endif

#endif
