#ifndef __PACKAGE_UTILS__
#define __PACKAGE_UTILS__

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "Types.h"
//#include "hal_types.h"
#include "logUtils.h"
#include "interface_srpcserver_defs.h"
#include "zbSocCmd.h"

bool checkPacketFCS(uint8_t* data,uint16_t length);
uint8_t lookupFirstPacketHeader(uint8_t* data,uint16_t length,uint16_t* startPos);
bool lookupSocFirstPacketHeader(uint8_t* data,uint16_t length,uint16_t* startPos);
#endif
