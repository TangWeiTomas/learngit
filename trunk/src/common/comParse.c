/**************************************************************************************************
 * Filename:       ZigbeeDevHandle.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 * Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comParse.h"

/****************低字节在前，高字节在后***************************/
/************************************************************************
* 函数名 :cmdSet16bitVal(hostCmd *cmd, uint8_t val)
* 描述   :   设置2个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdSet16bitVal_lh(hostCmd *cmd, uint16_t val)
{
    if((cmd->idx + 2) < MaxPacketLength)
    {
        cmd->data[cmd->idx++] = val&0xff;
        cmd->data[cmd->idx++] = (val>>8)&0xff;
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdSet32bitVal(hostCmd *cmd, uint32_t val)
* 描述   :   设置4个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdSet32bitVal_lh(hostCmd *cmd, uint32_t val)
{
    if((cmd->idx + 4) < MaxPacketLength)
    {
        cmd->data[cmd->idx++] = val&0xff;
        cmd->data[cmd->idx++] = (val>>8)&0xff;
        cmd->data[cmd->idx++] = (val>>16)&0xff;
        cmd->data[cmd->idx++] = (val>>24)&0xff;
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdSetStringVal_lh(hostCmd *cmd, uint8_t *val, uint8_t len)
* 描述   :   设置4个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdSetStringVal_lh(hostCmd *cmd, uint8_t *val, uint16_t len)
{
    if(!len)
    {
        return false;
    }

    if((cmd->idx + len) < MaxPacketLength)
    {
        uint8_t cnt;
        for(cnt=0; cnt<len; cnt++)
        {
            cmd->data[cmd->idx+cnt]=val[len-1-cnt];
        }
        cmd->idx += len;
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdGet16bitVal_lh(hostCmd *cmd, uint16_t *val)
* 描述   :  获取一个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdGet16bitVal_lh(hostCmd *cmd, uint16_t *val)
{
    if ((cmd->idx + 2) <= cmd->size)
    {
        *val = 0x0000;
        *val = cmd->data[(cmd->idx)++];
        *val |= ((cmd->data[(cmd->idx)++]<<8)&0xff00);
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdGet32bitVal_lh(hostCmd *cmd, uint32_t *val)
* 描述   :  获取一个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdGet32bitVal_lh(hostCmd *cmd, uint32_t *val)
{
    if ((cmd->idx + 4) <= cmd->size)
    {
        *val = 0x00000000;
        *val = cmd->data[(cmd->idx)++];
        *val |= ((cmd->data[(cmd->idx)++]<<8)&0x0000ff00);
        *val |= ((cmd->data[(cmd->idx)++]<<16)&0x00ff0000);
        *val |= ((cmd->data[(cmd->idx)++]<<24)&0xff000000);
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdGetStringVal_lh(FS_uint8 data)
* 描述   :  获取一个字节数据,反着读取
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdGetStringVal_lh(hostCmd *cmd, uint8_t *val, uint16_t len)
{

    if (!len)
    {
        return false;
    }

    if ((cmd->idx + len) <= cmd->size)
    {
        uint8_t cnt;
        for(cnt=0; cnt<len; cnt++)
        {
            val[cnt] = cmd->data[cmd->idx+len-1-cnt];
        }
        cmd->idx += len;
        return false;
    }
    else
    {
        return true;
    }
}

/****************高字节在前，低字节在后***************************/

/************************************************************************
* 函数名 :cmdSet8bitVal(FS_uint8 data)
* 描述   :   设置一个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdSet8bitVal(hostCmd *cmd, uint8_t val)
{
    if((cmd->idx + 1) < MaxPacketLength)
    {
        cmd->data[cmd->idx++] = val;
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdSet16bitVal(hostCmd *cmd, uint8_t val)
* 描述   :   设置2个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdSet16bitVal(hostCmd *cmd, uint16_t val)
{
    if((cmd->idx + 2) < MaxPacketLength)
    {
        cmd->data[cmd->idx++] = (val>>8)&0xff;
        cmd->data[cmd->idx++] = val&0xff;
        return false;
    }
    else
    {
        return true;
    }
}


/************************************************************************
* 函数名 :cmdSet32bitVal(hostCmd *cmd, uint32_t val)
* 描述   :   设置4个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdSet32bitVal(hostCmd *cmd, uint32_t val)
{
    if((cmd->idx + 4) < MaxPacketLength)
    {
        cmd->data[cmd->idx++] = (val>>24)&0xff;
        cmd->data[cmd->idx++] = (val>>16)&0xff;
        cmd->data[cmd->idx++] = (val>>8)&0xff;
        cmd->data[cmd->idx++] = val&0xff;
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdSetStringVal(hostCmd *cmd, uint32_t val)
* 描述   :   设置4个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdSetStringVal(hostCmd *cmd, uint8_t *val, uint16_t len)
{
    if((cmd->idx + len) < MaxPacketLength)
    {
        memcpy((uint8_t *) & (cmd->data[cmd->idx]),val, len);
        cmd->idx += len;
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdGet8bitVal(FS_uint8 data)
* 描述   :  获取一个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdGet8bitVal(hostCmd *cmd, uint8_t *val)
{
    if ((cmd->idx + 1) <= cmd->size)
    {
        *val = 0x00;
        *val = cmd->data[(cmd->idx)++];
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdGet8bitVal(FS_uint8 data)
* 描述   :  获取一个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdGet16bitVal(hostCmd *cmd, uint16_t *val)
{
    if ((cmd->idx + 2) <= cmd->size)
    {
        *val = 0x0000;
        *val = cmd->data[(cmd->idx)++];
        *val <<= 8;
        *val |= cmd->data[(cmd->idx)++];
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdGet8bitVal(FS_uint8 data)
* 描述   :  获取一个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdGet32bitVal(hostCmd *cmd, uint32_t *val)
{
    if ((cmd->idx + 4) <= cmd->size)
    {
        *val = 0x00000000;
        *val = cmd->data[(cmd->idx)++];
        *val <<= 8;
        *val |= cmd->data[(cmd->idx)++];
        *val <<= 8;
        *val |= cmd->data[(cmd->idx)++];
        *val <<= 8;
        *val |= cmd->data[(cmd->idx)++];
        return false;
    }
    else
    {
        return true;
    }
}

/************************************************************************
* 函数名 :cmdGetStringVal(FS_uint8 data)
* 描述   :  获取一个字节数据
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdGetStringVal(hostCmd *cmd, uint8_t *val, uint16_t len)
{

    if (!len)
    {
        return false;
    }

    if ((cmd->idx + len) <= cmd->size)
    {
        memcpy(val, (uint8_t *) & (cmd->data[cmd->idx]), len);
        cmd->idx += len;
        return false;
    }
    else
    {
        return true;
    }
}


/************************************************************************
* 函数名 :cmdGetStringAddr(FS_uint8 data)
* 描述   :  获取数据地址
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdGetStringAddr(hostCmd *cmd, uint8_t **addr, uint16_t len)
{

    *addr = (uint8_t *) & (cmd->data[cmd->idx]);
    cmd->idx += len;
    return false;
}


/************************************************************************
* 函数名 :cmdSetFCS(hostCmd *cmd)
* 描述   :   设置数据包的校验值
* 输入   ：*val : 数据存储地址*cmd:数据包指针
* 输出   ：无
* 返回   ：uint8  返回处理结果TRUE 处理失败FALSE处理成功
************************************************************************/
uint8_t cmdSetFCS(hostCmd *cmd)
{
    uint8_t fcs=0;
    if((cmd->idx + 1) < MaxPacketLength)
    {
        uint16_t cnt;
        for(cnt=1; cnt<cmd->idx; cnt++)
        {
            fcs ^= cmd->data[cnt];
        }

        cmd->data[cmd->idx++] = fcs;

        return false;
    }
    else
    {
        return true;
    }
}

bool cmdSetCheckSum(hostCmd *cmd)
{
	uint16_t sum = 0;
	if((cmd->idx + 1) < MaxPacketLength)
	{
		uint16_t cnt;
        for(cnt=0; cnt<cmd->idx; cnt++)
        {
            sum += cmd->data[cnt];
        }

        cmd->data[cmd->idx++] = ((sum&0xff00)>>8) ;
        cmd->data[cmd->idx++] = (sum&0x00ff) ;
        return true;
	}
	
	return false;
}

void setMsgBlockFlag(SCmdFlag *cmdFlag,uint16_t opcode)
{
    cmdFlag->opcode = opcode;
    cmdFlag->flag = true;
}

uint8_t CheckMsgBlockFlag(SCmdFlag *cmdFlag,uint16_t opcode)
{
    if((cmdFlag->opcode == opcode)
       &&(cmdFlag->flag == true))
    {
        return true;
    }
    return false;
}

void ClearCmdFlag(SCmdFlag *cmdFlag)
{
    cmdFlag->opcode = 0xffff;
    cmdFlag->flag = false;
}

