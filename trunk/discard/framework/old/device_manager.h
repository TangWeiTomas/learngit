/**************************************************************************************************
 * Filename:       device_manager.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-11-30,13:28)    :   Create the file.
 *
 *
 *************************************************************************/

#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_types.h"
#include "zbSocCmd.h"


uint8 devMgr_getAllRegisterOnlineDevices(uint8* devCount,uint8 *pBuf,uint16 *pBufLen);

uint8 devMgr_getAllUnregisterOnlineDevices(uint8* devCount,uint8 *pBuf,uint16 *pBufLen);

uint8 devMgr_UpdateDevOnlineStateByIeee(uint8* ieeeAddr, bool Status,uint8 rssi);

uint8 devMgr_UpdateDevOnlineTimeoutCounter(void);









#ifdef __cplusplus
}
#endif

#endif /* LOCAL_CMD_MNG_H */
