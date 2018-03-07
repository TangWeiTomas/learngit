
#include "PackageUtils.h"
/************************************************************************
* ������ :checkPacketFCS(UINT8* data,UINT16 length)
* ����   :   ������ݰ�У���Ƿ���ȷ
* ����   ��
* ���   ����
* ����   ��false ���ݰ�У�鲻��ȷtrue:���ݰ�У����ȷ
************************************************************************/
bool checkPacketFCS(uint8_t* data,uint16_t length)
{
    uint16_t cnt;
    uint8_t result=0;
    //�Ȳ��Ұ�ͷ
    for(cnt=0; cnt<length; cnt++)
    {
        result ^= data[cnt];
    }

    //У����Ϊ0�������У����ȷ
    if(result==0)
    {
        return true;
    }

    return false;
}

/************************************************************************
* ������ :lookupFirstPacketHeader(UINT8* data,UINT16 length,UINT16* startPos)
* ����   :   ����Ƿ����������ݰ�
* ����   ��
* ���   ����
* ����   ��int  ����:-1 ���ݰ����Ȳ��ԣ�0δ�ҵ���ͷ��1�ҵ���ͷ
            startPos ��0x9f��λ��
************************************************************************/
uint8_t lookupFirstPacketHeader(uint8_t* data,uint16_t length,uint16_t* startPos)
{
    uint16_t cnt;
    *startPos = 0xffff;

    //�Ȳ��Ұ�ͷ
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
* ������ :lookupSocFirstPacketHeader(UINT8* data,UINT16 length,UINT16* startPos)
* ����   :   ����Ƿ����������ݰ�
* ����   ��
* ���   ����
* ����   ��int  ����:-1 ���ݰ����Ȳ��ԣ�0δ�ҵ���ͷ��1�ҵ���ͷ
            startPos ��0x9f��λ��
************************************************************************/
bool lookupSocFirstPacketHeader(uint8_t* data,uint16_t length,uint16_t* startPos)
{
    uint16_t cnt;
    *startPos = 0xffff;

    //�Ȳ��Ұ�ͷ
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



