/**************************************************************************************************
 * Filename:       globalDef.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    定义全局.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00    新建文件
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
    //网络路由器的配置信息
    SStringType stassid;
    SStringType stapwd;
    uint8 staauthMod;
    uint8 stakeyMod;
    //自身的AP热点名称
    SStringType apssid;
    SStringType appwd;
    SStringType serverIPAddr;
    uint16 serverPort;
} SConfigInfos;


#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_DEFINE_H */
