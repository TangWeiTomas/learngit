/***********************************************************************************
 * 文 件 名   : updateHW.h
 * 负 责 人   : Edward
 * 创建日期   : 2016年5月27日
 * 文件描述   : 远程更新固件
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
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
