/**************************************************************************************************
 * Filename:       devStateMng.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-02,21:05)    :   Create the file.
 *
 *
 *************************************************************************/

#ifndef DEV_STATE_MNG_H
#define DEV_STATE_MNG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hal_types.h"
#include "zbSocCmd.h"

/********************************************************************/


void devState_updateSwitchVal(epInfo_t *epInfo,uint8 onoff);

void devState_updateHumitureVal(epInfo_t *epInfo,uint16 temp,uint16 hum);

bool devState_getHumitureVal(uint8* ieeeAddr,uint8 endpoint,uint16 *temp,uint16 *hum);


void devState_updateHumitureLightVal(epInfo_t *epInfo,uint16 temp,uint16 hum,uint16 light);

bool devState_getHumitureLightVal(uint8* ieeeAddr,uint8 endpoint,uint16 *temp,uint16 *hum,uint16 *light);
void devState_updateDoorLockVal(epInfo_t *epInfo,uint8 date);
int devState_updateevicePowerValue(epInfo_t *epInfo,uint32 value);
#ifdef __cplusplus
}
#endif

#endif /* DEV_STATE_MNG_H */
