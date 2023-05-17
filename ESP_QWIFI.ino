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

void loop() {
  
}
