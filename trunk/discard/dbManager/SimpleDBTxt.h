/**************************************************************************************************
* Filename:       SimpleDBTxt.h
* Author:             zxb
* E-Mail:          zxb@yystart.com
* Description:    This is a specific implemntation of a text-based db system, based on the SimpleDB module.
*
*  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
*
* Version:         1.00  (2014-11-30,10:03)    :   Create the file.
*
*
*************************************************************************/
#ifndef SIMPLE_DB_TXT_H
#define SIMPLE_DB_TXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SimpleDB.h"

#define SDBT_BAD_FORMAT_CHARACTER '@'
#define SDBT_PENDING_COMMENT_FORMAT_CHARACTER '?'
#define SDBT_DELETED_LINE_CHARACTER ';'

uint32_t sdbtGetRecordSize(void * record);
bool sdbtCheckDeleted(void * record);
bool sdbtCheckIgnored(void * record);
void sdbtMarkDeleted(void * record);
uint32_t sdbtGetRecordCount(db_descriptor * db);
bool sdbtErrorComment(db_descriptor * db, char * record);
void sdbtMarkError(db_descriptor * db, char * record, parsingResult_t * parsingResult);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLE_DB_TXT_H */

