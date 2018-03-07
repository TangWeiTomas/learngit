/*******************************************************************
    Filename:   comParse.h
    Author:     zxb
    Timer:      2014.1.4
    Describe:       本文件用于将数据包解析为具体的指令
    Version 1.01 :

*******************************************************************/
#ifndef _COM_PARSE_H_
#define _COM_PARSE_H_

//#include "hal_types.h"
#include <stdbool.h>
#include <stdint.h>
#define MaxCacheBufferLength      1024

typedef struct
{
    uint8_t buffer[MaxCacheBufferLength];
    uint16_t bufferNum;
} dataBuffer_t;

#define MaxPacketLength 2048

typedef struct
{
    uint8_t data[MaxPacketLength];
    uint16_t size;
    uint16_t idx;
} hostCmd;

typedef struct
{
    bool   flag;
    uint16_t opcode;
} SCmdFlag;

#define MSG_CMD_REQ 	0x01
#define MSG_CMD_CFM 	0x02
#define MSG_CMD_IND 	0x03
#define MSG_CMD_RESP    0x04


/****************低字节在前，高字节在后***************************/
uint8_t cmdSet16bitVal_lh(hostCmd *cmd, uint16_t val);

uint8_t cmdSet32bitVal_lh(hostCmd *cmd, uint32_t val);

uint8_t cmdSetStringVal_lh(hostCmd *cmd, uint8_t *val, uint16_t len);

uint8_t cmdGet16bitVal_lh(hostCmd *cmd, uint16_t *val);

uint8_t cmdGet32bitVal_lh(hostCmd *cmd, uint32_t *val);

uint8_t cmdGetStringVal_lh(hostCmd *cmd, uint8_t *val, uint16_t len);


/****************高字节在前，低字节在后***************************/

uint8_t cmdSet8bitVal(hostCmd *cmd, uint8_t val);

uint8_t cmdSet16bitVal(hostCmd *cmd, uint16_t val);

uint8_t cmdSet32bitVal(hostCmd *cmd, uint32_t val);

uint8_t cmdSetStringVal(hostCmd *cmd, uint8_t *val, uint16_t len);

uint8_t msgDataHeader(hostCmd *cmd, uint8_t *tp,uint8_t *msgFlg,uint16_t *opcode);

uint8_t cmdGet8bitVal(hostCmd *cmd, uint8_t *val);

uint8_t cmdGet16bitVal(hostCmd *cmd, uint16_t *val);

uint8_t cmdGet32bitVal(hostCmd *cmd, uint32_t *val);

uint8_t cmdGetStringVal(hostCmd *cmd, uint8_t *val, uint16_t len);

uint8_t cmdGetStringAddr(hostCmd *cmd, uint8_t **addr, uint16_t len);

uint8_t cmdSetFCS(hostCmd *cmd);
bool cmdSetCheckSum(hostCmd *cmd);

void setMsgBlockFlag(SCmdFlag *cmdFlag,uint16_t opcode);

uint8_t CheckMsgBlockFlag(SCmdFlag *cmdFlag,uint16_t opcode);

void ClearCmdFlag(SCmdFlag *cmdFlag);

#endif

