#ifndef __ONE_KEY_MATCH_H__
#define __ONE_KEY_MATCH_H__

#include <stdint.h>
#include "globalVal.h"
#define ArraySize(Array)	(sizeof(Array)/sizeof(Array[0]))

extern uint8_t IRC_Remote_Learn_Count;
extern uint8_t IRC_Remote_Learn_Device_Type;
extern uint8_t IRC_Remote_Learn_Device_Data_Type;

#define ARC_BRAND_COUNT  66
#define TV_BRAND_COUNT   41
#define STB_BRAND_COUNT  73

typedef struct oneKeyMatchInfo{
	int LibSize;
	const char **oneKeyMatchLib;
	char *BrandName;
}oneKeyMatchInfo_t;

typedef struct sortOneKeyMatch{
	uint16_t Matching;
	uint16_t ArcLibIndex;
	//uint16_t DeviceBrandIndex;
	//uint16_t DeviceTypeIndex;
}sortOneKeyMatch_t;
 
extern const oneKeyMatchInfo_t arc_one_key_match_info_struct[];
extern const oneKeyMatchInfo_t tv_one_key_match_info_struct[];
extern const oneKeyMatchInfo_t stb_one_key_match_info_struct[];

int StrToHex(char *p,uint8_t index);
int OneKeyMatchDevice(hostCmd *srcCmd,hostCmd *dstCmd);

#endif
