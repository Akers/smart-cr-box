#ifndef _EEPROM_CONFIG_
#define _EEPROM_CONFIG_

const static int configs_addr = 0;
const static int eeprom_size = 128;

struct Configs {
  char ssid[32];
  char pwd[64];
  float pwm_rate;
};

/*
 * 保存参数到eeprom
*/
void saveConfig(Configs *configs);

/*
 * 获取配置
*/
Configs* loadConfigs();

/**
 * 清空配置
*/
void clearConfig();

#endif