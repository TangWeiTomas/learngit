/***********************************************************************************
 * �� �� ��   : updateHW.h
 * �� �� ��   : Edward
 * ��������   : 2016��5��27��
 * �ļ�����   : Զ�̸��¹̼�
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/
#ifndef __UPDATE_HW_H__
#define __UPDATE_HW_H__

extern bool start_download;


int updateHW_DownLoadFiles(const char* filename,const char *url);

int updateHW_SysUpgrade(const char *pathfileName,uint8_t vfalg);
int updateHW_getServerAddr(char *srcaddr,int srclen);
int updateHW_VersionComplate(char*fwVersion,int len);
int updateHW_GetupdateInfo(void);
char updateHW_System(char args,char *fwname,char *md5name,char *fileurl);
void* updateHW_Update_Pthread(void* args);


#endif
