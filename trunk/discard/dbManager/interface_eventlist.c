/**************************************************************************************************
 * Filename:       interface_Eventlist.c
 * Author:          Edward
 * E-Mail:           ouxiangping@feixuekj.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2016-01-26,13:00)    :   Create the file.
 *************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "interface_eventlist.h"

//#include "hal_types.h"
#include "SimpleDBTxt.h"
#include "comParse.h"
#include "logUtils.h"

/*********************************************************************
 * LOCAL VARIABLES
 */

static db_descriptor * db;

/*********************************************************************
 * TYPEDEFS
 */
	//device states
#define DEVLIST_STATE_NOT_ACTIVE    0
#define DEVLIST_STATE_ACTIVE        1
	
#define MAX_SUPPORTED_DEVICE_NAME_LENGTH 32

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

typedef struct
{
    uint16_t nwkAddr;
    uint8_t endpoint;
} dev_key_NA_EP;

typedef struct
{
    uint8_t ieeeAddr[8];
    uint8_t endpoint;
} dev_key_IEEE_EP;

typedef struct {
	uint8_t ieeeAddr[8];
    uint8_t endpoint;
	uint8_t eventid;
}dev_key_IEEE_EP_ID;

typedef struct {
	uint8_t eventid;
}dev_key_Event_ID;
static uint8_t eventListGetUnusedEvnetId(void);


static char * eventListComposeRecord(ActionEvent_t *eventInfo, char * record)
{
	 int mlength = 0;
    //leave a space at the beginning to mark this record as deleted if needed later, or as bad format (can happen if edited manually). Another space to write the reason of bad format.

	sprintf(record,
		" 0x%02X , 0x%02X , 0x%02X , %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X , 0x%02X , 0x%02X ,",
		eventInfo->ActionEventID,eventInfo->EventState,eventInfo->type,
		eventInfo->Condition.IEEEAddr[0], eventInfo->Condition.IEEEAddr[1], eventInfo->Condition.IEEEAddr[2],
		eventInfo->Condition.IEEEAddr[3], eventInfo->Condition.IEEEAddr[4], eventInfo->Condition.IEEEAddr[5],
		eventInfo->Condition.IEEEAddr[6], eventInfo->Condition.IEEEAddr[7], 
		eventInfo->Condition.endpoint,eventInfo->Condition.length
	);

    for(mlength=0; mlength<eventInfo->Condition.length; mlength++)
    {
        sprintf(record+strlen(record)," 0x%02X ,",eventInfo->Condition.dataSegment[mlength]);
    }

	if(eventInfo->type == EVENT_TYPE_NODE_ACTION)
	{

		sprintf(record+strlen(record)," 0x%02X ,",eventInfo->membersCount);

		Event_t *nodeMembers  = eventInfo->members;
		while(nodeMembers!=NULL)
		{
			sprintf(record+strlen(record),
            " %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X , 0x%02X , 0x%02X ,",
	           nodeMembers->IEEEAddr[0],nodeMembers->IEEEAddr[1],nodeMembers->IEEEAddr[2],
	           nodeMembers->IEEEAddr[3],nodeMembers->IEEEAddr[4],nodeMembers->IEEEAddr[5],
	           nodeMembers->IEEEAddr[6],nodeMembers->IEEEAddr[7],nodeMembers->endpoint,
			   nodeMembers->length
			);

			//需要存储的数据按字节存储
			for(mlength=0; mlength<nodeMembers->length-1; mlength++)
			{
			    sprintf(record+strlen(record)," 0x%02X ,",nodeMembers->dataSegment[mlength]);
			}

			if(nodeMembers->next==NULL)
			{
		    	//需要存储的最后一个数据格式不同，单独处理
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
		sprintf( record+strlen(record)," 0x%02X\n",eventInfo->sceneid);	
	}
	
    return record;
}

/************************************************************************
* 函数名 :eventListAddDevice(ActionEvent_t *eventInfo)
* 描述   :   添加一条设备记录信息到数据库
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
uint8_t eventListAddDevice(ActionEvent_t *EventInfo)
{
	ActionEvent_t *exsistingScene;
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};
	log_debug("eventListAddDevice++\n");

	exsistingScene = eventListGetDeviceByIeeeEpID(EventInfo->Condition.IEEEAddr,EventInfo->Condition.endpoint,EventInfo->ActionEventID);

	if(exsistingScene != NULL)
	{
		return exsistingScene->ActionEventID;
	}

	if(EventInfo->ActionEventID == 0x0)
		EventInfo->ActionEventID = eventListGetUnusedEvnetId();
	
    eventListComposeRecord(EventInfo, rec);

    sdb_add_record(db, rec);
	log_debug("eventListAddDevice--\n");

	return EventInfo->ActionEventID;
}

static ActionEvent_t * eventListParseRecord(char * record)
{
    char * pBuf = record + 1; //+1 is to ignore the 'for deletion' mark that may just be added to this record.
	uint8_t i = 0;
    static ActionEvent_t eventInfo;
    static Event_t member[MAX_SUPPORTED_NODE_MEMBERS];
    Event_t ** nextMemberPtr;

    parsingResult_t parsingResult = { SDB_TXT_PARSER_RESULT_OK, 0 };

	uint8_t mCount = 0;
	
    if (record == NULL)
    {
        return NULL;
    }
    
	memset(&eventInfo,0,sizeof(ActionEvent_t));
	
	sdb_txt_parser_get_numeric_field(&pBuf, &eventInfo.ActionEventID, 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &eventInfo.EventState, 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &eventInfo.type, 1, false, &parsingResult);
	
    sdb_txt_parser_get_hex_field(&pBuf,  eventInfo.Condition.IEEEAddr, 8, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, &eventInfo.Condition.endpoint, 1, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &eventInfo.Condition.length, 1, false, &parsingResult);

	for(mCount=0; mCount<eventInfo.Condition.length; mCount++)
    {
        sdb_txt_parser_get_numeric_field(&pBuf, &eventInfo.Condition.dataSegment[mCount], 1, false, &parsingResult);
    }
	
	//解析节点内容
	if(eventInfo.type == EVENT_TYPE_NODE_ACTION)
	{
		sdb_txt_parser_get_numeric_field(&pBuf, &eventInfo.membersCount, 1, false, &parsingResult);

		nextMemberPtr = &eventInfo.members;

		for (i = 0;(parsingResult.code == SDB_TXT_PARSER_RESULT_OK) && (i < MAX_SUPPORTED_NODE_MEMBERS); i++)
		{
			*nextMemberPtr = &(member[i]);

			sdb_txt_parser_get_hex_field(&pBuf,member[i].IEEEAddr, 8, &parsingResult);
			sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *)&member[i].endpoint, 1, false, &parsingResult);
			sdb_txt_parser_get_numeric_field(&pBuf,(uint8_t *)&member[i].length, 1, false, &parsingResult);

			for(mCount=0; mCount<member[i].length; mCount++)
			{
			    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *)&member[i].dataSegment[mCount], 1, false, &parsingResult);
			}

			nextMemberPtr = &(member[i].next);
		}
		*nextMemberPtr = NULL;
		
	}
	else
	{
		sdb_txt_parser_get_numeric_field(&pBuf, &eventInfo.sceneid, 1, false, &parsingResult);
	}

    if ((parsingResult.code != SDB_TXT_PARSER_RESULT_OK)
        && (parsingResult.code != SDB_TXT_PARSER_RESULT_REACHED_END_OF_RECORD))
    {
        sdbtMarkError(db, record, &parsingResult);
        return NULL;
    }
	
    return &eventInfo;
}

static int eventListCheckKeyIeeeEp(char * record, dev_key_IEEE_EP * key)
{
	ActionEvent_t *eventInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    eventInfo = eventListParseRecord(record);
    if (eventInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if ((memcmp(eventInfo->Condition.IEEEAddr, key->ieeeAddr, Z_EXTADDR_LEN) == 0)
        && (eventInfo->Condition.endpoint == key->endpoint))
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int eventListCheckKeyIeeeEpID(char * record, dev_key_IEEE_EP_ID * key)
{
	ActionEvent_t *eventInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    eventInfo = eventListParseRecord(record);
    if (eventInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if ((memcmp(eventInfo->Condition.IEEEAddr, key->ieeeAddr, Z_EXTADDR_LEN) == 0)
        && (eventInfo->Condition.endpoint == key->endpoint) && (eventInfo->ActionEventID == key->eventid))
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int eventListCheckKeyIeee(char * record, uint8_t key[Z_EXTADDR_LEN])
{
    ActionEvent_t * eventInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    eventInfo = eventListParseRecord(record);
    if (eventInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }
	
    //printf("Comparing 0x%08X%08X to 0x%08X%08X\n", *(uint32_t *)((eventInfo->IEEEAddr)+4), *(uint32_t *)((eventInfo->IEEEAddr)+0), *(uint32_t *)(key+4), *(uint32_t *)(key+0));
    if (memcmp(eventInfo->Condition.IEEEAddr, key, Z_EXTADDR_LEN) == 0)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int eventListCheckKeyId(char * record, dev_key_Event_ID * key)
{
	ActionEvent_t * eventInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    eventInfo = eventListParseRecord(record);
    if (eventInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if (eventInfo->ActionEventID== key->eventid)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

/*
static int eventListCheckKeyNaEp(char * record, dev_key_NA_EP * key)
{
    ActionEvent_t * eventInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    eventInfo = eventListParseRecord(record);
    if (eventInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    //printf("eventInfo->nwkAddr = %04x key->nwkAddr= %04x\n",eventInfo->nwkAddr,key->nwkAddr);

    if ((eventInfo->nwkAddr == key->nwkAddr) && (( key->endpoint == 0xFF) || (eventInfo->endpoint == key->endpoint)) )
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}
*/

static int eventListCheckKeyAll(char * record, uint8_t endpoint)
{
    ActionEvent_t * eventInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    eventInfo = eventListParseRecord(record);

    if (eventInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }
    else
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}


static int eventListCheckKeyEp(char * record, uint8_t endpoint)
{
    ActionEvent_t * eventInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    eventInfo = eventListParseRecord(record);
    if (eventInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if ( (eventInfo->Condition.endpoint == endpoint) )
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

/*
ActionEvent_t * eventListRemoveDeviceByNaEp(uint16_t nwkAddr, uint8_t endpoint)
{
    dev_key_NA_EP key = { nwkAddr, endpoint};

    return eventListParseRecord(sdb_delete_record(db, &key, (check_key_f) eventListCheckKeyNaEp));

}
*/

ActionEvent_t * eventListRemoveDeviceByIeeeEp(uint8_t ieeeAddr[8],uint8_t endpoint)
{
    dev_key_IEEE_EP key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;

    return eventListParseRecord(sdb_delete_record(db, &key, (check_key_f) eventListCheckKeyIeeeEp));
}

ActionEvent_t * eventListRemoveDeviceByIeeeEpID(uint8_t ieeeAddr[8],uint8_t endpoint,uint8_t eventid)
{
    dev_key_IEEE_EP_ID key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;
	key.eventid = eventid;
    return eventListParseRecord(sdb_delete_record(db, &key, (check_key_f) eventListCheckKeyIeeeEpID));
}

ActionEvent_t * eventListRemoveDeviceByEventID(uint8_t eventid)
{
    dev_key_Event_ID key;

	key.eventid = eventid;
    return eventListParseRecord(sdb_delete_record(db, &key, (check_key_f) eventListCheckKeyId));
}


ActionEvent_t * eventListRemoveDeviceByIeee(uint8_t ieeeAddr[8])
{
    return eventListParseRecord(sdb_delete_record(db, ieeeAddr, (check_key_f) eventListCheckKeyIeee));
}

/*
ActionEvent_t * eventListRemoveAllDevice()
{
    return eventListParseRecord(
            sdb_delete_record(db, 0x00, (check_key_f) eventListCheckKeyAll));
}
*/

ActionEvent_t * eventListRemoveAllDevice(void)
{
    uint32_t context = 0;
    while(eventListGetNextDev(&context) != NULL)
    {
        eventListParseRecord(sdb_delete_record(db, 0x00, (check_key_f)eventListCheckKeyAll));
    }

    return NULL;
}

ActionEvent_t * eventListGetDeviceByIeeeEp(uint8_t ieeeAddr[8], uint8_t endpoint)
{
    char * rec;
    dev_key_IEEE_EP key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;
    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)eventListCheckKeyIeeeEp);
    if (rec == NULL)
    {
        return NULL;
    }

    return eventListParseRecord(rec);
}

static uint8_t eventListGetUnusedEvnetId(void)
{
   // static uint8_t lastUsedEventId = 0;
	static dev_key_Event_ID key;
	key.eventid++;

   // lastUsedEventId++;

    while (SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)eventListCheckKeyId)
           != NULL)
    {
       key.eventid++;;
    }

    return key.eventid;;
}


ActionEvent_t * eventListGetDeviceByIeeeEpID(uint8_t ieeeAddr[8], uint8_t endpoint,uint8_t eventid)
{
    char * rec;
    dev_key_IEEE_EP_ID key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;
	key.eventid = eventid;
    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)eventListCheckKeyIeeeEpID);
    if (rec == NULL)
    {
        return NULL;
    }

    return eventListParseRecord(rec);
}

ActionEvent_t * eventListGetDeviceByEventID(uint8_t eventid)
{
    char * rec;
    dev_key_Event_ID key;

	key.eventid = eventid;

    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)eventListCheckKeyId);
    if (rec == NULL)
    {
        return NULL;
    }

    return eventListParseRecord(rec);
}



//返回数据长度
uint16_t eventListGetAllDevice(uint8_t *devNum,uint8_t *pBuf,uint16_t bufsize)
{
	
	ActionEvent_t *evInfo;
	Event_t *nodeMembers;
	uint32_t context = 0;
	hostCmd cmd;
	cmd.idx = 0;

	*devNum=0;

	while((evInfo = eventListGetNextDev(&context))!=NULL)
	{
		if(bufsize >= cmd.idx) //防止数组越界
		{
			(*devNum)++;
			cmdSet8bitVal(&cmd,evInfo->ActionEventID);
			cmdSet8bitVal(&cmd,evInfo->EventState);
			cmdSetStringVal(&cmd, (uint8_t *)evInfo->Condition.IEEEAddr, 8);
			cmdSet8bitVal(&cmd, evInfo->Condition.endpoint);
			cmdSet8bitVal(&cmd, evInfo->Condition.length);
			cmdSetStringVal(&cmd, (uint8_t *)evInfo->Condition.dataSegment,evInfo->Condition.length);
			cmdSet8bitVal(&cmd, evInfo->type);
			if(evInfo->type == EVENT_TYPE_SCENEID_ACTION)
			{
				cmdSet8bitVal(&cmd, evInfo->sceneid);
			}
			else
			{
				nodeMembers = evInfo->members;
				cmdSet8bitVal(&cmd,evInfo->membersCount);
				while(nodeMembers != NULL)
				{
					cmdSetStringVal(&cmd, (uint8_t *)nodeMembers->IEEEAddr, 8);
					cmdSet8bitVal(&cmd,nodeMembers->endpoint);
					cmdSet8bitVal(&cmd, nodeMembers->length);
					cmdSetStringVal(&cmd, (uint8_t *)nodeMembers->dataSegment,nodeMembers->length);
					nodeMembers = nodeMembers->next;
				}
			}
		}
		usleep(1000);
	}
	
	if(bufsize >= cmd.idx)
	{
    	memcpy(pBuf,cmd.data,cmd.idx);
	}
	
    return cmd.idx;
}

/*
ActionEvent_t * eventListGetDeviceByNaEp(uint16_t nwkAddr, uint8_t endpoint)
{
    char * rec;
    dev_key_NA_EP key;

    key.nwkAddr = nwkAddr;
    key.endpoint = endpoint;
    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)eventListCheckKeyNaEp);

    //printf("rec = %s \n",rec);

    if (rec == NULL)
    {
        return NULL;
    }

    return eventListParseRecord(rec);
}
*/

/************************************************************************
* 函数名 :eventListModifyRecordByIeeeEp(uint8_t* ieeeAddr, uint8_t endpoint,ActionEvent_t * eventInfo)
* 描述   :    通过ieeeAddr 和endpoint来修改数据库内的记录信息
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
bool eventListModifyRecordByIeeeEp(uint8_t* ieeeAddr, uint8_t endpoint,ActionEvent_t * eventInfo)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};
	bool rtn = false;
    dev_key_IEEE_EP key;
	char *str = NULL;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;

   	str = eventListComposeRecord(eventInfo, rec);
    log_debug("%s\n",str);

    //如果数据长度一样，则可以修改，如果数据长度不一样则需要先删除在添加
	if((rtn = sdb_modify_record(db, &key, (check_key_f) eventListCheckKeyIeeeEp,rec))==false)
	{
		if(eventListRemoveDeviceByIeeeEp(ieeeAddr,endpoint)!=NULL)
		{
			rtn = sdb_add_record(db, rec);
		}
	}
	
    return rtn;

}

bool eventListModifyRecordByIeeeEpID(uint8_t* ieeeAddr, uint8_t endpoint,uint8_t eventid,ActionEvent_t * eventInfo)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};
	bool rtn = false;
	char *str = NULL;

    dev_key_IEEE_EP_ID key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;
	key.eventid = eventid;
	
    str = eventListComposeRecord(eventInfo, rec);
	log_debug("%s\n",str);

	//如果数据长度一样，则可以修改，如果数据长度不一样则需要先删除在添加
	if((rtn = sdb_modify_record(db, &key, (check_key_f) eventListCheckKeyIeeeEpID,rec))==false)
	{
		if(eventListRemoveDeviceByIeeeEpID(ieeeAddr,endpoint,eventid)!=NULL)
		{
			rtn = sdb_add_record(db, rec);
		}
	}
	
    return rtn;
}


bool eventListModifyRecordByEventID(uint8_t eventid,ActionEvent_t * eventInfo)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};
	bool rtn = false;
	char *str = NULL;

    dev_key_Event_ID key;

	key.eventid = eventid;
	
    str = eventListComposeRecord(eventInfo, rec);
	log_debug("%s\n",str);

	//如果数据长度一样，则可以修改，如果数据长度不一样则需要先删除在添加
	if((rtn = sdb_modify_record(db, &key, (check_key_f) eventListCheckKeyId,rec))==false)
	{
		if(eventListRemoveDeviceByEventID(eventid)!=NULL)
		{
			rtn = sdb_add_record(db, rec);
		}
	}
	
    return rtn;
}


/*
bool eventListModifyRecordByNaEp(uint16_t nwkAddr, uint8_t endpoint,ActionEvent_t * eventInfo)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};

    dev_key_NA_EP key = { nwkAddr, endpoint};
	log_debug("devListModifyRecordByNaEp++\n");
    eventListComposeRecord(eventInfo, rec);
#ifdef DEBUG
	printf("\n      IEEE	           nwkAddr endpoint profileID deviceID Version RSSI Regflag onFlag otime name\n");
	printf("%s\n",rec);
#endif
    return sdb_modify_record(db, &key, (check_key_f) eventListCheckKeyNaEp,rec);

}
*/

/************************************************************************
* 函数名 :eventListNumDevices(void)
* 描述   :    获取数据库的数据数量
* 输入   ：无
* 输出   ：无
* 返回   ：数据记录数量
************************************************************************/
uint32_t eventListNumDevices(void)
{
    return sdbtGetRecordCount(db);
}

ActionEvent_t * eventListGetNextDev(uint32_t *context)
{
    char * rec;
    ActionEvent_t *eventInfo;

    do
    {
        rec = SDB_GET_NEXT_RECORD(db,context);

        if (rec == NULL)
        {
            return NULL;
        }

        eventInfo = eventListParseRecord(rec);

        log_debug("%s\n",rec);
    }
    while (eventInfo == NULL); //in case of a bad-format record - skip it and read the next one
	
    return eventInfo;
}

/************************************************************************
* 函数名 :eventListInitDatabase(char * dbFilename)
* 描述   :    初始化数据库
* 输入   ：无
* 输出   ：无
* 返回   ：无
************************************************************************/
void eventListInitDatabase(char * dbFilename)
{
    db = sdb_init_db(dbFilename, sdbtGetRecordSize, sdbtCheckDeleted,
                     sdbtCheckIgnored, sdbtMarkDeleted,
                     (consolidation_processing_f) sdbtErrorComment, SDB_TYPE_TEXT, 0);

    log_debug("eventListInitDatabase %s\n", dbFilename);

    sdb_consolidate_db(&db);
}

void eventListrelaseDatabase(void)
{
	sdb_release_db(&db);
}

