#ifndef _WIFI_OPERATE_H_
#define _WIFI_OPERATE_H_
#ifdef __cplusplus
extern "C" {
#endif

int   wireless_ap_disable(bool st);
int   wireless_ap_setPasswd(char *encry,char *passwd);
int   wireless_ap_setSSID(char *ssid);
int   wireless_ap_setMode(char *mode);
int   wireless_radio_setChannel(char *ch);
int   gateway_airkiss_set(bool st);
char* gateway_airkiss_get(void);
char* gateway_server_getAddr(void);
char* gateway_server_getPort(void);
int   gateway_server_setPort(char* port);
int   gateway_server_setAddr(char *addr);
int   gateway_wireless_set(bool st);
char* gateway_wireless_get(void);
int   gateway_Binding_set(bool st);
char* gateway_Binding_get(void);
char* network_mac_get(void);
void  network_mac_free(char *obj);
void  wireless_sta_config(char *ssid,char *encry,char *passwd,char *channel);
int   gateway_server_config(char *addr,char* port);

#ifdef __cplusplus
}
#endif
#endif
