#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

//WiFiのSSIDとPASSWORDを定義
const char *SSID = "YOUR_SSID";
const char *PWD = "YOUR_PASSORD";
//出力PINを定義
const int red_pin = 5;   
const int green_pin = 18; 
//PWMの設定
const int frequency = 5000;
const int redChannel = 0;
const int greenChannel = 1;
const int resolution = 8;

//サーバのポートを80に設定
WebServer server(80);
 
//送受信するJSONの設定
StaticJsonDocument<250> jsonDocument;
char buffer[250];


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

void setup() {     
  //シリアル通信のボーレート設定
  Serial.begin(115200);   
    
  //GPIOピンの初期設定
  ledcSetup(redChannel, frequency, resolution);
  ledcSetup(greenChannel, frequency, resolution);
  //ピンとPWMチャンネルをバインド 
  ledcAttachPin(red_pin, redChannel);
  ledcAttachPin(green_pin, greenChannel);
         
  //WiFiに接続
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());
  setup_routing();     
   
}    
       
void loop() {    
  server.handleClient();     
}