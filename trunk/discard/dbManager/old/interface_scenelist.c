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
#include <unistd.h>

#include "interface_scenelist.h"
#include "hal_types.h"
#include "SimpleDBTxt.h"

static db_descriptor * db;

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    char* name;
    uint16 groupId;
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

static char * sceneListComposeRecord(sceneRecord_t *scene, char * record)
{
    sceneMembersRecord_t *sceneMembers;

    sprintf(record, "        0x%04X , 0x%02X , \"%s\"", //leave a space at the beginning to mark this record as deleted if needed later, or as bad format (can happen if edited manually). Another space to write the reason of bad format.
            scene->groupId, scene->sceneId, scene->name ? scene->name : "");

    sceneMembers = scene->members;

    while (sceneMembers != NULL)
    {
        sprintf(record + strlen(record), " , 0x%04X , 0x%02X, 0x%04X , 0x%02X, 0x%02X, 0x%02X, 0x%02X",
                sceneMembers->nwkAddr, sceneMembers->endpoint,sceneMembers->deviceid,sceneMembers->data1,sceneMembers->data2,sceneMembers->data3,sceneMembers->data4);
        sceneMembers = sceneMembers->next;
    }

    sprintf(record + strlen(record), "\n");

    return record;
}

#define MAX_SUPPORTED_SCENE_NAME_LENGTH 32
#define MAX_SUPPORTED_SCENE_MEMBERS 100

static sceneRecord_t * sceneListParseRecord(char * record)
{
    char * pBuf = record + 1; //+1 is to ignore the 'for deletion' mark that may just be added to this record.
    static sceneRecord_t scene;
    static char sceneName[MAX_SUPPORTED_SCENE_NAME_LENGTH + 1];
    static sceneMembersRecord_t member[MAX_SUPPORTED_SCENE_MEMBERS];
    sceneMembersRecord_t ** nextMemberPtr;
    parsingResult_t parsingResult =
    { SDB_TXT_PARSER_RESULT_OK, 0 };
    int i;

    if (record == NULL)
    {
        return NULL;
    }

    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &scene.groupId, 2, false,
                                     &parsingResult);
    sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &scene.sceneId, 1, false,
                                     &parsingResult);
    sdb_txt_parser_get_quoted_string(&pBuf, sceneName,
                                     MAX_SUPPORTED_SCENE_NAME_LENGTH, &parsingResult);
    nextMemberPtr = &scene.members;
    for (i = 0;
         (parsingResult.code == SDB_TXT_PARSER_RESULT_OK)
         && (i < MAX_SUPPORTED_SCENE_MEMBERS); i++)
    {
        *nextMemberPtr = &(member[i]);
        sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &(member[i].nwkAddr), 2,
                                         false, &parsingResult);
        sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &(member[i].endpoint),
                                         1, false, &parsingResult);

        sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &(member[i].deviceid), 2,
                                         false, &parsingResult);
        sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &(member[i].data1),
                                         1, false, &parsingResult);
        sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &(member[i].data2),
                                         1, false, &parsingResult);
        sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &(member[i].data3),
                                         1, false, &parsingResult);
        sdb_txt_parser_get_numeric_field(&pBuf, (uint8_t *) &(member[i].data4),
                                         1, false, &parsingResult);
        nextMemberPtr = &(member[i].next);
    }
    *nextMemberPtr = NULL;

    if ((parsingResult.code != SDB_TXT_PARSER_RESULT_OK)
        && (parsingResult.code != SDB_TXT_PARSER_RESULT_REACHED_END_OF_RECORD))
    {
        sdbtMarkError(db, record, &parsingResult);
        return NULL;
    }

    if (strlen(sceneName) > 0)
    {
        scene.name = sceneName;
    }
    else
    {
        scene.name = NULL;
    }

    return &scene;
}

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

    rec = SDB_GET_UNIQUE_RECORD(db, &key, (check_key_f)sceneListCheckKeyId);
    if (rec == NULL)
    {
        return NULL;
    }

    return sceneListParseRecord(rec);
}


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


uint8_t sceneListGetUnusedSceneId(void)
{
    static uint8_t lastUsedSceneId = 0;

    lastUsedSceneId++;

    while (SDB_GET_UNIQUE_RECORD(db, &lastUsedSceneId, (check_key_f)sceneListCheckKeyId)
           != NULL)
    {
        lastUsedSceneId++;
    }

    return lastUsedSceneId;
}

sceneRecord_t * sceneListRemoveSceneByName(char *sceneNameStr)
{
    return sceneListParseRecord(
               sdb_delete_record(db, sceneNameStr, (check_key_f)sceneListCheckKeyName));
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


/*********************************************************************
 * @fn      sceneListAddScene
 *
 * @brief   add a scene to the scene list.
 *
 * @return  sceneId
 */
uint8_t sceneListAddScene( char *sceneNameStr, uint16_t groupId , uint16_t dstAddr,
                           uint8_t endpoint,uint16_t deviceid,uint8_t data1,uint8_t data2,uint8_t data3,uint8_t data4)
{
    sceneRecord_t *Scene;
    sceneRecord_t newScene;
    char rec[MAX_SUPPORTED_RECORD_SIZE];

    sceneMembersRecord_t ** nextMemberPtr;
    sceneMembersRecord_t newMember;

    bool memberExists = false;
    uint8_t sceneId;

    printf("sceneListAddScene++\n");

    Scene = sceneListGetSceneByName(sceneNameStr);

    if (Scene == NULL)
    {

        printf("sceneListAddScene: add new scene !\n");
        Scene = &newScene;
        Scene->groupId = groupId;
        Scene->sceneId = sceneListGetUnusedSceneId();
        Scene->name = sceneNameStr;
        Scene->members = NULL;
    }

    printf("test1 !\n");
    sceneId = Scene->sceneId;

    nextMemberPtr = &(Scene->members);
    printf("test2 !\n");
    while ((*nextMemberPtr != NULL) && (!memberExists))
    {
        if (((*nextMemberPtr)->nwkAddr == dstAddr)
            && ((*nextMemberPtr)->endpoint == endpoint))
        {
            memberExists = true;

            (*nextMemberPtr)->deviceid = deviceid;
            (*nextMemberPtr)->data1 = data1;
            (*nextMemberPtr)->data2 = data2;
            (*nextMemberPtr)->data3 = data3;
            (*nextMemberPtr)->data4 = data4;
        }
        else
        {
            nextMemberPtr = &((*nextMemberPtr)->next);
        }
    }

    if (!memberExists)
    {
        *nextMemberPtr = &newMember;
        newMember.nwkAddr = dstAddr;
        newMember.endpoint = endpoint;
        newMember.deviceid = deviceid;
        newMember.data1 = data1;
        newMember.data2 = data2;
        newMember.data3 = data3;
        newMember.data4 = data4;

        newMember.next = NULL;

    }

    printf("test3 !\n");

    sceneListComposeRecord(Scene, rec);
    printf("test4 !\n");
    sceneListRemoveSceneByName(sceneNameStr);
    printf("test5 !\n");
    sdb_add_record(db, rec);

    printf("SceneListAddScene--\n");

    return sceneId;
}



uint8_t sceneListRemoveSceneMember( char *sceneNameStr, uint16_t groupId , uint16_t dstAddr, uint8_t endpoint)
{
    printf("sceneListRemoveSceneMember++\n");

    sceneRecord_t *Scene;
    char rec[MAX_SUPPORTED_RECORD_SIZE];

    sceneMembersRecord_t ** nextMemberPtr;

    bool memberExists = false;
    uint8_t sceneId;

    Scene = sceneListGetSceneByName(sceneNameStr);

    if (Scene == NULL)
    {
        printf("sceneListRemoveSceneMember: Not this scene\n");
    }

    sceneId = Scene->sceneId;

    if(dstAddr== 0xffff || endpoint == 0xff)
    {
        sceneListRemoveSceneByName(sceneNameStr);
        return sceneId;
    }

    nextMemberPtr = &(Scene->members);

    if (((*nextMemberPtr)->nwkAddr == dstAddr)
        && ((*nextMemberPtr)->endpoint == endpoint))
    {

        Scene->members = ((*nextMemberPtr)->next);
        memberExists = true;

        if(Scene->members != NULL )
            printf("Scene->members 1: %x  %x \n",(Scene->members)->nwkAddr,(Scene->members)->endpoint);
        else printf("Scene->members 2: %x \n",(Scene->members)->nwkAddr);
    }
    else
    {
        while (( ((*nextMemberPtr)->next) != NULL) && (!memberExists))
        {
            if (( ((*nextMemberPtr)->next)->nwkAddr == dstAddr)
                && (((*nextMemberPtr)->next)->endpoint == endpoint))
            {
                memberExists = true;

                (*nextMemberPtr)->next = ((*nextMemberPtr)->next)->next;
            }
            else
            {
                nextMemberPtr = &((*nextMemberPtr)->next);
            }
        }
    }
    if (memberExists)
    {
        //printf("Scene->members 4: %x  %x \n",(Scene->members)->nwkAddr,(Scene->members)->endpoint);
        sceneListComposeRecord(Scene, rec);
        printf("Scene->members 3: %x  %x \n",(Scene->members)->nwkAddr,(Scene->members)->endpoint);
        printf("Scene: %s \n",rec);


        sceneListRemoveSceneByName(sceneNameStr);
        printf("Scene->members 5: %x  %x \n",(Scene->members)->nwkAddr,(Scene->members)->endpoint);
        //sceneListComposeRecord(Scene, rec);
        //printf("Scene->members 3: %x  %x \n",(Scene->members)->nwkAddr,(Scene->members)->endpoint);
        //printf("Scene: %s \n",rec);
        sdb_add_record(db, rec);

    }

    printf("sceneListRemoveSceneMember--\n");

    return sceneId;
}

/*********************************************************************
 * @fn      sceneListAddScene
 *
 * @brief   add a scene to the scene list.
 *
 * @return  sceneId
 */
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

        scene = sceneListParseRecord(rec);
    }
    while (scene == NULL); //in case of a bad-format record - skip it and read the next one

    return scene;
}
