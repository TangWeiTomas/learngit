/***********************************************************************************
 * �� �� ��   : updateHW.c
 * �� �� ��   : Edward
 * ��������   : 2016��5��27��
 * �ļ�����   : Զ�̸��¹̼�
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
***********************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "curl.h"
#include "globalVal.h"
#include "fileMng.h"
#include "md5.h"
#include <stdlib.h>
#include "cJSON.h"
#include "interface_srpcserver.h"
#include "logUtils.h"

/* <DESC> * Get a single file from an FTP server. * </DESC> */

#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES  6000
#define MNIMAL_PROGRESS_FUNCTIONALITY_INTERVAL 3

bool start_download = false;

struct myprogress{	
	double lastruntime;	
	CURL *curl;
};

struct FtpFile { 
	const char *filename;  
	FILE *stream;
};

static curl_off_t getpercent(curl_off_t dltotal,curl_off_t dlnow)
{	
	if (dlnow < 100)
		return 0;
	curl_off_t onepercent = dltotal / 100;
	if(dlnow == 0|| dltotal == 0)		
		return 0;	
	curl_off_t percent = dlnow / onepercent;
	return percent;
}

curl_off_t newpercent = 0;
curl_off_t oldpercent = 0;

//���ؽ�����
static int updateHW_DownLoadProcess(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{	
	newpercent = getpercent(dltotal,dlnow);
	if(newpercent != oldpercent)
	{		
		fprintf(stderr,"DOWN: %" CURL_FORMAT_CURL_OFF_T "\n",newpercent);
		oldpercent = newpercent;	
		if((newpercent%4)==0)
		{
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_UP_PROGRESS,YY_STATUS_DOWNLOAD_PROGRESS,newpercent);
		}
	} 
	return 0;
}

//д�ļ�
static size_t updateHW_WriteFile(void *buffer, size_t size, size_t nmemb, void *stream)
{  
	struct FtpFile *out=(struct FtpFile *)stream;
	if(out && !out->stream) 
	{   
		/* open file for writing */
		out->stream=fopen(out->filename, "wb");
		if(!out->stream)
			return -1;
		/* failure, can't open file to write */
	}
	return fwrite(buffer, size, nmemb, out->stream);
}

int updateHW_DownLoadFiles(const char* filename,const char *url)
{
	CURL *curl;
	CURLcode res;
	char relUrl[URLSIZE] = {0};
	char mFilename[FILE_NAME_SIZE]={0};
	struct myprogress prog;

	if(filename==NULL)
	{
		fprintf(stderr, "unknow filename\n");
		return -1;
	}
	
	//���������ļ���ʱ�洢��ַ
#ifdef OPENWRT_TEST 
	sprintf(mFilename,"/var/%s",filename);
#else
	sprintf(mFilename,"%s",filename);
#endif

	remove(mFilename);

	struct FtpFile ftpfile = {
		mFilename,
		NULL
	};
	
	log_debug("FILE:%s\n",mFilename);
	log_debug("URL:%s\n",url);

	sprintf(relUrl,"%s%s",url,filename);
	log_debug("URL:%s\n",relUrl);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl)
	{	
		//prog.lastruntime = 0;	
		//prog.curl = curl;
		/*You better replace the URL with one that works!     */   
		curl_easy_setopt(curl, CURLOPT_URL,relUrl); 
		//"ftp://administrator:Yunyin123@139.196.152.23/fszigbeegw");    
		/* Define our callback to get called when there's data to be written */    
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, updateHW_WriteFile);    
		/* Set a pointer to our struct to pass to the callback */    
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);	
		curl_easy_setopt(curl,CURLOPT_XFERINFOFUNCTION,updateHW_DownLoadProcess);	
		curl_easy_setopt(curl,CURLOPT_XFERINFODATA,&prog);    
		/* Switch on full protocol/debug output */
		//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
		res = curl_easy_perform(curl);
		/* always cleanup */    
		curl_easy_cleanup(curl);
		if(CURLE_OK != res) 
		{ 
		  /* we failed */
		  fprintf(stderr, "curl told us %d\n", res);
		  return -1;
		}
	}
	if(ftpfile.stream)
		fclose(ftpfile.stream); 
		/* close the local file */  
	curl_global_cleanup();  
	return 0;
}

int updateHW_SysUpgrade(const char *fwfileName,uint8_t vflag)
{
	char cmd[URLSIZE]={0};
	char mfwfileName[URLSIZE] = {0};
	int ret = 0;

	if(strlen(fwfileName) ==0)
		return -1;
		
	SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_SUCCESS,0,0);
	
#ifdef OPENWRT_TEST

	sprintf(mfwfileName,"/var/%s",fwfileName);
	
	if(vflag == 0x01)
		//sprintf(cmd,"/sbin/sysupgrade -n -v %s\n",mfwfileName);
		sprintf(cmd,"/sbin/mtd erase rootfs_data && /sbin/mtd -r write %s firmware \n",mfwfileName);
	else
		sprintf(cmd,"/sbin/mtd -r write %s firmware \n",mfwfileName);
		//sprintf(cmd,"/sbin/sysupgrade -v %s\n",mfwfileName);
		
	log_debug("%s",cmd);

	system(cmd);
	
#endif

	return 0;
}


int updateHW_getServerAddr(char *srcaddr,int srclen)
{
	if(srcaddr==NULL || srclen <=0)
		return -1;
		
 	int fd;
//    char buf[30];
    int len;

    struct stat st;
    memset(&st,0,sizeof(st));

#ifdef OPENWRT_TEST
    char *path = "/etc/config/UpdateServer";
#else
    char *path = "./UpdateServer";
#endif

    if(stat(path,&st))
    {
        return -1;
    }

	fd = open(path, O_RDONLY);
	if(-1 == fd)
	{
		log_debug("open %s failed\n",path);
		return -1;
	}
	
   	len = read_line(fd,srcaddr,srclen);
   	
    if(len<0)
    {
        log_err("read failed for %s\n", strerror(errno));
        close(fd);
        return -1;;
    }
	
    close(fd);
    return 0;

}

int updateHW_MD5Sum(const char *fwFileName,const char *md5fileName)
{
	if(NULL == fwFileName||NULL==md5fileName)
		return -1;
	
	char mfwFileName[URLSIZE]={0};
	char mMd5fileName[URLSIZE]={0};
	char md5sums[MD5_STR_LEN+1] = {0};
	
#ifdef OPENWRT_TEST 
	sprintf(mfwFileName,"/var/%s",fwFileName);
	sprintf(mMd5fileName,"/var/%s",md5fileName);
#else
	sprintf(mfwFileName,"%s",fwFileName);
	sprintf(mMd5fileName,"%s",md5fileName);
#endif

	log_debug("%s \n %s",mfwFileName,mMd5fileName);

	if(-1 == MD5Sums(mfwFileName,md5sums))
	{
		log_debug("MD5Sums Failed\n");
		return -1;
	}

	if(-1 == MD5Complate(mMd5fileName,md5sums))
	{
		log_debug("MD5Complate Failed\n");
		return -1;
	}

	log_debug("MD5 Successful\n");
	return 0;
}


int updateHW_ChangeVersionNum(char *version,char*reversion,int len)
{
	if(NULL==version||0==len||NULL==reversion)
		return -1;

	char * s = strtok(version, ".");
	int i=0;
	while(s)
	{
		reversion[i] = atoi(s);;
		s = strtok(NULL, ".");
		i++;
		if(i>len)
			return -1;
	}
	return 0;
}

int updateHW_VersionComplate(char*fwVersion,int len)
{
	if(NULL==fwVersion||0==len)
		return -1;
	char remoteVersion[32] = {0};
	char curversion[] = VERSION;
	char curfwVersin[3] = {0};
	char newfwVersion[3] ={0};

	memcpy(remoteVersion,fwVersion,len);
	if(-1 == updateHW_ChangeVersionNum(remoteVersion,newfwVersion,len))
	{
		log_debug("updateHW_ChangeVersionNum 1 failed\n");
		return -1;
	}
	
	log_debug("NewVersion:V%d.%d.%d",newfwVersion[0],newfwVersion[1],newfwVersion[2]);
	
	if(-1 == updateHW_ChangeVersionNum(curversion,curfwVersin,strlen(curversion)))
	{
		log_debug("updateHW_ChangeVersionNum 2 failed\n");
		return -1;
	}
	
	log_debug("CurVersion:V%d.%d.%d",curfwVersin[0],curfwVersin[1],curfwVersin[2]);

	if(curfwVersin[0] > newfwVersion[0])
	{
		return 1;
	}
	else if(curfwVersin[0] < newfwVersion[0])
	{
		return 2;
	}
	else
	{
		if(curfwVersin[2] > newfwVersion[2])
		{
			return 1;
		}
		else if(curfwVersin[2] < newfwVersion[2])
		{
			return 2;
		}
		else
		{
			return 0;
		}
	}
	
}

char updateHW_System(char args,char *fwname,char *md5name,char *fileurl)
{

	log_debug("updateHW_System ++\n");
	if(fwname == NULL || md5name == NULL||fileurl==NULL)
		return -1;
		
	//���ع̼�
	if(-1 == updateHW_DownLoadFiles(fwname,fileurl))
	{
		log_debug("updateHW_DownLoadFiles Failed\n");
		return -1;
	}

	//����MD5У���ļ�
	if(-1 == updateHW_DownLoadFiles(md5name,fileurl))
	{
		log_debug("updateHW_DownLoadFiles Failed\n");
		return -1;
	}

	//У���ļ�
	if(-1==updateHW_MD5Sum(fwname,md5name))
	{
		log_debug("updateHW_MD5Sum Failed\n");
		return -1;
	}

	if (-1 == updateHW_SysUpgrade(fwname,args))
	{
		log_debug("updateHW_SysuUgrade Failed\n");
		return -1;
	}
	
	log_debug("updateHW_System --\n");
	return 0;
	
}

char updateHW_GetupdateInfo(char *fwVersion,char *fwname,char *md5name,char*fileurl)
{

/*
{
	"Version":"3.0.3",
	"FileName":"SmartHotalHost_fm_2016_09_07_V3.0.3_beta.bin",
	"FileMD5":"SmartHotalHost_fm_2016_09_07_V3.0.3_beta.md5",
	"FileUrl":"ftp://wx.feixuekj.cn/"
}
*/

	cJSON *json;
	cJSON *args;
	char fileName[URLSIZE] = {0};
#ifdef OPENWRT_TEST 
	 sprintf(fileName,"/var/%s",CONFIGFILE);
#else
	 sprintf(fileName,"%s",CONFIGFILE);
#endif
	json = GetJsonObject(fileName,json);
	if(NULL != json)
	{
		args = cJSON_GetObjectItem(json,"Version");	
		if(args!=NULL)
		{
			log_debug("Version =%s\n",args->valuestring);
			memcpy(fwVersion,args->valuestring,strlen(args->valuestring));
		}

		args = cJSON_GetObjectItem(json,"FileName");	
		if(args!=NULL)
		{
			log_debug("FileName =%s\n",args->valuestring);
			memcpy(fwname,args->valuestring,strlen(args->valuestring));
		}

		args = cJSON_GetObjectItem(json,"FileMD5");	
		if(args!=NULL)
		{
			log_debug("FileMD5 =%s\n",args->valuestring);
			memcpy(md5name,args->valuestring,strlen(args->valuestring));
		}
		args = cJSON_GetObjectItem(json,"FileUrl");	
		if(args!=NULL)
		{
			log_debug("FileUrl =%s\n",args->valuestring);
			memcpy(fileurl,args->valuestring,strlen(args->valuestring));
		}
		
		cJSON_Delete(json);
		return 0;
	}
	return -1;
}



/*****************************************************************************
 * �� �� ��  : updateHW_isFileExist
 * �� �� ��  : Edward
 * ��������  : 2016��6��7��
 * ��������  : �ж��ļ��Ƿ����
 * �������  : char *fileName  �ļ�����
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
int updateHW_isFileExist(char *fileName)
{
	char file[URLSIZE] = {0};
#ifdef OPENWRT_TEST 
	 //char* updatefile = "/etc/config/update.ini";
	 sprintf(file,"/var/%s",fileName);
#else
	 sprintf(file,"./%s",fileName);
#endif
	if(access(file,0)==0)
	{
		return 0;
	}

	return -1;
}

char updateHW_Update_Pthread(void* args)
{
	uint8_t vflag = (*((uint8_t*)args));
	char retVersion = 0;

	start_download = true;
	
	if(vflag == 1 || vflag == 0 )
	{
		log_debug("SRPC_UpDateMasterHostFWReq++2\n");

		char fileUrl[URLSIZE] = {0};
		char fwfilename[FILE_NAME_SIZE] = {0};
		char md5filename[FILE_NAME_SIZE] = {0};
		char fwVerson[FILE_NAME_SIZE] = {0};
		
		//���ظ����ļ���Ϣ
		//if(updateHW_isFileExist(CONFIGFILE)==-1)
		{
			if(-1 == updateHW_DownLoadFiles(CONFIGFILE,UPDATESERVER))
			{
				log_debug("updateHW_DownLoadFiles Failed\n");
				SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
				start_download = false;
				return -1;
			}
		}
		
		log_debug("SRPC_UpDateMasterHostFWReq++3\n");

		//��ȡ�汾���ļ������ص�ַ��Ϣ
		if(-1 == updateHW_GetupdateInfo(fwVerson,fwfilename,md5filename,fileUrl))
		{
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
			start_download = false;
			return -1;
		}
		
		log_debug("SRPC_UpDateMasterHostFWReq++4\n");
		//�Ƚϵ�ǰ�汾��Զ�̰汾
		
		retVersion = updateHW_VersionComplate(fwVerson,strlen(fwVerson));
		log_debug("Version = %d\n",retVersion);
		if(-1 == retVersion)
		{
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
			start_download = false;
			return -1;
		}
		else if(0 == retVersion)
		{
			//�̼��汾��ͬ
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_SAME_VERSION,0,0);
		}
		else if(1 == retVersion) //��ǰ�汾����Զ�̰汾
		{
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_DEGRADE_SUCCESS,0,0);
		}
		else //��ǰ�汾С�ڷ������汾
		{
			log_debug("SRPC_UpDateMasterHostFWReq++5\n");
			if(-1 == updateHW_System(vflag,fwfilename,md5filename,fileUrl))
			{
				SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
			}
		}
	}
	else
	{
		log_debug("SRPC_UpDateMasterHostFWReq++6\n");
		SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
	}
	start_download = false;
	return -1;
}
