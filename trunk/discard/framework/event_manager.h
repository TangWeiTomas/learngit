#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__
#include "zbSocCmd.h"
#include "interface_eventlist.h"
#include "queue.h"

uint8_t Event_ActionEventAdd(ActionEvent_t *event)	;
int eventList_DelEventWithSceneID(uint8_t sceneid);
uint8_t event_ProcessActionEvent(epInfo_t *epInfo,uint8_t state);
void eventList_Init(void);


#endif
