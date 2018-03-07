/***********************************************************************************
 * �� �� ��   : net_utils.h
 * �� �� ��   : Edward
 * ��������   : 2016��11��4��
 * �ļ�����   : net������صĹ��߰�
 * ��Ȩ˵��   : Copyright (c) 2008-2016   xx xx xx xx �������޹�˾
 * ��    ��   : 
 * �޸���־   : 
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

