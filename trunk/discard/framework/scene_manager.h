#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__
#include <stdio.h>
#include <stdint.h>

//#include "hal_types.h"
#include "zbSocCmd.h"
#include "interface_scenelist.h"
#include "queue.h"

typedef struct SceneListHead_t
{
	sceneRecord_t *sceneinfo;
    LIST_ENTRY(SceneListHead_t) entry_;
} SceneListHead_t;


LIST_HEAD(SceneList, SceneListHead_t) ;


uint8_t Scene_ProcessEvent(uint8_t sceneid);
uint8_t Device_ProcessEventByDeviceId(epInfo_t *epInfo,uint8_t *data,uint8_t lenght);
#endif
