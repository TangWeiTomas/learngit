/**************************************************************************************************
 * Filename:       interface_crontablist.c
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*********************************************************************
 * INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "interface_timetasklist.h"

#include "interface_scenelist.h"
//#include "hal_types.h"
#include "SimpleDBTxt.h"
#include "interface_srpcserver.h"

static db_descriptor * db;
#define MAX_SUPPORTED_DEVICE_NAME_LENGTH 32

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    char* name;
    uint16_t groupId;
} timeTask_key_NA_GID;


/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

void timeTaskListInitDatabase(char * dbFilename)
{
    db = sdb_init_db(dbFilename, sdbtGetRecordSize, sdbtCheckDeleted,
                     sdbtCheckIgnored, sdbtMarkDeleted,
                     (consolidation_processing_f) sdbtErrorComment, SDB_TYPE_TEXT, 0);
    sdb_consolidate_db(&db);
}

void timeTaskListrelaseDatabase(void)
{
	sdb_release_db(&db);
}


static char * timeTaskListComposeRecord(timeTaskRecord_t *timeTask, char * record)
{
	
	uint8_t mlength  =0;
    sprintf(record, " 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X ,", //leave a space at the beginning to mark this record as deleted if needed later, or as bad format (can happen if edited manually). Another space to write the reason of bad format.
            timeTask->timeTaskId,timeTask->timeTaskState,timeTask->timeTaskType,
            timeTask->timerType,timeTask->hours,timeTask->minute,
            timeTask->second);
	
	//�ڵ㶨ʱ����
   if(timeTask->timeTaskType== EVENT_TYPE_NODE_ACTION)
	{
		
		sprintf(record+strlen(record)," 0x%02X ,",timeTask->memberscount);
		
		nodeMembersRecord_t *nodeMembers  = timeTask->members;

		while(nodeMembers != NULL)
		{
			sprintf(record+strlen(record),
            " %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X , 0x%02X , 0x%02X ,",
            nodeMembers->IEEEAddr[0], nodeMembers->IEEEAddr[1], nodeMembers->IEEEAddr[2],
            nodeMembers->IEEEAddr[3], nodeMembers->IEEEAddr[4], nodeMembers->IEEEAddr[5],
            nodeMembers->IEEEAddr[6], nodeMembers->IEEEAddr[7], nodeMembers->endpoint,
			nodeMembers->length
			);

			//��Ҫ�洢�����ݰ��ֽڴ洢
			for(mlength=0; mlength<(nodeMembers->length); mlength++)
			{
			    sprintf(record+strlen(record)," 0x%02X ,",nodeMembers->dataSegment[mlength]);
			}

			if(nodeMembers->next==NULL)
			{
		    	//��Ҫ�洢�����һ�����ݸ�ʽ��ͬ����������
				sprintf(record+strlen(record)," 0x%02X",nodeMembers->dataSegment[mlength]);	
			}
			else
			{
				sprintf(record+strlen(record)," 0x%02X ,",nodeMembers->dataSegment[mlength]);
			}
			
			nodeMembers = nodeMembers->next;
		}	
		sprintf(record + strlen(record), "\n");
	}
	else
	{
		sprintf( record+strlen(record)," 0x%02X\n",timeTask->sceneid);	
	}
    return record;
}

static timeTaskRecord_t * timeTaskListParseRecord(char * record)
{
	uint8_t i=0;
    char * pBuf = record + 1; //+1 is to ignore the 'for deletion' mark that may just be added to this record.

    static timeTaskRecord_t timeTask;
    static nodeMembersRecord_t member[MAX_SUPPORTED_NODE_MEMBERS];
    nodeMembersRecord_t ** nextMemberPtr;

    parsingResult_t parsingResult = { SDB_TXT_PARSER_RESULT_OK, 0 };

	uint8_t mCount = 0;
	
    if (record == NULL)
    {
        return NULL;
    }
    
	memset(&timeTask,0,sizeof(timeTaskRecord_t));
    sdb_txt_parser_get_numeric_field(&pBuf, &(timeTask.timeTaskId), 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &(timeTask.timeTaskState), 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &(timeTask.timeTaskType), 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &(timeTask.timerType), 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &(timeTask.hours), 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &(timeTask.minute), 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &(timeTask.second), 1, false, &parsingResult);
	
	//�����ڵ�����
	if(timeTask.timeTaskType == EVENT_TYPE_NODE_ACTION)
	{

		sdb_txt_parser_get_numeric_field(&pBuf, &(timeTask.memberscount), 1, false, &parsingResult);

		nextMemberPtr = &(timeTask.members);

		for (i = 0;(parsingResult.code == SDB_TXT_PARSER_RESULT_OK) && (i < MAX_SUPPORTED_NODE_MEMBERS); i++)
		{
			*nextMemberPtr = &(member[i]);

			sdb_txt_parser_get_hex_field(&pBuf,member[i].IEEEAddr, 8, &parsingResult);
			sdb_txt_parser_get_numeric_field(&pBuf, &member[i].endpoint, 1, false, &parsingResult);
			sdb_txt_parser_get_numeric_field(&pBuf,&member[i].length, 1, false, &parsingResult);

			for(mCount=0; mCount<member[i].length; mCount++)
			{
			    sdb_txt_parser_get_numeric_field(&pBuf, &member[i].dataSegment[mCount], 1, false, &parsingResult);
			}

			nextMemberPtr = &(member[i].next);
		}
		*nextMemberPtr = NULL;
	}
	else
	{
		sdb_txt_parser_get_numeric_field(&pBuf, &timeTask.sceneid, 1, false, &parsingResult);
		timeTask.members = NULL;
	}

    if ((parsingResult.code != SDB_TXT_PARSER_RESULT_OK)
        && (parsingResult.code != SDB_TXT_PARSER_RESULT_REACHED_END_OF_RECORD))
    {
        sdbtMarkError(db, record, &parsingResult);
        return NULL;
    }
	
    return &timeTask;
}

/*
static int timeTaskListCheckKeyName(char * record, char * key)
{
    timeTaskRecord_t * timeTask;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    timeTask = timeTaskListParseRecord(record);
    if (timeTask == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if(strcmp(timeTask->name, key) == 0)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}
*/

static int timeTaskListCheckKeyId(char * record, uint8_t * key)
{
    timeTaskRecord_t * timeTask;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    timeTask = timeTaskListParseRecord(record);
    if (timeTask == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if (timeTask->timeTaskId == *key)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int timeTaskListCheckAll(char * record, uint8_t * key)
{
    timeTaskRecord_t * timeTask;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    timeTask = timeTaskListParseRecord(record);
    if (timeTask == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }
    else
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

timeTaskRecord_t * timeTaskListGetTaskByID(uint8_t key)
{
    char * rec;

    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)timeTaskListCheckKeyId);
    if (rec == NULL)
    {
        return NULL;
    }

    return timeTaskListParseRecord(rec);
}

/*
timeTaskRecord_t * timeTaskListGetSceneByName(char *timeTaskNameStr)
{
    char * rec;

    rec = SDB_GET_UNIQUE_RECORD(db, timeTaskNameStr, (check_key_f)timeTaskListCheckKeyName);
    if (rec == NULL)
    {
        return NULL;
    }

    return timeTaskListParseRecord(rec);
}
*/

uint8_t timeTaskListGetUnusedTaskId(void)
{
    static uint8_t lastUsedTaskId = 0;

    lastUsedTaskId++;

    while (SDB_GET_UNIQUE_RECORD(db, &lastUsedTaskId, (check_key_f)timeTaskListCheckKeyId)
           != NULL)
    {
        lastUsedTaskId++;
    }

    return lastUsedTaskId;
}

/*
timeTaskRecord_t * timeTaskListRemoveSceneByName(char *timeTaskNameStr)
{
    return timeTaskListParseRecord(
               sdb_delete_record(db, timeTaskNameStr, (check_key_f)timeTaskListCheckKeyName));
}
*/

timeTaskRecord_t * timeTaskListRemoveTaskByID(uint8_t timeTaskId)
{
    return timeTaskListParseRecord(sdb_delete_record(db, &timeTaskId, (check_key_f)timeTaskListCheckKeyId));
}


timeTaskRecord_t * timeTaskListRemoveALLTask(void)
{
    uint32_t context = 0;
    while(timeTaskListGetNextTask(&context) != NULL)
    {
        timeTaskListParseRecord( sdb_delete_record(db, 0x00, (check_key_f)timeTaskListCheckAll));
    }

    return NULL;

}

uint8_t timeTaskListGetTaskNum(void)
{
    uint8_t i=0;
    uint32_t context = 0;
    while(timeTaskListGetNextTask(&context) != NULL)
    {
        ++i;
    }

    return i;
}

bool timeTaskListModifyRecordByID(uint8_t timeTaskid,timeTaskRecord_t *timeTask)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};
	bool rtn = false;
	char *str = NULL;

    str = timeTaskListComposeRecord(timeTask, rec);
	
	//������ݳ���һ����������޸ģ�������ݳ��Ȳ�һ������Ҫ��ɾ�������
	if((rtn = sdb_modify_record(db, &timeTaskid, (check_key_f) timeTaskListCheckKeyId,rec))==false)
	{
		if(timeTaskListRemoveTaskByID(timeTaskid)!=NULL)
		{
			rtn = sdb_add_record(db, rec);
		}
	}
	
    return rtn;
}

/*********************************************************************
 * @fn      timeTaskListAddScene
 *
 * @brief   add a timeTask to the timeTask list.
 *
 * @return  timeTaskId
 */
uint8_t timeTaskListAddTask(timeTaskRecord_t *timeTask)
{
    timeTaskRecord_t *exsistingScene;
    char rec[MAX_SUPPORTED_RECORD_SIZE];
 
    exsistingScene = timeTaskListGetTaskByID(timeTask->timeTaskId);
	
	if(exsistingScene != NULL)
	{
		return exsistingScene->timeTaskId;
	}

	timeTask->timeTaskId = timeTaskListGetUnusedTaskId();

	timeTaskListComposeRecord(timeTask, rec);

	sdb_add_record(db, rec);

    return timeTask->timeTaskId;
}


/*********************************************************************
 * @fn      timeTaskListAddScene
 *
 * @brief   add a timeTask to the timeTask list.
 *
 * @return  timeTaskId
 */
 
 /*
uint8_t timeTaskListGetSceneId( char *timeTaskNameStr )
{
    uint8_t timeTaskId = 0;
    timeTaskRecord_t *timeTask;

    timeTask = timeTaskListGetSceneByName(timeTaskNameStr);

    if( timeTask == NULL)
    {
        timeTaskId = -1;
    }
    else
    {
        timeTaskId = timeTask->timeTaskId;
    }

    //printf("timeTaskListGetSceneId--\n");

    return timeTaskId;
}
*/

/*********************************************************************
 * @fn      timeTaskListGetNextScene
 *
 * @brief   Return the next timeTask in the list.
 *
 * @param   context Pointer to the current timeTask record
 *
 * @return  timeTaskRecord_t, return next timeTask record in the DB
 */
 
timeTaskRecord_t* timeTaskListGetNextTask(uint32_t *context)
{
    char * rec;
    timeTaskRecord_t *timeTask;

    do
    {
        rec = SDB_GET_NEXT_RECORD(db,context);

        if (rec == NULL)
        {
            return NULL;
        }

        timeTask = timeTaskListParseRecord(rec);
        log_debug("%s\n",rec);
    }
    while (timeTask == NULL); //in case of a bad-format record - skip it and read the next one

    return timeTask;
}

uint16_t timeTaskListGetAllTask(void)
{
	uint32_t byteCont = 16;//����ͳ��
	uint8_t timeCont =0;

	timeTaskRecord_t *timeTask;
	nodeMembersRecord_t *nodeMembers;
	
	uint32_t context = 0;

	hostCmd cmd;
	cmd.idx = 0;
	
//	*devNum=0;
	
	while((timeTask = timeTaskListGetNextTask(&context))!=NULL)
	{
		
		//(*devNum)++;
		timeCont++;
		cmdSet8bitVal(&cmd,timeTask->timeTaskId);
		cmdSet8bitVal(&cmd,timeTask->timeTaskState);
		cmdSet8bitVal(&cmd,timeTask->timerType);
		cmdSet8bitVal(&cmd,timeTask->hours);
		cmdSet8bitVal(&cmd,timeTask->minute);
		cmdSet8bitVal(&cmd,timeTask->second);
		
		cmdSet8bitVal(&cmd,timeTask->timeTaskType);


		if(timeTask->timeTaskType == EVENT_TYPE_SCENEID_ACTION)
		{
			cmdSet8bitVal(&cmd, timeTask->sceneid);
			byteCont += 8;
		}
		else
		{
			nodeMembers = timeTask->members;
			cmdSet8bitVal(&cmd,timeTask->memberscount);
			byteCont += 1;

			while(nodeMembers != NULL)
			{
				cmdSetStringVal(&cmd, (uint8_t *)nodeMembers->IEEEAddr, 8);
				cmdSet8bitVal(&cmd,nodeMembers->endpoint);
				cmdSet8bitVal(&cmd, nodeMembers->length);
				cmdSetStringVal(&cmd, (uint8_t *)nodeMembers->dataSegment,nodeMembers->length);
				nodeMembers = nodeMembers->next;
				byteCont += (9+nodeMembers->length);
			}
		}

		//���ݳ������ޣ��ְ�����
		if((byteCont+100) > MaxPacketLength)
		{
			SRPC_PeriodTimerSwitchCtrlInd(timeCont,cmd.data,cmd.idx);
			timeCont = 0;
			memset(&cmd,0,sizeof(hostCmd));
			cmd.idx = 0;
			byteCont = 16;//FC..FCS
		}
	}
	
	SRPC_PeriodTimerSwitchCtrlInd(timeCont,cmd.data,cmd.idx);
		
    return 0;
}

