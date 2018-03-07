#ifndef _WIFI_IWINFO_H_
#define _WIFI_IWINFO_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	char *device;
	char *ssid;
	char *pwd;
	char *encry;
	char channel;
}wfInfo_t;

wfInfo_t *wifi_iwinfoScan(const char*device,const char*ssid,const char*passwd);
void wifi_iwinfoDestroy(wfInfo_t *wfInfo);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
