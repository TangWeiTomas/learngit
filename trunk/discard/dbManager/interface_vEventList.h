#ifndef _EVENT_LIST_H__
#define _EVENT_LIST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "queue.h"
#include "interface_eventlist.h"
#define EVENT_FAILE		-1
#define EVENT_SUCCE		 0

typedef struct{
	uint8_t 	IEEEAddr[8];			//�豸����
	uint8_t 	endpoint;			    // Endpoint identifier
	uint8_t 	active;					//����
}condition_t;

typedef struct eventList_t
{
	uint8_t evnetId;				//�¼�ID
	uint8_t eventState;				//�¼�ʹ��״̬
	uint8_t type;					//0: ����1:�ڵ�����
	uint8_t sceneid;				//����ID
	condition_t Condition; 			//����
    LIST_ENTRY(eventList_t) entry_;
} eventList_t;


eventList_t *vEventList_GetEventByIeeeEpSt(uint8_t ieee[8],uint8_t endpoint,uint8_t state);
eventList_t* vEventList_GetEventByIeeEpID(uint8_t ieee[8],uint8_t endpoint,uint8_t eventid);

int vEventList_ModifyEvent(ActionEvent_t *eventinfo);
int vEventList_RemoveEventByEventID(uint8_t eventid);
int vEventList_RemoveEventBySceneID(uint8_t sceneid);
int vEventList_addEvent(ActionEvent_t *eventinfo);
int vEventList_Init(void);
int vEventList_Distory(void);

#ifdef __cplusplus
}
#endif
#endif
