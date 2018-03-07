#ifndef __TIME_TASK_LIST_H__
#define __TIME_TASK_LIST_H__



#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include "zbSocCmd.h"
#include "logUtils.h"

#define NEW_TIMER_TASK		0X00

typedef struct nodeMembersRecord_s
{
	struct nodeMembersRecord_s *next;
	uint8_t IEEEAddr[8];
	uint8_t endpoint;			   		// Endpoint identifier
	uint8_t length;						//���8�ֽ�
	uint8_t dataSegment[DATASEGMENT];
} nodeMembersRecord_t;

typedef struct timeTaskRecord_s
{
    void *next;				
    uint8_t timeTaskId;		//��ʱ����ID
	uint8_t timeTaskType;   //0:��������1:�ڵ�����
	uint8_t timeTaskState;	//��ʱ���Ƿ���Ч��0:�ر� 1:����

	//��ʱ����ʱ�����
	uint8_t timerType;		//0:����  1:ѭ��
	uint8_t hours;			//Сʱ
	uint8_t minute;			//����
    uint8_t second;			//��
	
	//��ʱ����ִ�еĳ�Ա
	uint8_t memberscount;	//�����¼�����
	//��������ִ��SCENDID�еĳ���
	uint8_t sceneid;		//��������ID
	//�豸����ִ��members�еĳ�Ա
	nodeMembersRecord_t *members;//��Ա
} timeTaskRecord_t;


/*
 * timeTaskListAddTask - create a timeTask and add a rec fto the list.
 */
//uint8_t timeTaskListAddTask(char *timeTaskNameStr, uint16_t groupId);

uint8_t timeTaskListAddTask( timeTaskRecord_t *Task);
timeTaskRecord_t * timeTaskListRemoveALLTask();
timeTaskRecord_t * timeTaskListRemoveTaskByID(uint8_t timeTaskId);


/*
 * timeTaskListAddTask - gets the scen id of a a timeTask
 */

uint8_t timeTaskListGetUnusedTaskId(void);
uint8_t timeTaskListGetTaskNum();

/*
 * timeTaskListGetNextTask - Return the next timeTask in the list.
 */
timeTaskRecord_t* timeTaskListGetNextTask(uint32_t *context);

/*
 * groupListInitDatabase - Restore Task List from file.
 */
timeTaskRecord_t * timeTaskListGetTaskByID(uint8_t key);
void timeTaskListInitDatabase(char * dbFilename);
uint16_t timeTaskListGetAllTask(void);

#ifdef __cplusplus
}
#endif

#endif
