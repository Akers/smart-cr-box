#ifndef _SMART_CR_BOX_CONFIG_
#define _SMART_CR_BOX_CONFIG_
// 0.96寸oled显示屏SPI引脚
#define PIN_SPI_SDA 4 
#define PIN_SPI_SCL 5
// EC11引脚定义
#define PIN_ENCODER_A 14
#define PIN_ENCODER_B 12
#define PIN_ENCODER_BTN 13
// PWM信号输出
#define PIN_PWM_OUT 15

// 是否启用自动配网
#define SMART_CONFIG_ENABLED 1
// 关闭自动配网后有效，wifi ssid和密码
// #define WIFI_SSID "ssid"
// #define WIFI_PWD "pwd"

// 是否启用MQTT集成
#define MQTT_ENABLED 1
// MQTT服务器地址
#define MQTT_SERVER_ADDRESS "10.10.10.250"
// MQTT服务器端口
#define MQTT_SERVER_PORT 1883
// MQTT服务认证信息
#define MQTT_USER "akers_home"
#define MQTT_PWD "117315Akers"

// Config订阅主题
#define MQTT_TOPIC_CONFIG "SMART-CR-BOX/config"
// 设置开关主题
#define MQTT_TOPIC_SETSWITCH "SMART-CR-BOX/switch/set"
// 设置转速主题
#define MQTT_TOPIC_SETREV "SMART-CR-BOX/rev/set"
// 模式转速主题
#define MQTT_TOPIC_SETMODE "SMART-CR-BOX/mode/set"
// 开关状态主题
#define MQTT_TOPIC_SWITCH_STATE "SMART-CR-BOX/switch/state"
// 转速状态主题
#define MQTT_TOPIC_REV_STATE "SMART-CR-BOX/rev/state"
// 模式状态主题
#define MQTT_TOPIC_MODE_STATE "SMART-CR-BOX/mode/state"

// 标识当前设备的客户端编号
#define MQTT_CLIENT_ID "client-smart-cr-box"
#define MQTT_DEVICE_NAME "esp8266-smart-cr-box"

#endif