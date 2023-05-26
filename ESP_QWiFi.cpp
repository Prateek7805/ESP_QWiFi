#include "ESP_QWiFi.h"

//_
bool ESP_QWiFi::_validateCreds(String ssid, String pass)
{
    if (ssid.length() == 0 || pass.length() == 0)
    {
        return false;
    }
    if (ssid.length() > 32)
    {
        return false;
    }
    if (pass.length() < 8 || pass.length() > 63)
    {
        return false;
    }
    return true;
}

bool ESP_QWiFi::_getCreds(void){
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS initialization unsuccessful");
        Serial.println("Resorting to default AP creds");
        return true;
    }
    //LittleFS.format(); // for testing only
    if (!LittleFS.exists(CREDS_PATH))
    {
        Serial.printf("No file found at %s\n", CREDS_PATH);
        Serial.println("Resorting to default AP credentials");
        return true;
    }

    File credsFile = LittleFS.open(CREDS_PATH, "r");
    if (!credsFile)
    {
        Serial.printf("Failed to read credentials file at %s\n", CREDS_PATH);
        Serial.println("Resorting to default AP creds");
        return true;
    }
    wc.mode = credsFile.readStringUntil('\n');
    wc.ap_ssid = credsFile.readStringUntil('\n');
    wc.ap_pass = credsFile.readStringUntil('\n');
    wc.sta_ssid[0] = credsFile.readStringUntil('\n');
    wc.sta_pass[0] = credsFile.readStringUntil('\n');
    wc.sta_ssid[1] = credsFile.readStringUntil('\n');
    wc.sta_pass[1] = credsFile.readStringUntil('\n');

    credsFile.close();

    return wc.mode == "AP"; // AP->true, STA->false
}

bool ESP_QWiFi::_saveCreds(void)
{
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS check unsuccessful");
        Serial.println("Try restarting the device");
        return false;
    }
    File credsFile = LittleFS.open(CREDS_PATH, "w");
    if (!credsFile)
    {
        Serial.printf("Failed to create file at %s\n", CREDS_PATH);
        return false;
    }
    credsFile.printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n"
                    ,wc.mode.c_str()
                    ,wc.ap_ssid.c_str()
                    ,wc.ap_pass.c_str()
                    ,wc.sta_ssid[0].c_str()
                    ,wc.sta_pass[0].c_str()
                    ,wc.sta_ssid[1].c_str()
                    ,wc.sta_pass[1].c_str()
    );
    credsFile.close();
    return true;
}

bool ESP_QWiFi::_parseCreds(uint8_t *data, String *ssid, String *pass){
    String body = (char *)data;
    int nsi = body.indexOf("\"ssid\":\"") + 8;
    if (nsi == -1)
    {
        return false;
    }
    int nei = body.indexOf("\",", nsi);
    if (nei == -1)
    {
        return false;
    }
    *ssid = body.substring(nsi, nei);
    nsi = body.indexOf("\"pass\":\"") + 8;
    if (nsi == -1)
    {
        return false;
    }
    nei = body.indexOf("\"}", nsi);
    if (nei == -1)
    {
        return false;
    }
    *pass = body.substring(nsi, nei);
    return true;
}

void ESP_QWiFi::_StartWiFiEvents(void){
    staGotIPHandler = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP &event)
                                              {
        _boot_wifi_conn_f = false;
        Serial.println("Connected to Wi-Fi sucessfully.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        _retr = 0;
        if(_server && !_server_started_f){
            _server->begin();
            _server_started_f = true;
            Serial.println("main server started");
            }
        });
    staDisconnectedHandler = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected &event)
                                                            {
        if(_retr < 5){
            Serial.printf("Disconnected from Wi-Fi Attempting reconnect : %d\n", _retr);
            _connectToAccessPoint();
            _retr++;
            return;
        }
        if(_boot_wifi_conn_f)
            wc.mode="AP";
        
        _saveCreds();
        ESP.restart(); 
    });
}

void ESP_QWiFi::_APServerDefinition(void)
{
    _ap_server = new AsyncWebServer(_port);
    _ap_server->on("/", HTTP_GET, [](AsyncWebServerRequest *req)
                   { req->send_P(200, "text/html", _index); });
    _ap_server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *req)
                   { req->send_P(200, "text/css", _styles); });
    _ap_server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *req)
                   { req->send_P(200, "application/javascript", _script); });
    _ap_server->on("/nw", HTTP_GET, [this](AsyncWebServerRequest *req){
        String resp = "[\"";
        resp += wc.sta_ssid[0]+"\",\"";
        resp += wc.sta_ssid[1]+"\"]";
        Serial.printf("Sending nw response : %s", resp.c_str());
        req->send(200, "application/json", resp); });
    _ap_server->on("/conn", HTTP_GET, [this](AsyncWebServerRequest *req){
        if(!req->hasParam("id")){
            req->send(400, "text/plain", "wrong parameter name");
            return;
        }
        int id = req->getParam("id")->value().toInt();
        if(id != 0 && id != 1){
            req->send(400, "text/plain", "invalid parameter");
            return;
        }
        String ssid = wc.sta_ssid[0];
        String pass = wc.sta_pass[0];
        wc.sta_ssid[0] = wc.sta_ssid[id];
        wc.sta_pass[0] = wc.sta_pass[id];
        wc.sta_ssid[id] = ssid;
        wc.sta_pass[id] = pass;
        wc.mode = "STA";
        if(!_saveCreds()){
                req->send(500, "text/plain", "Failed to save updated settings");
                return;
        }
        req->send(200, "text/plain", "ok");
        ESP.restart();
    });
    _ap_server->on("/reset", HTTP_GET, [this](AsyncWebServerRequest * req){
        if(!req->hasParam("t")){
            req->send(400, "text/plain", "wrong parameter name");
            return;
        }
        String type = req->getParam("t")->value();
        if(type == "AP"){
            wc.mode = "AP";
            wc.ap_ssid = Q_WIFI_DEFAULT_AP_SSID;
            wc.ap_pass = Q_WIFI_DEFAULT_AP_PASS;
            if(!_saveCreds()){
                req->send(500, "text/plain", "Failed to save updated settings");
                return;
            }
            req->send(200, "text/plain", "ok");
            ESP.restart();
        }
        else if(type == "ALL"){
            if(!LittleFS.begin()){
                req->send(500, "text/plain", "FS Init : Please restart the device");
                return;
            }
            if(!LittleFS.exists(CREDS_PATH)){
                req->send(404, "text/plain", "No saved credentials found");
                return;
            }
            if(!LittleFS.remove(CREDS_PATH)){
                req->send(500, "text/plain", "Failed to delete the saved credentials");
                return;
            }
            req->send(200, "text/plain", "ok");
            ESP.restart();
        }
        req->send(400, "text/plain", "invalid parameter");
        return;
    });
    _ap_server->onRequestBody([this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total){  
        if (len > 150){
            req->send(400, "text/plain", "body too large");
            return;
        }
        if (req->url() != "/update"){
            req->send(400, "text/plain", "bad request");
            return;
        }
        if (!req->hasParam("type")){   
            req->send(400, "text/plain", "wrong parameter name");
            return;
        }
        
        String type = req->getParam("type")->value();
        String ssid;
        String pass;
        
        if (!_parseCreds(data, &ssid, &pass)){
            req->send(400, "text/plain", "error parsing json body");
            return;
        }
        if(!_validateCreds(ssid, pass)){
            req->send(400, "text/plain", "bad ssid/password length");
            return;
        }
        if(type == "AP"){
            wc.ap_ssid = ssid;
            wc.ap_pass = pass;
            if(!_saveCreds()){
                req->send(500, "text/plain", "Failed to save AP credentials");
                return;
            }
            req->send(200, "text/plain", "AP updated, in effect after device restart");
            return;
        }
        if(type == "STA"){
            wc.sta_ssid[1] = wc.sta_ssid[0];
            wc.sta_pass[1] = wc.sta_pass[0];
            wc.sta_ssid[0] = ssid;
            wc.sta_pass[0] = pass;
            wc.mode = "STA";
            if(!_saveCreds()){
                req->send(500, "text/plain", "Failed to save STA credentials");
                return;
            }
            req->send(200, "text/plain", "STA updated attemping to connect to AP...");
            ESP.restart();
            return;
        }
        req->send(400, "text/plain", "invalid parameter"); });
    
}
void ESP_QWiFi::_startAP(void)
{
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(wc.ap_ssid, wc.ap_pass);
    delay(50);
    Serial.println("started Access point");
    Serial.println(WiFi.softAPIP());
    _ap_server->begin();
}
void ESP_QWiFi::_connectToAccessPoint(void)
{
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(wc.sta_ssid[0], wc.sta_pass[0]);
    Serial.printf("Attempting to connect to %s", wc.sta_ssid[0].c_str());
}

ESP_QWiFi::ESP_QWiFi(uint16_t port)
{
    _port = port;
}

void ESP_QWiFi::begin(AsyncWebServer *server)
{
    _server = server;
    _StartWiFiEvents();
    if (_getCreds())
    {
        _APServerDefinition();
        _startAP();
        return;
    }
    _connectToAccessPoint();
}