/**************************************************************************************************
 * Filename:       interface_devicelist.c
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-11-23,13:00)    :   Create the file.
 *                     1.01  (2014-12-02,13:53)    :
 *    增加了新的接口devListModifyRecordByNaEp，用于修改数据库中的存储数据。
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

#include "logUtils.h"
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

typedef struct
{
	uint8_t  ieeeAddr[8];
	uint16_t deviceID;
}dev_key_IEEE_DeviceID;

//typedef uint8_t dev_key_IEEE[8];

static char * devListComposeRecord(epInfo_t *epInfo, char * record)
{
    uint8_t hasName=0;

    if(epInfo->deviceName != NULL)
    {
        hasName = 1;
    }

    //leave a space at the beginning to mark this record as deleted if needed later, or as bad format (can happen if edited manually). Another space to write the reason of bad format.
    sprintf(record,
            " %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X , 0x%04X , 0x%02X , 0x%04X , 0x%04X , 0x%02X , 0x%02X , 0x%02X , 0x%02X , 0x%04X , \"%s\"\n",
            epInfo->IEEEAddr[0], epInfo->IEEEAddr[1], epInfo->IEEEAddr[2],
            epInfo->IEEEAddr[3], epInfo->IEEEAddr[4], epInfo->IEEEAddr[5],
            epInfo->IEEEAddr[6], epInfo->IEEEAddr[7], epInfo->nwkAddr,
            epInfo->endpoint, epInfo->profileID, epInfo->deviceID, epInfo->version,0/*epInfo->onlineDevRssi*/,
            epInfo->registerflag, epInfo->onlineflag,0/*epInfo->onlineTimeoutCounter*/, hasName ? epInfo->deviceName : "");
		log_debug("%s\n",record);
	
    return record;
}

/************************************************************************
* 函数名 :devListAddDevice(epInfo_t *epInfo)
* 描述   :   添加一条设备记录信息到数据库
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
void devListAddDevice(epInfo_t *epInfo)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};

    devListComposeRecord(epInfo, rec);

    if(sdb_add_record(db, rec)!=false)
    {
    	if(epInfo->deviceID != 0x0 && epInfo->endpoint != 0x00)
			vdevListAddDevice(epInfo);//添加到设备列表中
	}
}

static epInfo_t * devListParseRecord(char * record)
{
    char * pBuf = record + 1; //+1 is to ignore the 'for deletion' mark that may just be added to this record.
    static epInfo_t epInfo;
    static char deviceName[MAX_SUPPORTED_DEVICE_NAME_LENGTH + 1];
    parsingResult_t parsingResult = { SDB_TXT_PARSER_RESULT_OK, 0 };

    if (record == NULL)
    {
        return NULL;
    }

//	log_debug("%s\n",record);
	
//	memset(&epInfo,0,sizeof(epInfo_t));
	
    sdb_txt_parser_get_hex_field(&pBuf, epInfo.IEEEAddr, 8, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &epInfo.nwkAddr, 2, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, &epInfo.endpoint, 1, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &epInfo.profileID, 2, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &epInfo.deviceID, 2, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, &epInfo.version, 1, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, &epInfo.onlineDevRssi, 1, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, &epInfo.registerflag, 1, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, &epInfo.onlineflag, 1, false, &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &epInfo.onlineTimeoutCounter, 2, false, &parsingResult);
    sdb_txt_parser_get_quoted_string(&pBuf, deviceName, MAX_SUPPORTED_DEVICE_NAME_LENGTH, &parsingResult);

    if ((parsingResult.code != SDB_TXT_PARSER_RESULT_OK)
        && (parsingResult.code != SDB_TXT_PARSER_RESULT_REACHED_END_OF_RECORD))
    {
        sdbtMarkError(db, record, &parsingResult);
        return NULL;
    }

    if (strlen(deviceName) > 0)
    {
        epInfo.deviceName = deviceName;
    }
    else
    {
        epInfo.deviceName = NULL;
    }

    return &epInfo;
}

static int devListCheckKeyIeeeEp(char * record, dev_key_IEEE_EP * key)
{
    epInfo_t * epInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epInfo = devListParseRecord(record);
    if (epInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if ((memcmp(epInfo->IEEEAddr, key->ieeeAddr, Z_EXTADDR_LEN) == 0)
        && ((epInfo->endpoint == key->endpoint)||(key->endpoint == 0XFF)))
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int devListCheckKeyIeeeDeviceID(char * record, dev_key_IEEE_DeviceID * key)
{
    epInfo_t * epInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epInfo = devListParseRecord(record);
    if (epInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if ((memcmp(epInfo->IEEEAddr, key->ieeeAddr, Z_EXTADDR_LEN) == 0)
        && ((epInfo->deviceID== key->deviceID)))
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}


static int devListCheckKeyIeee(char * record, uint8_t key[Z_EXTADDR_LEN])
{
    epInfo_t * epInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epInfo = devListParseRecord(record);
    if (epInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }
	
    //printf("Comparing 0x%08X%08X to 0x%08X%08X\n", *(uint32_t *)((epInfo->IEEEAddr)+4), *(uint32_t *)((epInfo->IEEEAddr)+0), *(uint32_t *)(key+4), *(uint32_t *)(key+0));
    if (memcmp(epInfo->IEEEAddr, key, Z_EXTADDR_LEN) == 0)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int devListCheckKeyNaEp(char * record, dev_key_NA_EP * key)
{
    epInfo_t * epInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epInfo = devListParseRecord(record);
    if (epInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    //printf("epInfo->nwkAddr = %04x key->nwkAddr= %04x\n",epInfo->nwkAddr,key->nwkAddr);

    if ((epInfo->nwkAddr == key->nwkAddr) && (( key->endpoint == 0xFF) || (epInfo->endpoint == key->endpoint)) )
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}


static int devListCheckKeyAll(char * record, uint8_t endpoint)
{
    epInfo_t * epInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epInfo = devListParseRecord(record);

    if (epInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }
    else
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int devListCheckKeyEp(char * record, uint8_t endpoint)
{
    epInfo_t * epInfo;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    epInfo = devListParseRecord(record);
    if (epInfo == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if ( (epInfo->endpoint == endpoint) )
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

epInfo_t * devListRemoveDeviceByNaEp(uint16_t nwkAddr, uint8_t endpoint)
{
    dev_key_NA_EP key = { nwkAddr, endpoint};

    return devListParseRecord(sdb_delete_record(db, &key, (check_key_f) devListCheckKeyNaEp));

}

epInfo_t * devListRemoveDeviceByIeeeEp(uint8_t ieeeAddr[8],uint8_t endpoint)
{
    dev_key_IEEE_EP key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;

    return devListParseRecord(sdb_delete_record(db, &key, (check_key_f) devListCheckKeyIeeeEp));
}


epInfo_t * devListRemoveDeviceByIeee(uint8_t ieeeAddr[8])
{
    return devListParseRecord(sdb_delete_record(db, ieeeAddr, (check_key_f) devListCheckKeyIeee));
}

/*
epInfo_t * devListRemoveAllDevice()
{
    return devListParseRecord(
            sdb_delete_record(db, 0x00, (check_key_f) devListCheckKeyAll));
}
*/

epInfo_t * devListRemoveAllDevice(void)
{
    uint32_t context = 0;
    while(devListGetNextDev(&context) != NULL)
    {
        devListParseRecord(sdb_delete_record(db, 0x00, (check_key_f)devListCheckKeyAll));
    }

    return NULL;
}

epInfo_t * devListGetDeviceByIeeeEp(uint8_t ieeeAddr[8], uint8_t endpoint)
{
    char * rec;
    dev_key_IEEE_EP key;
	log_debug("devListGetDeviceByIeeeEp++\n");

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;
    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)devListCheckKeyIeeeEp);
    if (rec == NULL)
    {
        return NULL;
    }
    log_debug("%s",rec);
	log_debug("devListGetDeviceByIeeeEp--\n");
    return devListParseRecord(rec);
}

epInfo_t * devListGetDeviceByIeeeDeviceID(uint8_t ieeeAddr[8], uint16_t deviceid)
{
    char * rec;
    dev_key_IEEE_DeviceID key;
	log_debug("devListGetDeviceByIeeeEp++\n");

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.deviceID= deviceid;
    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)devListCheckKeyIeeeDeviceID);
    if (rec == NULL)
    {
        return NULL;
    }
    log_debug("%s",rec);
	log_debug("devListGetDeviceByIeeeEp--\n");
    return devListParseRecord(rec);
}

epInfo_t * devListGetDeviceByNaEp(uint16_t nwkAddr, uint8_t endpoint)
{
    char * rec;
    dev_key_NA_EP key;

    key.nwkAddr = nwkAddr;
    key.endpoint = endpoint;
    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)devListCheckKeyNaEp);

    if (rec == NULL)
    {
        return NULL;
    }
    
    log_debug("%s",rec);
//	printf("rec = %s \n",rec);

    return devListParseRecord(rec);
}

/************************************************************************
* 函数名 :devListModifyRecordByIeeeEp(uint8_t* ieeeAddr, uint8_t endpoint,epInfo_t * epInfo)
* 描述   :    通过ieeeAddr 和endpoint来修改数据库内的记录信息
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
bool devListModifyRecordByIeeeEp(uint8_t* ieeeAddr, uint8_t endpoint,epInfo_t * epInfo)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};

    dev_key_IEEE_EP key;

    memcpy(key.ieeeAddr, ieeeAddr, 8);
    key.endpoint = endpoint;

    devListComposeRecord(epInfo, rec);

    //printf("devListModifyRecordByIeeeEp : rec = %s \n",rec);

    return sdb_modify_record(db, &key, (check_key_f) devListCheckKeyIeeeEp,rec);

}

/************************************************************************
* 函数名 :devListModifyRecordByNaEp(uint16_t nwkAddr, uint8_t endpoint,epInfo_t * epInfo)
* 描述   :    通过nwkAddr 和endpoint来修改数据库内的记录信息
* 输入   ：
* 输出   ：无
* 返回   ：0:处理成功
************************************************************************/
bool devListModifyRecordByNaEp(uint16_t nwkAddr, uint8_t endpoint,epInfo_t * epInfo)
{
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};

    dev_key_NA_EP key = { nwkAddr, endpoint};
	log_debug("devListModifyRecordByNaEp++\n");
    devListComposeRecord(epInfo, rec);
#ifdef DEBUG
	printf("\n      IEEE	           nwkAddr endpoint profileID deviceID Version RSSI Regflag onFlag otime name\n");
	printf("%s\n",rec);
#endif
    return sdb_modify_record(db, &key, (check_key_f) devListCheckKeyNaEp,rec);
}

/************************************************************************
* 函数名 :devListNumDevices(void)
* 描述   :    获取数据库的数据数量
* 输入   ：无
* 输出   ：无
* 返回   ：数据记录数量
************************************************************************/
uint32_t devListNumDevices(void)
{
    return sdbtGetRecordCount(db);
}

epInfo_t * devListGetNextDev(uint32_t *context)
{
    char * rec;
    epInfo_t *epInfo;

    do
    {
        rec = SDB_GET_NEXT_RECORD(db,context);

        if (rec == NULL)
        {
            return NULL;
        }
		
//		log_debug("%s\n",rec);
		
        epInfo = devListParseRecord(rec);
    }
    while (epInfo == NULL); //in case of a bad-format record - skip it and read the next one

    return epInfo;
}

/************************************************************************
* 函数名 :devListInitDatabase(char * dbFilename)
* 描述   :    初始化数据库
* 输入   ：无
* 输出   ：无
* 返回   ：无
************************************************************************/
void devListInitDatabase(char * dbFilename)
{
    db = sdb_init_db(dbFilename, sdbtGetRecordSize, sdbtCheckDeleted,
                     sdbtCheckIgnored, sdbtMarkDeleted,
                     (consolidation_processing_f) sdbtErrorComment, SDB_TYPE_TEXT, 0);

    log_debug("devListInitDatabase %s\n", dbFilename);

    sdb_consolidate_db(&db);
}

void devListrelaseDatabase(void)
{
	sdb_release_db(&db);
}
