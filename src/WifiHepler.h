#ifndef _WIFI_HELPER_
#define _WIFI_HELPER_
const static int WIFI_CONFIG_STATE_NOCONFIG = 0;
const static int WIFI_CONFIG_STATE_CONNECTED = 1;


/**
 * 一键配网
*/
void smartConfig(std::function<void(char[], IPAddress)> connected);

/**
 * wifi连接
*/
int wifiConnect(std::function<void(char[], IPAddress)> connected);

#endif