#ifndef _UDP_LISTENER_H_
#define _UDP_LISTENER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"
#include "logUtils.h"
#include "globalVal.h"


int8_t udpBroadcast_Init(struct event_base *base);
void udpBroadcast_relase(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
