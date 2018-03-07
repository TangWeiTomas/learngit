#ifndef __PERMISSION_MANAGE_H__
#define __PERMISSION_MANAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"

#define PERMI_BUF_SIZE	64

#if PERMIMNG

extern bool g_PermiMngisPass;
extern uint64_t g_PermiMngRequestInterval;
extern bool g_PermiMngRequestScuess;

void PermiMng_WriteConfig(uint8_t permiType,time_t permiData);
bool PermiMng_Config(void);
int PermiMng_close(void);
int PermiMng_SendMsg(uint8_t* cmdMsg,uint16_t cmdMsgLen);
uint64_t PermiMng_getRequestIntr(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
