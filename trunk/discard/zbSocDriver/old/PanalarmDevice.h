#ifndef __PANALARM_DEVICE_H__
#define __PANALARM_DEVICE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"
#include "zbSocUart.h"
#include "hal_types.h"
#include "Timer_utils.h"
#include "LogUtils.h"
#include "globalVal.h"
#include "zbSocCmd.h"
#include "zbSocPrivate.h"

int8 PanalarmDevice_UpdateDeviceState(epInfo_t *epinfo);


#ifdef __cplusplus
}
#endif

#endif
