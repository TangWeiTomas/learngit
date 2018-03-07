/**************************************************************************************************
 * Filename:       globalDef.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    ����ȫ��.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00    �½��ļ�
 *
 */

#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_types.h"


typedef struct
{
    uint8 periodType;
    uint8 day;
    uint8 week;
    uint8 hour;
    uint8 min;
    uint8 sec;
} SPeriodTimers;

typedef struct
{
    uint8 stringLength;
    uint8 string[32];
} SStringType;

typedef struct
{
    //����·������������Ϣ
    SStringType stassid;
    SStringType stapwd;
    uint8 staauthMod;
    uint8 stakeyMod;
    //�����AP�ȵ�����
    SStringType apssid;
    SStringType appwd;
    SStringType serverIPAddr;
    uint16 serverPort;
} SConfigInfos;


#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_DEFINE_H */
