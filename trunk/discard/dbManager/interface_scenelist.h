/**************************************************************************************************
 * Filename:       interface_scenelist.h
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

#ifndef INTERFACE_SCENELIST_H
#define INTERFACE_SCENELIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include "Types.h"
#include "zbSocCmd.h"

#define NEW_TIMER_SCENE		0X00

typedef struct sceneMembersRecord_s
{
    struct sceneMembersRecord_s * next;
	uint8_t IEEEAddr[8];
	uint8_t endpoint;			   		// Endpoint identifier
	uint8_t length;						//最大8字节
	uint8_t dataSegment[DATASEGMENT];
} sceneMembersRecord_t;

typedef struct 
{
    sceneMembersRecord_t *members;
    void *next;
	uint8_t submbers; //任务数
    uint8_t sceneId;
} sceneRecord_t;

/*
 * sceneListAddScene - create a scene and add a rec fto the list.
 */
//uint8_t sceneListAddScene(char *sceneNameStr, uint16_t groupId);

uint8_t sceneListAddScene( sceneRecord_t *Scene);
sceneRecord_t * sceneListRemoveALLScene();


/*
 * sceneListAddScene - gets the scen id of a a scene
 */

uint8_t sceneListGetUnusedSceneId(void);
uint8_t sceneListGetSceneNum();

/*
 * sceneListGetNextScene - Return the next scene in the list.
 */
sceneRecord_t* sceneListGetNextScene(uint32_t *context);

/*
 * groupListInitDatabase - Restore Scene List from file.
 */
sceneRecord_t * sceneListGetSceneByID(uint8_t key);
void sceneListInitDatabase(char * dbFilename);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_SCENELIST_H */
