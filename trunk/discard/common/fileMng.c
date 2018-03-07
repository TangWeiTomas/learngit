/***********************************************************
    ����˵����ϵͳ���÷�ʽ�ļ�����
    author: linux.sir@qq.com

***********************************************************/
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "fileMng.h"
#include "logUtils.h"

#include "config_operate.h"

/*****************************************************************************
 * �� �� ��  : wifiConfigbackups
 * �� �� ��  : Edward
 * ��������  : 2016��3��31��
 * ��������  : ����������������ļ�
 * �������  : void  ��
 * �������  : ��
 * �� �� ֵ  : 
 * ���ù�ϵ  : 
 * ��    ��  : 

*****************************************************************************/
void wifiConfigbackups(void)
{
	struct stat st;
    memset(&st,0,sizeof(st));

	//����wireless
	if(isFileExist("/etc/config/wireless.ap-only"))//����
	{
		system("cp /etc/config/wireless.ap-only /etc/config/wireless");
	}
	else //������
	{
		system("cp /etc/config/wireless /etc/config/wireless.ap-only");
	}
	
	//����network
	if(isFileExist("/etc/config/network.bak"))
	{
		 system("cp /etc/config/network.bak /etc/config/network");
	}
	else
	{
		 system("cp /etc/config/network /etc/config/network.bak");
	}
	
}

/**********************************************************

    �������ܣ�������
./rt5350_sta XC901 zxb123456zxb psk2
**********************************************************/
void wififileConfig(char* ssidInput, char* pwdInput,char authNum)
{
    int fd;

    fd = openfile("/etc/config/wireless");

    char buf[] = "\n\nconfig wifi-iface\n\toption device 'radio0'\n\toption mode 'sta'\n\toption network 'wan'";
    writefile(fd,buf);

    char ssid[50] = "\n\toption ssid ";
    strcat(ssid, ssidInput);
    writefile(fd,ssid);

    char password[50] = "\n\toption key ";
    
    if (authNum != AUTH_OPEN || authNum != AUTH_SHARED)
    {
    	strcat(password, pwdInput);
    	writefile(fd,password);
	}
	
    char auth[50] = "\n\toption encryption ";
    switch(authNum)
    {
        case AUTH_OPEN:
        {
            strcat(auth, "none");
        }
        break;
        case AUTH_SHARED:
        {
            strcat(auth, "none");
        }
        break;
        case AUTH_PSK:
        {
            strcat(auth, "psk");
        }
        break;
        case AUTH_PSK2:
        {
            strcat(auth, "psk2");
        }
        break;
        default:
            break;
    }

    writefile(fd,auth);

    closefile(fd);

}

/**********************************************************

    �������ܣ�������
./rt5350_sta XC901 zxb123456zxb psk2
**********************************************************/
void wifiServerInfofileConfig(uint8_t* serverIPAddr, uint16_t serverPort)
{
    int fd;
    char portString[8] = {0};
    struct stat st;
    memset(&st,0,sizeof(st));
    //�ж�û��wireless.bak �ļ��򱸷�һ��
#ifdef OPENWRT_TEST

    if(!stat("/etc/config/serverInfo",&st))
    {
        system("rm -rf /etc/config/serverInfo");
    }

    fd = openfile("/etc/config/serverInfo");

#else

    if(!stat("./serverInfo",&st))
    {
        system("rm -rf ./serverInfo");
    }

    fd = openfile("./serverInfo");
#endif

    writefile(fd,(char*)serverIPAddr);
    writefile(fd,",");

    //����תΪ�ַ���
    sprintf(portString, "%d", serverPort);

    writefile(fd,portString);

    closefile(fd);
}

/**********************************************************

    �������ܣ���ȡserverfile�ļ���IP��ַ�Ͷ˿ں�

**********************************************************/
int getWifiServerInfofile(char* address, uint16_t *port)
{

	ASSERT(address != NULL && port != NULL);

	int ret = -1;
	char *pAddr = NULL;
	char *pPort = NULL;

	do{
		pAddr = gateway_server_getAddr();
		if(pAddr == NULL)
		{
			ret = -1;
			break;
		}

		memcpy(address,pAddr,strlen(pAddr));
		
		pPort = gateway_server_getPort();
		if(pPort == NULL)
		{
			ret = -1;
			break;
		}
		*port = atoi(pPort);
		ret = 0;
	}while(0);
	
	if(pAddr != NULL)
		gateway_server_free(pAddr);

	if(pPort != NULL)
		gateway_server_free(pPort);

	return ret;
#if 0
    int fd;
    char buf[128] = {0};
    int len,cnt;
//	char *pstr = NULL;

    struct stat st;
    memset(&st,0,sizeof(st));

#ifdef OPENWRT_TEST
    char *path = "/etc/config/serverInfo";
#else
    char *path = "./serverInfo";
#endif

    if(stat(path,&st))
    {
        return -1;
    }

    fd = openfile(path);
    if(-1 == fd)
    {
        return -1;
    }

	len = read_line(fd,buf,sizeof(buf));
//	log_debug("%s\n",buf);
    if(len<0)
    {
        log_err("read failed for %s\n", strerror(errno));
        closefile(fd);
        return -1;;
    }
    
	/*
	pstr = strtok(buf,",");
	if(NULL != pstr)
	{
		memset(address,0,sizeof(address));
		memcpy(address,pstr,strlen(pstr));
		//printf("Addr:%s\n",serverIPAddr);
	}

	pstr = strtok(NULL,",");
	if(NULL != pstr)
	{
		memset(port,0,sizeof(port));
		*port = strtol(pstr,NULL,10);
		//printf("Port:%d\n",*serverPort);
	}
	*/
	
    for(cnt=0; cnt<len; cnt++)
    {
        if(buf[cnt] != ',' )
        {
            *address = buf[cnt];
            address++;
        }
		else if(buf[cnt]=='#')
		{
			break;
		}
        else
        {
            *port = atoi(&buf[cnt+1]);
            break;
        }
    }

    closefile(fd);
#endif
    return 0;
}

int getWifiServerInfo(char* serverIPAddr, char *serverPort)
{
    int fd;
    char buf[128] = {0};
    int len;
	char *pstr = NULL;

    struct stat st;
    memset(&st,0,sizeof(st));

#ifdef OPENWRT_TEST
    char *path = "/etc/config/serverInfo";
#else
    char *path = "./serverInfo";
#endif

    if(stat(path,&st))
    {
        return -1;
    }

    fd = openfile(path);
    if(-1 == fd)
    {
        return -1;
    }

	len = read_line(fd,buf,sizeof(buf));
    //len = read( fd, buf, 30);
    if(len<0)
    {
        log_err("read failed for %s\n", strerror(errno));
        closefile(fd);
        return -1;;
    }

	pstr = strtok(buf,",");
	if(NULL != pstr)
	{
		bzero(serverIPAddr,sizeof(serverIPAddr));
		bcopy(pstr,serverIPAddr,strlen(pstr));
	}
	
	pstr = strtok(NULL,",");
	if(NULL != pstr)
	{
		bzero(serverPort,sizeof(serverPort));
		bcopy(pstr,serverPort,strlen(pstr)-1);
	}

	/*
    for(cnt=0; cnt<len; cnt++)
    {
        if(buf[cnt] != ',' )
        {
            *serverIPAddr = buf[cnt];
            serverIPAddr++;
        }
		else if(buf[cnt]=='#')
		{
			break;
		}
        else
        {
            *serverPort = atoi(&buf[cnt+1]);
            break;
        }
    }
	*/
	
    closefile(fd);
    return 0;
}

/**********************************************************

    �������ܣ�ϵͳ���� open����

**********************************************************/
int openfile(const char *pfile)
{
    int fd;
    fd =  open(pfile, O_CREAT|O_RDWR|O_APPEND, S_IRWXU);
    if(fd==-1)
    {
        log_err("open error %s\n", strerror(errno));
        return -1;
    }
    return fd;
}


//true:1
//false:0
bool isFileExist(const char *file)
{
	ASSERT(file!=NULL);
	
	struct stat st;
	memset(&st,0,sizeof(st));
	
	if(!stat(file,&st))
	{
		//�и��ļ�
		return true;
	}	
	//δ��ȡ�����ļ���������Ϣ
	return false;
}

/**********************************************************

    �������ܣ�ϵͳ���� close()����

**********************************************************/
int closefile(int fd)
{
    int r = close(fd);
    if(r == -1)
    {
        log_err("close error for %s\n", strerror(errno));
        exit(1);
    }
 
    return r ;
}


/**********************************************************

    �������ܣ��ļ���ȡreadfile()����

**********************************************************/
int readfile(int fd)
{
    char buf[512];
    int len = read( fd, buf, 512);
    if(len<0)
    {
        log_err("read failed for %s\n", strerror(errno));
       return len;
    }
    log_debug("read result:\n%s", buf);
	return len;
}

/**********************************************************

    �������ܣ��ļ�д��  write()����

**********************************************************/
int writefile(int fd,char buf[])
{
    int len = write( fd, buf, strlen(buf));
    if(len==-1)
    {
        log_err("write error for %s\n", strerror(errno));
    }
    return len;
}


int read_line(int fd, void *vptr, int maxlen)
{
    int n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++)
    {
        again:
        if ((rc = read(fd, &c, 1)) == 1)
        {
            *ptr++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0)
        {
            *ptr = 0;
            return(n - 1);
        } 
        else
        {
            if (errno == EINTR)
                goto again ;
            return(-1);
        }
    }
    *ptr = 0;
    return(n);
}

int writeBinaryFile(const char* _fileName, void* _buf, int _bufLen)
{
    FILE * fp = NULL;
    size_t size = 0;
    if( NULL == _buf || _bufLen <= 0 ) return (-1);

    log_debug("writeBinaryFile++\n");
	
	log_debug("_fileName:%s\n",_fileName);
	
    fp = fopen(_fileName, "w+b"); // ����ȷ������ ������д�����ʽ��
	
    if( NULL == fp )
    {
        return (-1);
    }

    size = fwrite(_buf, _bufLen, 1, fp); //������д

    fclose(fp);
    fp = NULL;
	log_debug("writeBinaryFile--\n");

    return size;    
}

int readBinaryFile(const char* _fileName, void* _buf, int _bufLen)
{
    FILE* fp = NULL;
    size_t size = 0;
    if( NULL == _buf || _bufLen <= 0 ) return (-1);

    fp = fopen(_fileName, "rb"); // ����ȷ������ �����ƶ�ȡ����ʽ�� 

    if( NULL == fp )
    {
        return (-1);
    }

    fread(_buf, _bufLen, 1, fp); // �����ƶ�

    fclose(fp);
    return size;        
}


