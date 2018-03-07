/**************************************************************************************************
 * Filename:       db_deviceStatelist.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    存储节点设备的状态
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-02,20:10)    :   Create the file.
 *
 *************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "interface_devicelist.h"
#include "interface_deviceStatelist.h"

//#include "hal_types.h"
#include "SimpleDBTxt.h"
#include "logUtils.h"

/*********************************************************************
 * LOCAL VARIABLES
 */

static db_descriptor * statedb;

/*********************************************************************
 * TYPEDEFS
 */
#undef DEV_STATE_LOG


/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

typedef struct
{
    uint16_t nwkAddr;
    uint8_t endpoint;
} devState_key_NA_EP;

typedef struct
{
    uint8_t ieeeAddr[8];
    uint8_t endpoint;
} devState_key_IEEE_EP;

typedef uint8_t devState_key_IEEE[8];


/************************************************************************
* 函数名 :devStateListComposeRecord(epInfo_t *epInfo, uint8_t byteNum,uint8_t* bytes,char * record)
* 描述   :   作成设备状态数据库所需要的数据
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
static char * devStateListComposeRecord(devState_t *epState,char * record)
{
	int mlength = 0;
    //leave a space at the beginning to mark this record as deleted if needed later, or as bad format (can happen if edited manually). Another space to write the reason of bad format.
    sprintf(record,
            "	%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X , 0x%02X , 0x%04X , 0x%02X , ",
            epState->IEEEAddr[0], epState->IEEEAddr[1], epState->IEEEAddr[2],
            epState->IEEEAddr[3], epState->IEEEAddr[4], epState->IEEEAddr[5],
            epState->IEEEAddr[6], epState->IEEEAddr[7], epState->endpoint,
            epState->deviceID,epState->length);

	for(mlength=0; mlength<epState->length-1; mlength++)
	{
		sprintf(record+strlen(record)," 0x%02X ,",epState->dataSegment[mlength]);
	}

	sprintf(record+strlen(record)," 0x%02X\n",epState->dataSegment[mlength]);		

    return record;
}

/************************************************************************
* 函数名 :devStateListAddDevice(epInfo_t *epInfo)
* 描述   :   添加一条设备记录信息到数据库
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void devStateListAddDevice(devState_t *epState)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};
	devState_t *exsistingState;
	log_debug("devStateListAddDevice++\n");
	exsistingState = devStateListGetRecordByIeeeEp(epState->IEEEAddr,epState->endpoint);

	if(exsistingState != NULL)
		return;
	
    devStateListComposeRecord(epState,rec);

    sdb_add_record(statedb, rec);
	log_debug("devStateListAddDevice--\n");
}

static devState_t * devStateListParseRecord(char * record)
{
    char * pBuf = record + 1; //+1 is to ignore the 'for deletion' mark that may just be added to this record.
    static devState_t epState;
    parsingResult_t parsingResult = { SDB_TXT_PARSER_RESULT_OK, 0 };

	uint8_t mCount = 0;
    if (record == NULL)
    {
        return NULL;
    }
    
	memset(&epState,0,sizeof(devState_t));
	
    sdb_txt_parser_get_hex_field(&pBuf, epState.IEEEAddr, 8, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, &epState.endpoint, 1, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &epState.deviceID, 2, false, &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, &epState.length, 1, false, &parsingResult);

#ifdef DEV_STATE_LOG
    printf("endpoint %02x,deviceID %04x\n,",epState.endpoint,epState.deviceID);
#endif

	for(mCount=0; mCount<epState.length; mCount++)
    {
        sdb_txt_parser_get_numeric_field(&pBuf, &epState.dataSegment[mCount], 1, false, &parsingResult);
    }

    if ((parsingResult.code != SDB_TXT_PARSER_RESULT_OK)
        && (parsingResult.code != SDB_TXT_PARSER_RESULT_REACHED_END_OF_RECORD))
    {
        sdbtMarkError(statedb, record, &parsingResult);
        return NULL;
    }

    return &epState;
}

static int devStateListCheckKeyIeeeEp(char * record, devState_key_IEEE_EP * key)
{
    devState_t *epState;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epState = devStateListParseRecord(record);
    if (epState == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if ((memcmp(epState->IEEEAddr, key->ieeeAddr, Z_EXTADDR_LEN) == 0)
        && (epState->endpoint == key->endpoint))
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int devStateListCheckKeyIeee(char * record, uint8_t key[Z_EXTADDR_LEN])
{
    devState_t *epState;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epState = devStateListParseRecord(record);
    if (epState == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }
    //printf("Comparing 0x%08X%08X to 0x%08X%08X\n", *(uint32_t *)((epInfo->IEEEAddr)+4), *(uint32_t *)((epInfo->IEEEAddr)+0), *(uint32_t *)(key+4), *(uint32_t *)(key+0));
    if (memcmp(epState->IEEEAddr, key, Z_EXTADDR_LEN) == 0)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int devStateListCheckKeyAll(char * record, uint8_t endpoint)
{
    devState_t *epState;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epState = devStateListParseRecord(record);

    if (epState == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }
    else
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}


devState_t * devStateListRemoveDeviceByIeeeEp(uint8_t ieeeAddr[8],uint8_t endpoint)
{
    devState_key_IEEE_EP key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;

    return devStateListParseRecord(sdb_delete_record(statedb, &key, (check_key_f) devStateListCheckKeyIeeeEp));
}


devState_t * devStateListRemoveDeviceByIeee(uint8_t ieeeAddr[8])
{
    return devStateListParseRecord(sdb_delete_record(statedb, ieeeAddr, (check_key_f) devStateListCheckKeyIeee));
}


devState_t * devStateListRemoveAllDevice(void)
{
    uint32_t context = 0;
    while(devStateListGetNextDev(&context) != NULL)
    {
        devStateListParseRecord(sdb_delete_record(statedb, 0x00, (check_key_f)devStateListCheckKeyAll));
    }

    return NULL;
}

/************************************************************************
* 函数名 :devStateListGetRecordByIeeeEp(uint8_t ieeeAddr[8], uint8_t endpoint,uint8_t* byteNum,uint8_t *bytes)
* 描述   :    根据ieeeAddr和endpoint来查找数据库的记录
* 输入   ：无
* 输出   ：无
* 返回   ：数据记录数量
************************************************************************/
devState_t * devStateListGetRecordByIeeeEp(uint8_t ieeeAddr[8], uint8_t endpoint)
{
    char * rec;
    devState_key_IEEE_EP key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;

    rec = SDB_GET_UNIQUE_RECORD(statedb, &key, (check_key_f)devStateListCheckKeyIeeeEp);
    if (rec == NULL)
    {
#ifdef DEV_STATE_LOG
        printf("rec == NULL\n,");
#endif
        return NULL;
    }

    return devStateListParseRecord(rec);
}

/************************************************************************
* 函数名 :devStateListModifyRecordByNaEp(uint16_t nwkAddr, uint8_t endpoint,epInfo_t * epInfo)
* 描述   :    通过nwkAddr 和endpoint来修改数据库内的记录信息,
*                如果没有记录，则添加一条新的记录。
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
bool devStateListModifyRecordByIeeeEp(devState_t * epState)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};

    devState_key_IEEE_EP key;
    memcpy(key.ieeeAddr, epState->IEEEAddr, 8);
    key.endpoint = epState->endpoint;

    devStateListComposeRecord(epState, rec);

	bool state = sdb_modify_record(statedb, &key, (check_key_f) devStateListCheckKeyIeeeEp,rec);

	log_debug("state = %d\n",state);

	if( state == false)
    {	
        devStateListAddDevice(epState);
    }

    return true;
}

/************************************************************************
* 函数名 :devStateListNumDevices(void)
* 描述   :    获取数据库的数据数量
* 输入   ：无
* 输出   ：无
* 返回   ：数据记录数量
************************************************************************/
uint32_t devStateListNumDevices(void)
{
    return sdbtGetRecordCount(statedb);
}

devState_t * devStateListGetNextDev(uint32_t *context)
{
    char * rec;
    devState_t *epState;

    do
    {
        rec = SDB_GET_NEXT_RECORD(statedb,context);

        if (rec == NULL)
        {
            return NULL;
        }

        epState = devStateListParseRecord(rec);
    }
    while (epState == NULL); //in case of a bad-format record - skip it and read the next one

    return epState;
}

void devStateListInitDatabase(char * dbFilename)
{
    statedb = sdb_init_db(dbFilename, sdbtGetRecordSize, sdbtCheckDeleted,
                          sdbtCheckIgnored, sdbtMarkDeleted,
                          (consolidation_processing_f) sdbtErrorComment, SDB_TYPE_TEXT, 0);

    log_debug("devStateListInitDatabase %s\n", dbFilename);

    sdb_consolidate_db(&statedb);
}

void devStateListrelaseDatabase(void)
{
	sdb_release_db(&statedb);
}

