/***********************************************************************************
 * 文 件 名   : updateHW.c
 * 负 责 人   : Edward
 * 创建日期   : 2016年5月27日
 * 文件描述   : 远程更新固件
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
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

#if 1
#include "aos_log.h"
#include "aos_util.h"
#include "aos_string.h"
#include "aos_status.h"
#include "oss_auth.h"
#include "oss_util.h"
#include "oss_api.h"
#include "oss_config.h"
#include "oss_sample_util.h"
#endif

/* <DESC> * Get a single file from an FTP server. * </DESC> */

#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES  6000
#define MNIMAL_PROGRESS_FUNCTIONALITY_INTERVAL 3

bool start_download = false;

#define FX7620N_UNKNOW_VERSION		-1
#define FX7620NA_VERSION			0
#define FX7620NB_VERSION			1
#define FX7620NC_VERSION			2
#define FX7688_VERSION				3

static int hostVersion = FX7620N_UNKNOW_VERSION;

static char *model[] = {"FX7620NA","FX7620NB","FX7620NC","FX7688"};

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

//下载进度条
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

//写文件
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

#if 0
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
	
	//设置下载文件零时存储地址
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
#endif

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
	{
		sprintf(cmd,"/sbin/mtd -r write %s firmware \n",mfwfileName);
	}
	else
	{
		sprintf(cmd,"/sbin/sysupgrade -b /tmp/sysupgrade.tgz && /sbin/mtd -j /tmp/sysupgrade.tgz write %s firmware && reboot -f \n",mfwfileName);
	}
	
	log_debug("%s",cmd);
	system(cmd);

	/*系统指示灯闪烁*/
	sprintf(cmd,"zbLedIndicator \"wr8305rt:sys\" \"blink\"\n");
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

char updateHW_System(char args,char *fwname,char *md5name)
{

	log_debug("updateHW_System ++\n");
	if(fwname == NULL || md5name == NULL)
		return -1;
	sleep(1);
	//下载固件
	if(-1 == get_object_to_local_file(fwname))
	{
		log_debug("updateHW_DownLoadFiles Failed\n");
		return -1;
	}
	sleep(1);
	//下载MD5校验文件
	log_debug("-----------md5name = %s------------\n",md5name);
	
	if(-1 == get_object_to_local_file(md5name))
	{
		log_debug("updateHW_DownLoadFiles Failed\n");
		return -1;
	}

	//校验文件
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

char updateHW_GetupdateInfo(char *fwVersion,char *fwname,char *md5name)
{

/*
{
	"Version":"3.0.3",
	"FileName":"SmartHotalHost_fm_2016_09_07_V3.0.3_beta.bin",
	"FileMD5":"SmartHotalHost_fm_2016_09_07_V3.0.3_beta.md5",
	"FileUrl":"ftp://wx.feixuekj.cn/"
}
*/

/*
//版本JSON格式
 {
 	"FX7620NA":{
        "Version":"3.0.3",
        "FileName":"SmartHotalHost_fm_2016_09_07_V3.0.3_beta.bin",
        "FileMD5":"SmartHotalHost_fm_2016_09_07_V3.0.3_beta.md5",
        "FileUrl":"ftp://wx.feixuekj.cn/"
    },
    
    "FX7620NB":{
        "Version":"3.0.3",
        "FileName":"SmartHotalHost_fm_2016_09_07_V3.0.3_beta.bin",
        "FileMD5":"SmartHotalHost_fm_2016_09_07_V3.0.3_beta.md5",
        "FileUrl":"ftp://wx.feixuekj.cn/"
    },
    
    "FX7620NC":{
        "Version": "3.0.3",
        "FileName": "SmartHotalHost_fm_2016_09_07_V3.0.3_beta.bin",
        "FileMD5": "SmartHotalHost_fm_2016_09_07_V3.0.3_beta.md5",
        "FileUrl": "ftp://wx.feixuekj.cn/"
    }
}
*/
	cJSON *root;
	cJSON *args;
	cJSON *host;
	int ret = -1;
	char fileName[URLSIZE] = {0};
//#ifdef OPENWRT_TEST 
	 sprintf(fileName,"/var/%s",CONFIGFILE);
//#else
	 //sprintf(fileName,"%s",CONFIGFILE);
//#endif
	log_debug("------------filename = %s-----------------\n",fileName);
	ret = updateHW_GetBoardInfo();
	if(ret == FAILED)
	{
		log_debug("-------------updateHW_GetBoardInfo---------failed\n");
		return -1;
	}
		
	root = GetJsonObject(fileName,root);
	if(root == NULL)
	{
		log_debug("---------------GetJsonObject------------failed\n");
		return -1;
	}
	do
	{
		//获取响应版本的固件名称
		host = cJSON_GetObjectItem(root,model[hostVersion]);
		if(host == NULL)
			break;
		
		args = cJSON_GetObjectItem(host,"Version");	
		if(args == NULL)
			break;

		log_debug("Version =%s\n",args->valuestring);
		memcpy(fwVersion,args->valuestring,strlen(args->valuestring));

	
		args = cJSON_GetObjectItem(host,"FileName");	
		if(args == NULL)
			break;
		
		log_debug("FileName =%s\n",args->valuestring);
		memcpy(fwname,args->valuestring,strlen(args->valuestring));
		
		args = cJSON_GetObjectItem(host,"FileMD5");	
		if(args == NULL)
			break;
	
		log_debug("FileMD5 =%s\n",args->valuestring);
		memcpy(md5name,args->valuestring,strlen(args->valuestring));
#if 0
		args = cJSON_GetObjectItem(host,"FileUrl");	
		if(args == NULL)
			break;

		log_debug("FileUrl =%s\n",args->valuestring);
		memcpy(fileurl,args->valuestring,strlen(args->valuestring));
#endif
		ret = SUCCESS;
	}while(0);

	cJSON_Delete(root);
	return ret;
}



/*****************************************************************************
 * 函 数 名  : updateHW_isFileExist
 * 负 责 人  : Edward
 * 创建日期  : 2016年6月7日
 * 函数功能  : 判断文件是否存在
 * 输入参数  : char *fileName  文件名称
 * 输出参数  : 无
 * 返 回 值  : 
 * 调用关系  : 
 * 其    它  : 

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

/*获取网关型号*/
int updateHW_GetBoardInfo(void)
{
	char buf[1024] = {0};
	char *pBuf = NULL;
	char *pStr = NULL;
	int ret = FAILED;
	FILE *fd = NULL;
	
	fd = fopen(FX_MODE_NAME, "r");
	if(fd == NULL)
		return ret;

	do
	{
		pBuf = fgets(buf,sizeof(buf),fd);
		if(pBuf == NULL)
			break;

		pStr = strstr(pBuf,FX7620NA);
		if(pStr != NULL)
		{
			hostVersion = FX7620NA_VERSION;
			break;
		}

		pStr = strstr(pBuf,FX7620NB);
		if(pStr != NULL)
		{
			hostVersion = FX7620NB_VERSION;
			break;
		}

		pStr = strstr(pBuf,FX7620NC);
		if(pStr != NULL)
		{
			hostVersion = FX7620NC_VERSION;
			break;
		}

		pStr = strstr(pBuf,FX7688);
		if(pStr != NULL)
		{
			hostVersion = FX7688_VERSION ;
			break;
		}

		hostVersion = FX7620N_UNKNOW_VERSION;
	}while(0);

	if(pStr != NULL)
	{
		ret = SUCCESS;
		log_debug("this is %s Board\n",pBuf);
	}
	
	fclose(fd);
	
	return ret;
}

int get_object_to_local_file(char *object_name)
{
	int ret = -1;
    aos_pool_t *p = NULL;
    aos_string_t bucket;
    aos_string_t object;
    int is_cname = 0;
    oss_request_options_t *options = NULL;
    aos_table_t *headers = NULL;
    aos_table_t *params = NULL;
    aos_table_t *resp_headers = NULL;
    aos_status_t *s = NULL;
    aos_string_t file;
	char fileName[128]={0};

	sprintf(fileName,"/var/%s",object_name);

    aos_pool_create(&p, NULL);
    options = oss_request_options_create(p);
    init_sample_request_options(options, is_cname);//这里有ENDPOINT、AccessKeyId 和AccessKeySecret初始化
    aos_str_set(&bucket, BUCKET_NAME);
    aos_str_set(&object, object_name);
    headers = aos_table_make(p, 0);
    aos_str_set(&file, fileName);

    s = oss_get_object_to_file(options, &bucket, &object, headers, 
                               params, &file, &resp_headers);
    if (aos_status_is_ok(s)) {
        log_debug("get object to local file succeeded\n");
		ret = 0;
    } else {
        log_debug("get object to local file failed\n");
		log_debug("s->error_code:%s,-----s->error_msg:%s\n",s->error_code,s->error_msg);
    }

End:
    aos_pool_destroy(p);
	return ret;
}

char updateHW_Update_Pthread(void* args)
{
	uint8_t vflag = (*((uint8_t*)args));
	char retVersion = 0;

	start_download = true;

	if(vflag == 1 || vflag == 0 )
	{
		if (aos_http_io_initialize(NULL, 0) != AOSE_OK)
    	{
			log_debug("aos_http_io_initialize FAILED\n");
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
			start_download = false;
			return -1;
    	}
		
		log_debug("SRPC_UpDateMasterHostFWReq++2\n");
		printf("SRPC_UpDateMasterHostFWReq++2\n");

		char fwfilename[FILE_NAME_SIZE] = {0};
		char md5filename[FILE_NAME_SIZE] = {0};
		char fwVerson[FILE_NAME_SIZE] = {0};
		//下载更新文件信息
		//if(updateHW_isFileExist(CONFIGFILE)==-1)
		{
			//下载oss云端保存更新文件版本、文件名、md5文件名等信息的配置文件
			if(-1 == get_object_to_local_file(CONFIGFILE))
			{
				log_debug("updateHW_DownLoadFiles Failed\n");
				printf("updateHW_DownLoadFiles Failed\n");
				SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
				start_download = false;
				aos_http_io_deinitialize();
				return -1;
			}
		}
		
		log_debug("SRPC_UpDateMasterHostFWReq++3\n");
		printf("SRPC_UpDateMasterHostFWReq++3\n");

		//获取版本、文件、下载地址信息
		if(-1 == updateHW_GetupdateInfo(fwVerson,fwfilename,md5filename))
		{
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
			start_download = false;
			aos_http_io_deinitialize();
			return -1;
		}
		
		log_debug("SRPC_UpDateMasterHostFWReq++4\n");
		printf("SRPC_UpDateMasterHostFWReq++4\n");
		//比较当前版本和远程版本
		
		retVersion = updateHW_VersionComplate(fwVerson,strlen(fwVerson));
		log_debug("Version = %d\n",retVersion);
		printf("Version = %d\n",retVersion);
		if(-1 == retVersion)
		{
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
			start_download = false;
			aos_http_io_deinitialize();
			return -1;
		}
		else if(0 == retVersion)
		{
			//固件版本相同
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_SAME_VERSION,0,0);
		}
		else if(1 == retVersion) //当前版本大于远程版本
		{
			SRPC_UpDateMasterHostFWCfm(YY_STATUS_DEGRADE_SUCCESS,0,0);
		}
		else //当前版本小于服务器版本
		{
			log_debug("SRPC_UpDateMasterHostFWReq++5\n");
			printf("SRPC_UpDateMasterHostFWReq++5\n");
			if(-1 == updateHW_System(vflag,fwfilename,md5filename))
			{
				SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
			}
		}
	}
	else
	{
		log_debug("SRPC_UpDateMasterHostFWReq++6\n");
		printf("SRPC_UpDateMasterHostFWReq++6\n");
		SRPC_UpDateMasterHostFWCfm(YY_STATUS_UPGRADE_FAIL,0,0);
	}
	
	start_download = false;
	aos_http_io_deinitialize();

	return -1;
}
