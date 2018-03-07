/**************************************************************************************************
 * Filename:       interface_devicelist.c
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
#include <unistd.h>

//#include "interface_srpcserver.h"
#include "interface_tasklist.h"

#include "hal_types.h"
#include "SimpleDBTxt.h"

/*********************************************************************
 * LOCAL VARIABLES
 */

static db_descriptor * db;

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */


static char * taskListComposeRecord(taskInfo_t *taskInfo, char * record)
{
    sprintf(record,
            "        0x%04X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X \n", //leave a space at the beginning to mark this record as deleted if needed later, or as bad format (can happen if edited manually). Another space to write the reason of bad format.
            taskInfo->nwkAddr, taskInfo->endpoint, taskInfo->taskid,
            taskInfo->alarmMode, taskInfo->alarmHours, taskInfo->alarmMinute,
            taskInfo->alarmSecond,taskInfo->taskType,taskInfo->taskdata1,taskInfo->taskdata2);

    return record;
}


taskInfo_t * taskListAddTask(taskInfo_t *taskInfo,uint16_t dstAddr,uint8_t endpoint,uint8_t alarmMode,
                             uint8_t alarmHours,uint8_t alarmMinute,uint8_t alarmSecond,uint8_t taskType,uint8_t taskdata1,uint8_t taskdata2)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE];

    printf("taskListAddTask++:  \n");

    taskInfo->taskid= taskListGetUnusedTaskId();
    taskInfo->nwkAddr= dstAddr;
    taskInfo->endpoint= endpoint;
    taskInfo->alarmMode= alarmMode;
    taskInfo->alarmHours= alarmHours;
    taskInfo->alarmMinute= alarmMinute;
    taskInfo->alarmSecond= alarmSecond;
    taskInfo->taskType= taskType;
    taskInfo->taskdata1= taskdata1;
    taskInfo->taskdata2= taskdata2;

    taskListComposeRecord(taskInfo, rec);

    sdb_add_record(db, rec);

    return taskInfo ;
}

static taskInfo_t * taskListParseRecord(char * record)
{
    char * pBuf = record + 1; //+1 is to ignore the 'for deletion' mark that may just be added to this record.
    static taskInfo_t taskInfo;

    parsingResult_t parsingResult = { SDB_TXT_PARSER_RESULT_OK, 0 };

    if (record == NULL)
    {
        return NULL;
    }

    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.nwkAddr, 2, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, &taskInfo.endpoint, 1, false,&parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.taskid, 1, false,&parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmMode, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmHours, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmMinute, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmSecond, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.taskType, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.taskdata1, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.taskdata2, 1,false, &parsingResult);

    if ((parsingResult.code != SDB_TXT_PARSER_RESULT_OK)
        && (parsingResult.code != SDB_TXT_PARSER_RESULT_REACHED_END_OF_RECORD))
    {
        sdbtMarkError(db, record, &parsingResult);
        return NULL;
    }

    return &taskInfo;
}

static int taskListCheckKeyId(char * record, uint8_t * key)
{
    taskInfo_t * taskInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    taskInfo = taskListParseRecord(record);
    if (taskInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if (taskInfo->taskid == *key)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int taskListCheckAll(char * record, uint8_t * key)
{
    taskInfo_t * taskInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    taskInfo = taskListParseRecord(record);
    if (taskInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    else
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}


taskInfo_t * taskListGetTaskByID(uint8_t taskid)
{
    char * rec;

    rec = SDB_GET_UNIQUE_RECORD(db, &taskid, (check_key_f)taskListCheckKeyId);
    if (rec == NULL)
    {
        return NULL;
    }

    return taskListParseRecord(rec);
}

uint8_t taskListGetUnusedTaskId(void)
{

    printf("taskListGetUnusedTaskId++:	\n");

    static uint8_t lastUsedTaskId = 0;

    lastUsedTaskId++;

    while (SDB_GET_UNIQUE_RECORD(db, &lastUsedTaskId, (check_key_f)taskListCheckKeyId)!= NULL)
    {
        lastUsedTaskId++;
    }

    return lastUsedTaskId;
}



taskInfo_t * taskListRemovetaskByID(uint8_t taskid)
{
    return taskListParseRecord(sdb_delete_record(db, &taskid, (check_key_f) taskListCheckKeyId));
}

taskInfo_t * taskListRemoveAlltask()
{

    uint32_t context = 0;
    while(taskListGetNextTask(&context) != NULL)
    {
        taskListParseRecord(sdb_delete_record(db, 0x00, (check_key_f) taskListCheckAll));
    }

    return NULL;

}


uint32_t taskListNumTasks(void)
{
    return sdbtGetRecordCount(db);
}

taskInfo_t * taskListGetNextTask(uint32_t *context)
{
    char * rec;
    taskInfo_t *taskInfo;

    do
    {
        rec = SDB_GET_NEXT_RECORD(db,context);

        if (rec == NULL)
        {
            return NULL;
        }

        taskInfo = taskListParseRecord(rec);
    }
    while (taskInfo == NULL); //in case of a bad-format record - skip it and read the next one

    return taskInfo;
}

void taskListInitDatabase(char * dbFilename)
{
    db = sdb_init_db(dbFilename, sdbtGetRecordSize, sdbtCheckDeleted,
                     sdbtCheckIgnored, sdbtMarkDeleted,
                     (consolidation_processing_f) sdbtErrorComment, SDB_TYPE_TEXT, 0);
    sdb_consolidate_db(&db);
}

