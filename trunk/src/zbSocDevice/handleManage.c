#include "handleManage.h"
#include "string.h"

#include "logUtils.h"

typedef struct
{
    void *handle;   //句柄
    void *id;       //标识
    uint32_t checkSum;
}handleList_t;

typedef struct
{
    uint32_t idLength;
    uint32_t handleLength;      //句柄的长度
    uint32_t maxListLength;
    uint32_t count;
    
    handleList_t *handleList;
    
}HANDLE_MANAGE_T;


HANDLE_MANAGE_HANDLE HandleManage_Create(uint32_t maxListLength, uint32_t handleLength, uint32_t idLength)
{
    HANDLE_MANAGE_T *manage = malloc(sizeof(HANDLE_MANAGE_T));
    
    if (manage == NULL)
    {
        return NULL;
    }
    
    manage->maxListLength = maxListLength;
    manage->idLength = idLength;    //标识长度
    manage->handleLength = handleLength;    //句柄长度
    manage->handleList = malloc(sizeof(handleList_t) * maxListLength);
    if (manage->handleList == NULL)
    {
        free(manage);
        return NULL;
    }
    manage->count = 0;
    log_debug(" manage %x\r\n", manage);
    return manage;
}


void *HandleMange_GetHadnle(HANDLE_MANAGE_HANDLE manageHandle, void *id)
{
	if(!manageHandle)
		return NULL;

    HANDLE_MANAGE_T *manage = manageHandle;
    uint32_t i, checkSum;
    
    for (i = 0; i < manage->maxListLength; i++)
    {
        checkSum = ~((int)manage->handleList[i].id ^ (int)manage->handleList[i].handle);
        if (checkSum == manage->handleList[i].checkSum)
        {
            if (memcmp(id, manage->handleList[i].id, manage->idLength) == 0)
            {
                log_debug("find handle %x\r\n", manage->handleList[i].handle);
                return manage->handleList[i].handle;
            }
        }
    }
    
    for (i = 0; i < manage->maxListLength; i++)
    {
        checkSum = ~((int)manage->handleList[i].id ^ (int)manage->handleList[i].handle);
        if (checkSum != manage->handleList[i].checkSum)
        {
            log_debug("handle length %x\r\n", manage->handleLength);
            manage->handleList[i].handle = malloc(manage->handleLength);
            memset(manage->handleList[i].handle, 0, manage->handleLength);
            
            manage->handleList[i].id = malloc(manage->idLength);
            memcpy(manage->handleList[i].id, id, manage->idLength);
            
            manage->handleList[i].checkSum =  ~((int)manage->handleList[i].id ^ (int)manage->handleList[i].handle);
            
            log_debug("not find handle, create %x \r\n", manage->handleList[i].handle);
            return manage->handleList[i].handle;
        }
    }
    log_debug("ERROR\r\n");
    return NULL;
}


