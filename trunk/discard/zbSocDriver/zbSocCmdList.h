#ifndef __ZB_SOC_CMD_LIST_H__
#define __ZB_SOC_CMD_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zbSocPrivate.h"
#include "zbSocCmd.h"
#include "event.h"

bool zblist_evInit(struct event_base *base);
bool zblist_add(epInfo_t *epinfo,uint8_t *cmd,uint8_t cmdlen);
bool zblist_remove(uint16_t nwkAddr,uint8_t endpoint);


#ifdef __cplusplus
}
#endif
#endif /*__ZB_SOC_CMD_LIST_H__*/
