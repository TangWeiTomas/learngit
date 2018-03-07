#ifndef __ZBDEV_DOORLOCK_H__
#define __ZBDEV_DOORLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "zbSocCmd.h"

void zbDevDoorLock_virtualDeviceRegister(epInfoExtended_t *epInfoEx);
uint8 zbDevDoorLock_SetOnOffState(epInfo_t *epInfo,uint8 switchcmd);



#ifdef __cplusplus
}
#endif

#endif 

