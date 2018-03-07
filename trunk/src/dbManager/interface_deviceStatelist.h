/**************************************************************************************************
 * Filename:       interface_deviceStatelist.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    ?洢????豸????
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-02,20:10)    :   Create the file.
 *
 *************************************************************************/

#ifndef DB_DEVICE_STATE_LIST_H
#define DB_DEVICE_STATE_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include "zbSocCmd.h"
//#include "hal_types.h"
#include "Types.h"

typedef struct devState{
	uint8_t IEEEAddr[8];
	uint8_t endpoint;			    // Endpoint identifier
	uint8_t length;					//最大8字节
	uint8_t dataSegment[DATASEGMENT];
	uint16_t deviceID; 				//设备类型
}devState_t;

/*
 * devListAddDevice - create a device and add a rec to the list.
 */
void devStateListAddDevice(devState_t *epState);

devState_t * devStateListRemoveDeviceByIeeeEp(uint8_t ieeeAddr[],uint8_t endpoint);


devState_t * devStateListRemoveAllDevice(void);


/*
 * devStateListNumDevices - get the number of devices in the list.
 */
uint32_t devStateListNumDevices( void );

/*
 * devStateListInitDatabase - restore device list from file.
 */
void devStateListInitDatabase( char * dbFilename );

devState_t * devStateListGetNextDev(uint32_t *context);

devState_t * devStateListGetRecordByIeeeEp(uint8_t ieeeAddr[], uint8_t endpoint);

devState_t * devStateListRemoveDeviceByIeee( uint8_t ieeeAddr[] );

bool devStateListModifyRecordByIeeeEp(devState_t *epState);

#ifdef __cplusplus
}
#endif

#endif /* DB_DEVICE_STATE_LIST_H */
