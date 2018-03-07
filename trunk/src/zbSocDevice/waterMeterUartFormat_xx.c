#include "waterMeterUartFormat_xx.h"
#include "string.h"


#define STX 0x68
#define ETX 0x16

typedef struct
{
    uint8_t nStep;
    uint32_t nCount;
    UART_FORMAT_PACKET_T  pack;
}UART_FORMAT_DRIVE_T;

typedef struct
{
    UART_FORMAT_HANDLE_T handle;
    uint16_t id;
    uint32_t usedCheckSum;
}HANDLE_MANAGE_T;

HANDLE_MANAGE_T handleList[UART_FORMAT_MAX_HANDLE];

/******************************************************************
  * @函数说明：   计算校验码
  * @输入参数：   FORMAT_PACKET_T *pPacket 数据包
  * @输出参数：   无
  * @返回参数：   uint8_t 校验码         
  * @修改记录：   
******************************************************************/
static uint8_t Format_CalculateCheckSum(UART_FORMAT_PACKET_T *pPacket)
{
    uint8_t i;
    uint8_t checkSum = STX;
    
    if (pPacket == NULL)
    {
        return 0; 
    }
    
    checkSum += pPacket->nMeterType;
    checkSum += pPacket->nCtrlNumber;
    checkSum += pPacket->nLength;
    
    for (i = 0; i < PACKET_ADDRESS_LENGTH; i++)
    {
        checkSum += pPacket->aAddress[i];
    }
    
    for (i = 0; i < pPacket->nLength; i++)
    {
        checkSum += pPacket->aData[i];
    }
    
    return checkSum;
}


/******************************************************************
  * @函数说明：   解析接收到的数据
  * @输入参数：   void *pHandle  句柄 
                 uint8_t nData  数据
  * @输出参数：   无
  * @返回参数：   FORMAT_PACKET_T 解析后的数据包，未完成时返回NULL              
  * @修改记录：   
******************************************************************/
UART_FORMAT_PACKET_T *uartFormat_Resolve(UART_FORMAT_HANDLE_T handle , uint8_t nData)
{    
    UART_FORMAT_DRIVE_T *uartFormat = handle;
    switch(uartFormat->nStep)
    {
        case 0:
        {
            if (nData == STX)   //数据头
            {
                uartFormat->nStep++;
            }
        }
        break;

        case 1:
        {
            uartFormat->pack.nMeterType = nData;
            uartFormat->nStep++;
            uartFormat->nCount = 0;
        }
        break;
        
        case 2:
        {
            uartFormat->pack.aAddress[uartFormat->nCount] = nData;
            uartFormat->nCount++;
            if (uartFormat->nCount >= PACKET_ADDRESS_LENGTH)    //长度够了
            {
                uartFormat->nStep++;
                uartFormat->nCount = 0;
            }
        }
        break;
        
        case 3:
        {
            uartFormat->pack.nCtrlNumber = nData;
            uartFormat->nStep++;
        }
        break;
        
        
        case 4:
        {
            uartFormat->nCount = 0;
            uartFormat->pack.nLength = nData;
            uartFormat->nStep++;
        }
        break;
        
        case 5:
        {
            uartFormat->pack.aData[uartFormat->nCount] = nData;
            uartFormat->nCount++;
            
            if (uartFormat->nCount >= uartFormat->pack.nLength)    //长度够了
            {
                uartFormat->nStep++;
                uartFormat->nCount = 0;
            }
        }
        break;
        
        case 6:
        {
            if (Format_CalculateCheckSum(&uartFormat->pack) == nData)
            {
                uartFormat->nStep++;
            }
            else
            {
                uartFormat->nStep = 0;
            }
        }
        break;
        
        case 7:
        {
            uartFormat->nStep = 0;
            if (ETX == nData)
            {
                return &uartFormat->pack;
            }
        }
        break;
    }
    
    return NULL;
}

/******************************************************************
  * @函数说明：   构建并且发送数据包
  * @输入参数：   FORMAT_HADNLE_T formatHandle  句柄 
                  FORMAT_PACKET_T *pPacket      数据包
  * @输出参数：   无
  * @返回参数：   int 发送的数据长度，错误返回-1              
  * @修改记录：   
******************************************************************/
int uartFormat_BuilPacket(uint8_t *outBuff, UART_FORMAT_PACKET_T *pPacket)
{
    if (pPacket == NULL) 
    {
        return -1;
    }
        
    uint8_t *buff = outBuff;
    
    if (buff == NULL)
    {
        return -1;
    }
    
    uint8_t count = 0;
        
    buff[count++] = STX;                        //帧头
    buff[count++] = pPacket->nMeterType;        //仪表类型
    memcpy(&buff[count], pPacket->aAddress, PACKET_ADDRESS_LENGTH); //地址域
    count += PACKET_ADDRESS_LENGTH; 
    buff[count++] = pPacket->nCtrlNumber;   //控制码qgh
    buff[count++] = pPacket->nLength;       //数据长度
    memcpy(&buff[count], pPacket->aData, pPacket->nLength); //数据 
    count += pPacket->nLength;  
    buff[count++] = Format_CalculateCheckSum(pPacket);  //校验码
    buff[count++] = ETX;    //帧尾
    
    return count;
}

UART_FORMAT_HANDLE_T uartFormat_GetHandleForId(uint16_t identity)
{
    uint32_t i;
    
    for(i = 0; i < UART_FORMAT_MAX_HANDLE; i++)
    {
        if(handleList[i].usedCheckSum == (handleList[i].id + (int)handleList[i].handle) && handleList[i].handle != NULL)    //已使用
        {
            if (handleList[i].id == identity)
            {
                return handleList[i].handle;
            }
        }
        else
        {
            handleList[i].handle = malloc(sizeof(UART_FORMAT_DRIVE_T));
            memset(handleList[i].handle, 0, sizeof(HANDLE_MANAGE_T));
            handleList[i].id = identity;
            handleList[i].usedCheckSum = handleList[i].id + (int)handleList[i].handle;
            return handleList[i].handle;
        }
    }
    return NULL;
}


