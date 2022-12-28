#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "SmartCrBoxConfig.h"
#include "icons.h"
#include "EepromConfig.h"
#include "WifiHepler.h"

char save_config_flag = '0';

int encoder_btn_state = 0;
// 
long encoder_btn_press_time = 0;
// 上次发布状态时间
long last_pub_state = 0;
long last_active_time = 0;


U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/PIN_SPI_SCL, /*data=*/PIN_SPI_SDA, /*reset=*/U8X8_PIN_NONE);   

static float pwm_rate = 100.0;
String switch_state;
static uint8_t powerSave = 0;

int flag = 0;  //标志位
boolean CW_1 = 0;
boolean CW_2 = 0;

long last_pwm_changed = 0;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

/**
 * 发布mqtt状态
*/
void pub_mqtt_state() {
  char a[20];
  sprintf(a, "%g", pwm_rate);
  mqttClient.publish(MQTT_TOPIC_REV_STATE, a);
  mqttClient.publish(MQTT_TOPIC_SWITCH_STATE, switch_state.c_str());
}

/**
 * 设置转速百分比
*/
void setRev(float newVal) {
  if (newVal > 100) {
    pwm_rate = 100;
  } else if (newVal < 0) {
    pwm_rate = 0; 
  } else {
    pwm_rate = newVal;
  }
  pub_mqtt_state();
  last_active_time = millis();
}
void setRev(const char* newVal) {
  String s = newVal;
  if (!s.equals("STOP")) {
    setRev(atof(newVal));
  }
}

/**
 * 显示转速百分比
*/
void displayRev() {
  u8g2.setFont(u8g2_font_7Segments_26x42_mn);
  u8g2.firstPage();
  do
  {
    
    if (pwm_rate >= 100) {
      u8g2.setCursor(5, 61); //指定显示位置
    } else if (pwm_rate >= 10) {
      u8g2.setCursor(35, 61); //指定显示位置
    } else if (pwm_rate >= 0) {
      u8g2.setCursor(65, 61); //指定显示位置
    }
    // u8g2.drawXBM(5, 42, 16, 16, icon_fan_16);
    u8g2.printf("%d", (int)pwm_rate); //使用print来显示字符串
    u8g2.setCursor(100, 61);
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.print("%");
  } while (u8g2.nextPage());
}

void displayOFF() {
  u8g2.setFont(u8g2_font_fub42_tf);
  u8g2.firstPage();
  do
  {
    
    u8g2.setCursor(20, 61); //指定显示位置
    u8g2.print("OFF");
  } while (u8g2.nextPage());
}

/**
 * 显示wifi连接中页面
*/
void displayWifiConnecting() {
  u8g2.firstPage();
  do
  {
    u8g2.setCursor(5, 32);
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.print("wifi connecting...");
  } while (u8g2.nextPage());
}

/**
 * 显示wifi连接成功页面
*/
void displayWifiConnected(char *ssid, IPAddress ipAddress) {
  u8g2.firstPage();
  do
  {
    u8g2.setCursor(5, 3);
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.print("wifi connected!");
    u8g2.setCursor(5, 16);
    u8g2.printf("ssid: %s", ssid);
    u8g2.setCursor(5, 32);
    u8g2.printf("ip: %s", ipAddress.toString());
  } while (u8g2.nextPage());
}

/**
 * 自动配网页面
*/
void displaySmartConfig() {
  u8g2.firstPage();
  do
  {
    u8g2.setCursor(5, 32);
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.print("waiting SmartConfig...");
  } while (u8g2.nextPage());
}

/**
 * 旋转编码器中断
*/
IRAM_ATTR void encoder_a_inter() {
  // 只要处理一个脚的外部中断--上升沿&下降沿
  int alv = digitalRead(PIN_ENCODER_A);
  int blv = digitalRead(PIN_ENCODER_B);
  if (flag == 0 && alv == LOW) {
    CW_1 = blv;
    flag = 1;
  }
  if (flag && alv) {
    CW_2 = !blv;  //取反是因为 alv,blv必然异步，一高一低。
    // Serial.printf("CW_1: %d, CW_2: %d\n",CW_1, CW_2);
    if (CW_1 && CW_2) {
      setRev(pwm_rate + 1);
    }
    if (CW_1 == false && CW_2 == false) {
      setRev(pwm_rate - 1);
    }
    
    save_config_flag = '1';
    last_pwm_changed = millis();
    flag = 0;
  }
}

IRAM_ATTR void encoder_btn_inter() {
  int btn = digitalRead(PIN_ENCODER_BTN);
  Serial.printf("\nencoder btn state: %d\n", btn);
  if (btn == LOW) {
    long spand = millis() - encoder_btn_press_time;
    // 长按大于500ms
    if (spand > 3000) {
      // 长按
      if (switch_state.equals("ON")) {
        clearConfig();
      }
    } else {
      // 短按
      if (switch_state.equals("OFF")) {
        u8g2.setPowerSave(false);
        switch_state = "ON";
      }
      if (powerSave == 1) {
        powerSave = 0;
      }
      last_active_time = millis();
    } 
  } else {
    encoder_btn_press_time = millis();
  }

}

/**
 * wifi连接成功回调
*/
IRAM_ATTR void wifiConnectedCallback(char *ssid, IPAddress ipAddress) {
  displayWifiConnected(ssid, ipAddress);
  delay(1500);
}

/**
 * mqtt监听回调
*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived ["); 
  Serial.print(topic);   
  Serial.print("] ");

  String set_str = "";
  for (int i = 0; i < length; i++) {
    set_str += (char)payload[i];
  }  

  String topic_str = topic;
  Serial.println(set_str);

  if (topic_str.equals(MQTT_TOPIC_SETREV)) {
    setRev(set_str.c_str());
  }
  if (topic_str.equals(MQTT_TOPIC_SETSWITCH)) {
    switch_state = set_str;
    Serial.println("MQTT_TOPIC_SETSWITCH:");
    Serial.println(switch_state);
    last_active_time = millis();
    mqttClient.publish(MQTT_TOPIC_SWITCH_STATE, switch_state.c_str());
  }
}

/**
 * wifi和mqtt自动重连
*/
void reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi is not connected, retry setup");
    wifiConnect(wifiConnectedCallback);
  }
    
  while (!mqttClient.connected()) {
    Serial.println("MQTT connecting...");
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PWD)) {              
      Serial.println("MQTT connect success.");  
      mqttClient.subscribe(MQTT_TOPIC_SETREV);
      mqttClient.subscribe(MQTT_TOPIC_SETSWITCH);
    } else {
      delay(1000);  
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  switch_state = "ON";
  powerSave = 0;
  Serial.begin(115200);
  pinMode(PIN_ENCODER_A, INPUT_PULLUP);
  pinMode(PIN_ENCODER_B, INPUT_PULLUP);
  //编码器旋转中断
  attachInterrupt(PIN_ENCODER_A, encoder_a_inter, CHANGE);
  attachInterrupt(PIN_ENCODER_BTN, encoder_btn_inter, CHANGE);
  // 初始化u8g2
  u8g2.begin();
  u8g2.enableUTF8Print(); // enable UTF8 support for the Arduino print() function
  // clearConfig();
  // wifi 
  displayWifiConnecting();

  uint8_t wifi_state = wifiConnect(wifiConnectedCallback);

  if (WIFI_CONFIG_STATE_NOCONFIG == wifi_state) {
    displaySmartConfig();
    smartConfig(wifiConnectedCallback);
  }

  // 读取配置
  Configs* configs = loadConfigs();
  // 
  if (configs->pwm_rate != NULL && configs->pwm_rate > 0) {
    // 如果已经保存
    pwm_rate = configs->pwm_rate;
  } else {
    // 未保存则先100%转速
    pwm_rate = 100;
  }

  // 初始化mqtt
  mqttClient.setServer(MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT);
  mqttClient.setCallback(callback);
  pub_mqtt_state();

  // 开机先输出一个pwm激活调速模块
  analogWriteFreq(100);
  analogWrite(PIN_PWM_OUT, 200);
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  // wifi 和 mqtt掉线重连
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  // 保存配置
  if (save_config_flag == '1' && (millis() - last_pwm_changed > 1000)) {
    Configs* configs = loadConfigs();
    configs->pwm_rate = pwm_rate;
    saveConfig(configs);
    save_config_flag = '0';
    // Serial.println("Configs saved!");
  }

  // 发布状态
  if (last_pub_state <= 0 || (millis() - last_pub_state > 5000)) {
    last_pub_state = millis();
    pub_mqtt_state();
  }

  // 60秒无操作关闭屏幕
  if (millis() - last_active_time > 60000) {
    powerSave = 1;
  }

  if (powerSave == 1) {
    u8g2.setPowerSave(true);
  } else {
    u8g2.setPowerSave(false);
  }
  if (switch_state.equals("OFF")) {
    analogWrite(PIN_PWM_OUT, 0);
    displayOFF();
  } else {
    analogWrite(PIN_PWM_OUT, (int) (255 * (pwm_rate / 100.00) + 0.5));
    displayRev();
  }
  delay(500);
}