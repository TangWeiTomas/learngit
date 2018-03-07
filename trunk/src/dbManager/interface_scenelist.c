/**************************************************************************************************
 * Filename:       interface_scenelist.c
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

#include "interface_scenelist.h"
//#include "hal_types.h"
#include "SimpleDBTxt.h"
#include "logUtils.h"

static db_descriptor * db;

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    char* name;
    uint16_t groupId;
} scene_key_NA_GID;


/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

void sceneListInitDatabase(char * dbFilename)
{
    db = sdb_init_db(dbFilename, sdbtGetRecordSize, sdbtCheckDeleted,
                     sdbtCheckIgnored, sdbtMarkDeleted,
                     (consolidation_processing_f) sdbtErrorComment, SDB_TYPE_TEXT, 0);
    sdb_consolidate_db(&db);
}

void sceneListrelaseDatabase(void)
{
	sdb_release_db(&db);
}

static char * sceneListComposeRecord(sceneRecord_t *scene, char * record)
{
    sceneMembersRecord_t *sceneMembers;
	uint8_t mlength  =0;
    sprintf(record, " 0x%02X , 0x%02X ,", //leave a space at the beginning to mark this record as deleted if needed later, or as bad format (can happen if edited manually). Another space to write the reason of bad format.
            scene->sceneId,scene->submbers);
	
    sceneMembers = scene->members;

    while (sceneMembers != NULL)
    {
		sprintf(record + strlen(record),
				" %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X , 0x%02X , 0x%02X ,",
				sceneMembers->IEEEAddr[0], sceneMembers->IEEEAddr[1], sceneMembers->IEEEAddr[2],
				sceneMembers->IEEEAddr[3], sceneMembers->IEEEAddr[4], sceneMembers->IEEEAddr[5],
				sceneMembers->IEEEAddr[6], sceneMembers->IEEEAddr[7], sceneMembers->endpoint,
				sceneMembers->length);

		for(mlength=0; mlength<((sceneMembers->length)-1); mlength++)
	    {
			sprintf(record + strlen(record)," 0x%02X ,",sceneMembers->dataSegment[mlength]);
	    }
	    
		if(sceneMembers->next==NULL)
		{
	    //需要存储的最后一个数据格式不同，单独处理
			sprintf(record + strlen(record)," 0x%02X",sceneMembers->dataSegment[mlength]);	
		}
		else
		{
			sprintf(record + strlen(record)," 0x%02X ,",sceneMembers->dataSegment[mlength]);
		}
		
		sceneMembers = sceneMembers->next;
    }

    sprintf(record + strlen(record), "\n");

    return record;
}


static sceneRecord_t * sceneListParseRecord(char * record)
{
    char * pBuf = record + 1; //+1 is to ignore the 'for deletion' mark that may just be added to this record.
    static sceneRecord_t scene;
    static sceneMembersRecord_t member[MAX_SUPPORTED_NODE_MEMBERS];
    sceneMembersRecord_t ** nextMemberPtr;
    parsingResult_t parsingResult =
    { SDB_TXT_PARSER_RESULT_OK, 0 };
    int i,mCount;
	
    if (record == NULL)
    {
        return NULL;
    }
	
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &scene.sceneId, 1, false,
                                     &parsingResult);
	sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &scene.submbers, 1, false,
                                     &parsingResult);
    nextMemberPtr = &scene.members;
    for (i = 0;
         (parsingResult.code == SDB_TXT_PARSER_RESULT_OK)
         && (i < MAX_SUPPORTED_NODE_MEMBERS) &&(i < scene.submbers); i++)
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

    if ((parsingResult.code != SDB_TXT_PARSER_RESULT_OK)
        && (parsingResult.code != SDB_TXT_PARSER_RESULT_REACHED_END_OF_RECORD))
    {
        sdbtMarkError(db, record, &parsingResult);
        return NULL;
    }
	
    return &scene;
}
/*
static int sceneListCheckKeyName(char * record, char * key)
{
    sceneRecord_t * scene;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    scene = sceneListParseRecord(record);
    if (scene == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if(strcmp(scene->name, key) == 0)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}
*/

static int sceneListCheckKeyId(char * record, uint8_t * key)
{
    sceneRecord_t * scene;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    scene = sceneListParseRecord(record);
    if (scene == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    if (scene->sceneId == *key)
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

static int sceneListCheckAll(char * record, uint8_t * key)
{
    sceneRecord_t * scene;
    int result = SDB_CHECK_KEY_NOT_EQUAL;

    scene = sceneListParseRecord(record);
    if (scene == NULL)
    {
        return SDB_CHECK_KEY_ERROR;
    }

    else
    {
        result = SDB_CHECK_KEY_EQUAL;
    }

    return result;
}

sceneRecord_t * sceneListGetSceneByID(uint8_t key)
{
    char * rec;
	log_debug("sceneListGetSceneByID++\n");

    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)sceneListCheckKeyId);
    if (rec == NULL)
    {
        return NULL;
    }
	log_debug("%s",rec);
    return sceneListParseRecord(rec);
}

/*
sceneRecord_t * sceneListGetSceneByName(char *sceneNameStr)
{
    char * rec;

    rec = SDB_GET_UNIQUE_RECORD(db, sceneNameStr, (check_key_f)sceneListCheckKeyName);
    if (rec == NULL)
    {
        return NULL;
    }

    return sceneListParseRecord(rec);
}
*/

uint8_t sceneListGetUnusedSceneId(void)
{
    static uint8_t lastUsedSceneId = 0;
	bool FullScene = false;
	
    lastUsedSceneId++;

	if(lastUsedSceneId >= 255)
		lastUsedSceneId = 1;

    while (SDB_GET_UNIQUE_RECORD(db, &lastUsedSceneId, (check_key_f)sceneListCheckKeyId)
           != NULL)
    {
        lastUsedSceneId++;
        if(lastUsedSceneId >= 255)
        {
        	lastUsedSceneId = 1;
        	//当第二次进入到这里的时候，说明场景已经满了，所以覆盖将第一个场景覆盖
        	if(FullScene == true)
        	{
        		lastUsedSceneId = 255;//将最数据覆盖在最后一条
				break;
        	}
        	
			FullScene = true;
		}
    }

    return lastUsedSceneId;
}

/*
sceneRecord_t * sceneListRemoveSceneByName(char *sceneNameStr)
{
    return sceneListParseRecord(
               sdb_delete_record(db, sceneNameStr, (check_key_f)sceneListCheckKeyName));
}
*/
sceneRecord_t * sceneListRemoveSceneByID(uint8_t SceneId)
{
    return sceneListParseRecord(
               sdb_delete_record(db, &SceneId, (check_key_f)sceneListCheckKeyId));
}


sceneRecord_t * sceneListRemoveALLScene()
{
    uint32_t context = 0;
    while(sceneListGetNextScene(&context) != NULL)
    {
        sceneListParseRecord(
            sdb_delete_record(db, 0x00, (check_key_f)sceneListCheckAll));
    }
    return NULL;
}


uint8_t sceneListGetSceneNum()
{
    uint8_t i=0;
    uint32_t context = 0;
    while(sceneListGetNextScene(&context) != NULL)
    {
        ++i;
    }

    return i;

}

bool sceneListModifyRecordByID(uint8_t sceneid,sceneRecord_t *scene)
{
	bool rtn = false;
    char rec[MAX_SUPPORTED_RECORD_SIZE]= {0};
	char *str = NULL;
	
   	str = sceneListComposeRecord(scene, rec);

    log_debug("%s\n",str);
    
	//如果数据长度一样，则可以修改，如果数据长度不一样则需要先删除在添加
	if((rtn = sdb_modify_record(db, &sceneid, (check_key_f) sceneListCheckKeyId,rec))==false)
	{
		if(sceneListRemoveSceneByID(sceneid)!=NULL)
		{
			rtn = sdb_add_record(db, rec);
		}
	}
	
	log_debug("%d\n",rtn);
	
    return rtn;
}


/*********************************************************************
 * @fn      sceneListAddScene
 *
 * @brief   add a scene to the scene list.
 *
 * @return  sceneId
 */
uint8_t sceneListAddScene(sceneRecord_t *Scene)
{
    sceneRecord_t *exsistingScene = NULL;
    char rec[MAX_SUPPORTED_RECORD_SIZE] = {0};
    
 	log_debug("sceneListAddScene++\n");
 	
    exsistingScene = sceneListGetSceneByID(Scene->sceneId);

	if(exsistingScene != NULL)
	{
		return exsistingScene->sceneId;
	}
	
	Scene->sceneId = sceneListGetUnusedSceneId();

	sceneListComposeRecord(Scene, rec);

	sdb_add_record(db, rec);

    return Scene->sceneId;
}


/*********************************************************************
 * @fn      sceneListAddScene
 *
 * @brief   add a scene to the scene list.
 *
 * @return  sceneId
 */
 
 /*
uint8_t sceneListGetSceneId( char *sceneNameStr )
{
    uint8_t sceneId = 0;
    sceneRecord_t *scene;

    scene = sceneListGetSceneByName(sceneNameStr);

    if( scene == NULL)
    {
        sceneId = -1;
    }
    else
    {
        sceneId = scene->sceneId;
    }

    //printf("sceneListGetSceneId--\n");

    return sceneId;
}
*/

/*********************************************************************
 * @fn      sceneListGetNextScene
 *
 * @brief   Return the next scene in the list.
 *
 * @param   context Pointer to the current scene record
 *
 * @return  sceneRecord_t, return next scene record in the DB
 */
sceneRecord_t* sceneListGetNextScene(uint32_t *context)
{
    char * rec;
    sceneRecord_t *scene;
		
    do
    {
        rec = SDB_GET_NEXT_RECORD(db,context);

        if (rec == NULL)
        {
            return NULL;
        }
		log_debug("%s",rec);
        scene = sceneListParseRecord(rec);
    }
    while (scene == NULL); //in case of a bad-format record - skip it and read the next one

    return scene;
}

uint16_t sceneListGetAllScene(uint8_t *devNum,uint8_t *pBuf,uint16_t bufsize)
{
	sceneRecord_t *scene;
	sceneMembersRecord_t *sceneMembers;
	uint32_t context = 0;
	hostCmd cmd;
	cmd.idx = 0;

	*devNum=0;
	log_debug("sceneListGetAllScene++\n");
	while((scene = sceneListGetNextScene(&context))!=NULL)
	{
		if(bufsize >= cmd.idx) //防止数组越界
		{
			(*devNum)++;
			cmdSet8bitVal(&cmd,scene->sceneId);
			cmdSet8bitVal(&cmd,scene->submbers);
			sceneMembers = scene->members;
			while(sceneMembers != NULL)
			{
				cmdSetStringVal(&cmd, (uint8_t *)sceneMembers->IEEEAddr, 8);
				cmdSet8bitVal(&cmd,sceneMembers->endpoint);
				cmdSet8bitVal(&cmd, sceneMembers->length);
				cmdSetStringVal(&cmd, (uint8_t *)sceneMembers->dataSegment,sceneMembers->length);
				sceneMembers = sceneMembers->next;
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

