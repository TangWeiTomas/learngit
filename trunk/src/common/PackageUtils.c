
#include "PackageUtils.h"
/************************************************************************
* 函数名 :checkPacketFCS(UINT8* data,UINT16 length)
* 描述   :   检测数据包校验是否正确
* 输入   ：
* 输出   ：无
* 返回   ：false 数据包校验不正确true:数据包校验正确
************************************************************************/
bool checkPacketFCS(uint8_t* data,uint16_t length)
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

/************************************************************************
* 函数名 :lookupFirstPacketHeader(UINT8* data,UINT16 length,UINT16* startPos)
* 描述   :   检测是否有完整数据包
* 输入   ：
* 输出   ：无
* 返回   ：int  返回:-1 数据包长度不对，0未找到包头，1找到包头
            startPos 是0x9f的位置
************************************************************************/
uint8_t lookupFirstPacketHeader(uint8_t* data,uint16_t length,uint16_t* startPos)
{
    uint16_t cnt;
    *startPos = 0xffff;

    //先查找包头
    for(cnt=0; cnt<length; cnt++)
    {
        if(data[cnt]==CMD_MSG_FLAG)
        {
            *startPos = cnt;
            return FindPacketHeader;
        }
    }
	
    return FindNoHeader;
}

/************************************************************************
* 函数名 :lookupSocFirstPacketHeader(UINT8* data,UINT16 length,UINT16* startPos)
* 描述   :   检测是否有完整数据包
* 输入   ：
* 输出   ：无
* 返回   ：int  返回:-1 数据包长度不对，0未找到包头，1找到包头
            startPos 是0x9f的位置
************************************************************************/
bool lookupSocFirstPacketHeader(uint8_t* data,uint16_t length,uint16_t* startPos)
{
    uint16_t cnt;
    *startPos = 0xffff;

    //先查找包头
    for(cnt=0; cnt<length; cnt++)
    {
        if(data[cnt]==SOC_MSG_FLAG)
        {
            *startPos = cnt;
            return true;
        }
    }

    return false;
}

