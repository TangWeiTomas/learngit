/**************************************************************************************************
 * Filename:       localServerHandle.h
 * Author:             zxb      
 * E-Mail:          zxb@yystart.com 
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/ 
 * 
 * Version:         
 *
 */

#ifndef LOCAL_SERVER_HANDLE_H
#define LOCAL_SERVER_HANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "comParse.h"


#define LOCAL_SERVER_TCP_PORT 8100


void local_Server_DataInit(void);

void local_Server_RxCB(int clientFd);

void local_Server_ConnectCB(int clientFd);




#ifdef __cplusplus
}
#endif

#endif //LOCAL_SERVER_HANDLE_H
