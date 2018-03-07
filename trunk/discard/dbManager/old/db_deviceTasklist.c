/**************************************************************************************************
 * Filename:       db_deviceTasklist.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    存储待处理的任务，如定时器任务
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-10,13:57)    :   Create the file.
 *
 *************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include "interface_srpcserver.h"
#include "db_deviceTasklist.h"

#include "hal_types.h"
#include "SimpleDBTxt.h"

/*********************************************************************
 * LOCAL VARIABLES
 */

static db_descriptor * taskdb;

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */


/************************************************************************
* 函数名 :devTaskListComposeRecord(devTaskInfo_t *taskInfo, uint8 byteNum,uint8* bytes,char * record)
* 描述   :   作成设备状态数据库所需要的数据
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static char * devTaskListComposeRecord(devTaskInfo_t *taskInfo, uint8 byteNum,uint8* bytes,char * record)
{
    char* tmp_record = record;
    uint8 valNum;

    sprintf(record,
            " 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%02X ,",
            taskInfo->taskid, taskInfo->taskType,  taskInfo->alarmType, taskInfo->alarmDay,
            taskInfo->alarmWeek,taskInfo->alarmHours,taskInfo->alarmMinute,taskInfo->alarmSecond,
            byteNum);

    tmp_record = record+strlen(record);

    //需要存储的数据按字节存储
    for(valNum=0; valNum<byteNum-1; valNum++)
    {
        sprintf(tmp_record," 0x%02X ,",bytes[valNum]);
        tmp_record = record+strlen(record);
    }

    //需要存储的最后一个数据格式不同，单独处理
    sprintf(tmp_record," 0x%02X \n",bytes[valNum]);

    return record;
}


/************************************************************************
* 函数名 :devTaskListAddTask(devTaskInfo_t *taskInfo,uint8 byteNum,uint8* bytes)
* 描述   :   添加记录到数据库
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
devTaskInfo_t * devTaskListAddTask(devTaskInfo_t *taskInfo,uint8 byteNum,uint8* bytes)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE];

    printf("devTaskListAddTask++:  \n");

    devTaskListComposeRecord(taskInfo, byteNum, bytes, rec);

    sdb_add_record(taskdb, rec);

    return taskInfo ;
}

/************************************************************************
* 函数名 :devTaskListParseRecord(char * record,uint8 *byteNum,uint8 *bytes)
* 描述   :   解析数据库的记录
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static devTaskInfo_t * devTaskListParseRecord(char * record,uint8 *byteNum,uint8 *bytes)
{
    char * pBuf = record + 1; //+1 is to ignore the 'for deletion' mark that may just be added to this record.
    static devTaskInfo_t taskInfo;

    parsingResult_t parsingResult = { SDB_TXT_PARSER_RESULT_OK, 0 };
    uint8 valNum;

    if (record == NULL)
    {
        return NULL;
    }

    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.taskid, 1, false,&parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.taskType, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmType, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmDay, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmWeek, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmHours, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmMinute, 1,false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &taskInfo.alarmSecond, 1,false, &parsingResult);

    if(byteNum != NULL)
    {
        sdb_txt_parser_get_numeric_field(&pBuf, byteNum, 1, false, &parsingResult);

        for(valNum=0; valNum<*byteNum; valNum++)
        {
            sdb_txt_parser_get_numeric_field(&pBuf, bytes, 1, false, &parsingResult);
            bytes++;
        }
    }

    if ((parsingResult.code != SDB_TXT_PARSER_RESULT_OK)
        && (parsingResult.code != SDB_TXT_PARSER_RESULT_REACHED_END_OF_RECORD))
    {
        sdbtMarkError(taskdb, record, &parsingResult);
        return NULL;
    }

    return &taskInfo;
}

/************************************************************************
* 函数名 :devTaskListCheckKeyId(char * record, uint8_t * key)
* 描述   :   通过Task ID来检测
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static int devTaskListCheckKeyId(char * record, uint8_t * key)
{
    devTaskInfo_t * taskInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    taskInfo = devTaskListParseRecord(record,NULL,NULL);
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


static int devTaskListCheckAll(char * record, uint8_t * key)
{
    devTaskInfo_t * taskInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    taskInfo = devTaskListParseRecord(record,NULL,NULL);
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


devTaskInfo_t * devTaskListGetTaskByID(uint8_t taskid,uint8* byteNum,uint8 *bytes)
{
    char * rec;

    rec = SDB_GET_UNIQUE_RECORD(taskdb, &taskid, (check_key_f)devTaskListCheckKeyId);
    if (rec == NULL)
    {
        return NULL;
    }

    return devTaskListParseRecord(rec,byteNum,bytes);
}

/************************************************************************
* 函数名 :devTaskListGetUnusedTaskId(void)
* 描述   :   获取未使用的Task ID
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
uint8_t devTaskListGetUnusedTaskId(void)
{

    printf("taskListGetUnusedTaskId++:	\n");

    static uint8_t lastUsedTaskId = 0;

    lastUsedTaskId++;

    while (SDB_GET_UNIQUE_RECORD(taskdb, &lastUsedTaskId, (check_key_f)devTaskListCheckKeyId)!= NULL)
    {
        lastUsedTaskId++;
    }

    return lastUsedTaskId;
}

devTaskInfo_t * devTaskListRemovetaskByID(uint8_t taskid)
{
    return devTaskListParseRecord(sdb_delete_record(taskdb, &taskid, (check_key_f) devTaskListCheckKeyId),NULL,NULL);
}

devTaskInfo_t * devTaskListRemoveAlltask()
{

    uint32_t context = 0;
    while(devTaskListGetNextTask(&context,NULL,NULL) != NULL)
    {
        devTaskListParseRecord(sdb_delete_record(taskdb, 0x00, (check_key_f) devTaskListCheckAll),NULL,NULL);
    }

    return NULL;

}


uint32_t devTaskListNumTasks(void)
{
    return sdbtGetRecordCount(taskdb);
}

/************************************************************************
* 函数名 :devTaskListGetNextTask(uint32_t *context,uint8* byteNum,uint8 *bytes)
* 描述   :   获取下一个记录信息
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
devTaskInfo_t * devTaskListGetNextTask(uint32_t *context,uint8* byteNum,uint8 *bytes)
{
    char * rec;
    devTaskInfo_t *taskInfo;

    do
    {
        rec = SDB_GET_NEXT_RECORD(taskdb,context);

        if (rec == NULL)
        {
            return NULL;
        }

        taskInfo = devTaskListParseRecord(rec,byteNum,bytes);
    }
    while (taskInfo == NULL); //in case of a bad-format record - skip it and read the next one

    return taskInfo;
}

/************************************************************************
* 函数名 :devTaskListInitDatabase(char * dbFilename)
* 描述   :   初始化Task 数据库
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void devTaskListInitDatabase(char * dbFilename)
{
    taskdb = sdb_init_db(dbFilename, sdbtGetRecordSize, sdbtCheckDeleted,
                         sdbtCheckIgnored, sdbtMarkDeleted,
                         (consolidation_processing_f) sdbtErrorComment, SDB_TYPE_TEXT, 0);

    sdb_consolidate_db(&taskdb);
}

