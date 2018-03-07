#ifndef __WATER_METER_UART_FORMAT_XX_H__
#define __WATER_METER_UART_FORMAT_XX_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "Types.h"

#define UART_FORMAT_MAX_HANDLE      100
#define PACKET_MAX_DATA_LENGTH         256
#define PACKET_ADDRESS_LENGTH   7

#define PACKET_EXCEPT_DATA_SIZE 13 //1+1+7+1+1+1+1



typedef struct
{
    uint8_t nMeterType;     //仪表类型
    uint8_t aAddress[PACKET_ADDRESS_LENGTH];    //地址域
    uint8_t nCtrlNumber;    //控制码
    uint8_t nLength;        //数据长度
    uint8_t aData[PACKET_MAX_DATA_LENGTH]; //数据域
    
}UART_FORMAT_PACKET_T;

typedef void *UART_FORMAT_HANDLE_T;

UART_FORMAT_PACKET_T *uartFormat_Resolve(UART_FORMAT_HANDLE_T handle , uint8_t nData);
int uartFormat_BuilPacket(uint8_t *outBuff, UART_FORMAT_PACKET_T *pPacket);
UART_FORMAT_HANDLE_T uartFormat_GetHandleForId(uint16_t identity);

#ifdef __cplusplus
}
#endif
#endif

