#include <EEPROM.h>
#include "EepromConfig.h"

/*
 * 保存参数到eeprom
*/
void saveConfig(Configs *configs) {
  // 申请 0 - 127的地址
  EEPROM.begin(configs_addr + eeprom_size);
  uint8_t *p = (uint8_t*)(configs);
  for (int i = 0; i < sizeof(*configs); i++) {
    EEPROM.write(i, *(p + i));
  }
  EEPROM.commit();
  // 释放内存
  delete configs;
}

/*
 * 获取配置
*/
Configs* loadConfigs() {
  // 为变量请求内存
  Configs *pvalue  = new Configs;   
  EEPROM.begin(configs_addr + eeprom_size);
  uint8_t *p = (uint8_t*)(pvalue);
  for (int i = 0; i < sizeof(*pvalue); i++) {
    *(p + i) = EEPROM.read(i);
  }
  EEPROM.commit();
  return pvalue;
}

/**
 * 清空配置
*/
void clearConfig() {
  EEPROM.begin(configs_addr + eeprom_size);
  // 这里为啥是96 ，因为在结构体声明的长度之和就是96
  for (int i = 0; i < eeprom_size; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}