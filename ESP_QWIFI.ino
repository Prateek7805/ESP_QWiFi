#include<ESP8266WiFi.h>
#include "ESP_QWiFi.h"
#include <ESPAsyncWebServer.h>

ESP_QWiFi qw(80);
AsyncWebServer server(80);
void setup() {
  Serial.begin(115200);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * req){
    req->send(200, "text/plain", "message from server");
  });
  qw.begin(&server);
}

//for testing
unsigned long reset_mode_timer = millis();
void loop() {
  if(millis() - reset_mode_timer > 100){
    if(!digitalRead(0)){
      LittleFS.format();
      ESP.restart();
    }
    reset_mode_timer = millis();
  }
}
