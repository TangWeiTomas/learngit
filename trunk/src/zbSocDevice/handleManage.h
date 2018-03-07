#ifndef __HANDLE_MANAGE_H__
#define __ELECTRICITY_METER_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "Types.h"

typedef void * HANDLE_MANAGE_HANDLE;

HANDLE_MANAGE_HANDLE HandleManage_Create(uint32_t maxListLength, uint32_t handleLength, uint32_t idLength);
void *HandleMange_GetHadnle(HANDLE_MANAGE_HANDLE manageHandle, void *id);



#endif