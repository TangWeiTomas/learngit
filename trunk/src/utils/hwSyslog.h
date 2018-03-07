#ifndef _HW_SYSLOG_H_
#define _HW_SYSLOG_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
	OffLine =0x00,
	OnLine = 0x01,
}epState_t;

int hwSyslog(uint8_t addrs[8],uint8_t action);

#ifdef __cplusplus
}
#endif
#endif
