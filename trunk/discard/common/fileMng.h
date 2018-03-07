/**************************************************************************************************
 * Filename:       fileMng.h
 * Author:             zxb
 * E-Mail:          zxb@yystart.com
 * Description:    Socket Remote Procedure Call Interface - sample device application.
 *
 *  Copyright (C) 2014 Yun Yin Company - http://www.yystart.com/
 *
 * Version:         1.00  (2014-12-02,21:05)    :   Create the file.
 *
 *
 *************************************************************************/

#ifndef FILE_MNG_H
#define FILE_MNG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "Types.h"
/********************************************************************/
enum
{
    AUTH_OPEN = 0,
    AUTH_SHARED,
    AUTH_PSK,
    AUTH_PSK2
};

/********************************************************************/
void wififileConfig(char* ssidInput, char* pwdInput,char authNum);

void wifiServerInfofileConfig(uint8_t* serverIPAddr, uint16_t serverPort);

int openfile(const char *pfile);

int closefile(int fd);

int readfile(int fd);

int writefile(int fd,char buf[]);
void wifiConfigbackups(void);

int read_line(int fd, void *vptr, int maxlen);
int getWifiServerInfo(char* address, char *port);
int getWifiServerInfofile(char* address, uint16_t *port);
int writeBinaryFile(const char* _fileName, void* _buf, int _bufLen);
int readBinaryFile(const char* _fileName, void* _buf, int _bufLen);
bool isFileExist(const char *file);

#ifdef __cplusplus
}
#endif

#endif /* FILE_MNG_H */
