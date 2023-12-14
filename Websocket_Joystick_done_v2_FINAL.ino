#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>

#define VRXPIN 35
#define VRYPIN 34

const char* ssid = "BANBAR";
const char* password = "r4h4512dong";

int interval = 1000;

// pakai www.textfixer.com
String web = "<!DOCTYPE html><html><head> <title>Arduino and ESP32 Websocket</title> <meta name='viewport' content='width=device-width, initial-scale=1.0' /> <meta charset='UTF-8'> <style> body { background-color: #E6D8D5; text-align: center; } </style></head><body> <h1>VRX: <span id='vrx'>-</span></h1> <h1>VRY: <span id='vry'>-</span></h1> <h1>Received message: <span id='message'>-</span></h1><button type='button' id='BTN_1'> <h1>ON</h1> </button><button type='button' id='BTN_2'> <h1>OFF</h1> </button></body><script> var Socket; document.getElementById('BTN_1').addEventListener('click', button_1_pressed); document.getElementById('BTN_2').addEventListener('click', button_2_pressed); function init() { Socket = new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage = function(event) { processCommand(event); }; } function processCommand(event) { var obj = JSON.parse(event.data); document.getElementById('message').innerHTML = obj.PIN_Status; document.getElementById('vrx').innerHTML = obj.Vrx; document.getElementById('vry').innerHTML = obj.Vry; console.log(obj.PIN_Status); console.log(obj.Vrx); console.log(obj.Vry); } function button_1_pressed() { Socket.send('1'); } function button_2_pressed() { Socket.send('0'); } window.onload = function(event) { init(); }</script></html>";

String jsonString;
String pin_status = "";
int vrx;
int vry;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void setup() {
  pinMode(2, OUTPUT); //LED
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP ADDRESS:");
  Serial.println(WiFi.localIP());

  server.on("/", [](){
    server.send(200, "text\html", web);
  });

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
  update_joystick_value();
  update_webpage();
}

// fungsi wajib, ini diset di awal dan akan diloop terus di websocket.loop(). Alhasil nonstop dan lebih cepat dari kecepatan delay millis yang printing data
void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length){
  switch(type){
    case WStype_DISCONNECTED:
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": Disconnected");
      break;
    case WStype_CONNECTED:
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": Connected");
      if(digitalRead(2) == HIGH){
        pin_status = "ON";
        update_webpage();
      }
      else{
        pin_status = "OFF";
        update_webpage();
      }
      break;
    case WStype_TEXT:
      Serial.println();
      Serial.print("payload 0 : ");
      Serial.println(payload[0]);
      if(payload[0] == '1'){
        pin_status = "ON";
        digitalWrite(2, HIGH);
      }
      if(payload[0] == '0'){
        pin_status = "OFF";
        digitalWrite(2, LOW);
      }
      break;
  }
}

void update_webpage(){
    StaticJsonDocument<100> doc;

    JsonObject object = doc.to<JsonObject>();
    object["PIN_Status"] = pin_status;
    object["Vrx"] = vrx;
    object["Vry"] = vry;
    serializeJson(doc, jsonString);
    Serial.println(jsonString);
    webSocket.broadcastTXT(jsonString);
    jsonString = "";
  }

  void update_joystick_value(){
    vrx = analogRead(VRXPIN);
    vry = analogRead(VRYPIN);
  }