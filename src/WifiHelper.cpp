#include <ESP8266WiFi.h>
#include <Arduino.h>
#include "WifiHepler.h"
#include "EepromConfig.h"

/**
 * 一键配网
*/
void smartConfig(std::function<void(char*, IPAddress)> connected) {
  // Serial.println("\n\rwait smart config wifi");
  WiFi.beginSmartConfig();
  // Serial.println("\n\rsmart config wifi started");
  while (1) {
    delay(500);
    if (WiFi.smartConfigDone()) {
      // Serial.println("\n\rsmartConfigDone");
      // Serial.printf("\n\rssid: %s, pwd: %s\n\r", WiFi.SSID().c_str(), WiFi.psk().c_str());
      Configs *configs  = new Configs;
      strcpy(configs->ssid, WiFi.SSID().c_str());
      strcpy(configs->pwd, WiFi.psk().c_str());
      // 设置自动连接
      WiFi.setAutoConnect(true);
      connected(configs->ssid, WiFi.localIP());
      // 记住wifi账号密码
      saveConfig(configs);
      break;
    }
  }
}

/**
 * 自动连接wifi
*/
int wifiConnect(std::function<void(char*, IPAddress)> connected) {
  Configs* configs = loadConfigs();
  // Serial.println(configs->pwd);
  if (WiFi.status() != WL_CONNECTED && strlen(configs->pwd) > 0 ) {
    // Serial.println("\n\ruse password connect wifi");
    // 如果说有账号密码信息,那就直接用账号密码连接wifi
    WiFi.mode(WIFI_STA);
    // Serial.printf("\n\rssid: %s, pwd: %s\n\r", configs->ssid, configs->pwd);
    WiFi.begin(configs->ssid, configs->pwd);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
    // Serial.printf("\n\rssid: %s, ip: %s\n\r", configs->ssid, WiFi.localIP().toString());
    // 设置自动连接
    WiFi.setAutoConnect(true);
    
    connected(configs->ssid, WiFi.localIP());
    // 删除内存
    delete configs;
    return WIFI_CONFIG_STATE_CONNECTED;
  } else {
    return WIFI_CONFIG_STATE_NOCONFIG;
  }
}