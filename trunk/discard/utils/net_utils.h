/***********************************************************************************
 * 文 件 名   : net_utils.h
 * 负 责 人   : Edward
 * 创建日期   : 2016年11月4日
 * 文件描述   : net网络相关的工具包
 * 版权说明   : Copyright (c) 2008-2016   xx xx xx xx 技术有限公司
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/
#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__
#ifdef __cplusplus
extern "C" {
#endif
typedef enum{
	NETUTILS_SUCC,
	NETUTILS_FAILE,
	NETUTILS_UP,
	NETUTILS_DOWN
}netUtils_state_t;

int netUtils_isConnect(const char *eth);
int netUtils_isDetect(const char* eth);
int netUtils_getHostMacByUci(char resultMac[6]);
int netUtils_getHostMacByIoctl(const char *device,char resultMac[6]);
int netUtils_getHostAddrs(char *host);

#ifdef __cplusplus
}
#endif
#endif

