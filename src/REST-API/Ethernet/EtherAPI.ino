#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <WebServer.h>
#include <ArduinoJson.h>

//PHY接続の定義(RMII接続)
#define ETH_CLOCK_IN_PIN 0
#define ETH_TXD0_PIN 19
#define ETH_TXEN_PIN 21
#define ETH_TXD1_PIN 22
#define ETH_RXD0_PIN 25
#define ETH_RXD1_PIN 26
#define ETH_MODE2_PIN 27

//任意のPINに変更可能
#define ETH_MDIO_PIN 18
#define ETH_MDC_PIN 23
#define ETH_POWER_PIN 17

//EthernetICの設定
#define ETH_ADDR 1
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN

static bool eth_connected = false;

//サーバのポートを80に設定
WebServer server(80);

// 出力PIN
const int red_pin = 14;   
const int green_pin = 12; 

// PWN周波数、チャンネル、分解能の設定
const int frequency = 5000;
const int redChannel = 0;
const int greenChannel = 1;
const int resolution = 8;

//送受信するJSONの設定
StaticJsonDocument<250> jsonDocument;
char buffer[250];

//接続イベント
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    
    case 18:
      //ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED 
      //ソフト AP インターフェイスでプローブ要求パケットを受信

      Serial.println("ETH Started");

      //ホストネームの設定
      ETH.setHostname("IoT-murasame");
      break;
    case 20:
      //ARDUINO_EVENT_ETH_START
      //ESP32 イーサネット 開始

      Serial.println("ETH Connected (SYSTEM_EVENT_ETH_CONNECTED)");
      break;
    case 22:
      //ARDUINO_EVENT_ETH_CONNECTED  
      //ESP32 イーサネット phy リンクアップ

      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");

      eth_connected = true;
      break;

    case 5:
      //ARDUINO_EVENT_WIFI_STA_DISCONNECTED
      //ESP32 APから切断された

      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case 3:
      //ARDUINO_EVENT_WIFI_STA_STOP
      //ESP32 イーサネットがストップした
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

// REST-APIのルーティング設定
void setup_routing() {      
  // POST address/led + data(json)を受けたときのハンドラ
  server.on("/led", HTTP_POST, handlePost);    
  //サーバを開始する
  server.begin();    
}
//LED-POSTを受けたときのハンドラ
void handlePost() {
  if (server.hasArg("plain") == false) {
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  //JSON解釈
  //Body例
  // {
  //   "red":255,
  //   "green":100,
  // }
  int red_value = jsonDocument["red"];
  int green_value = jsonDocument["green"];
  //LEDのデューティーを値を変更
  ledcWrite(redChannel, red_value);
  ledcWrite(greenChannel,green_value);
  //ステータスとレスポンスを返す
  server.send(200, "application/json", "{}");
}

//初期化
void setup() {
    //シリアル通信のボーレート設定
    Serial.begin(115200);   
      
    //GPIOピンの初期設定
    ledcSetup(redChannel, frequency, resolution);
    ledcSetup(greenChannel, frequency, resolution);
    //ピンとPWMチャンネルをバインド 
    ledcAttachPin(red_pin, redChannel);
    ledcAttachPin(green_pin, greenChannel);

    delay(200);

    //ネットワークに接続
    WiFi.onEvent(WiFiEvent);
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE,ETH_CLK_MODE);
    
    //サーバ初期化
    setup_routing();  
}
void loop() {
  //リクエスト処理
  server.handleClient();     
}