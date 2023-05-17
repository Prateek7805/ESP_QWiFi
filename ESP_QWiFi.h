#ifndef _ESP_QWIFI_
    #define _ESP_QWIFI_
    #include <ESP8266WiFi.h>       // Include the ESP8266WiFi library
    #include <ESPAsyncWebServer.h> // Include the ESPAsyncWebServer library
    #include <ESPAsyncTCP.h>       // Include the ESPAsyncTCP library
    #include "LittleFS.h"

    #include "index.h"
    #include "styles.h"
    #include "script.h"

    #define CREDS_PATH "/wificreds.bin"
    #define Q_WIFI_DEFAULT_AP_SSID "ESP_QWiFi"
    #define Q_WIFI_DEFAULT_AP_PASS "12345678"
    class ESP_QWiFi{
        private:
            AsyncWebServer * _ap_server = NULL;
            AsyncWebServer * _server = NULL;
            WiFiEventHandler staDisconnectedHandler, staGotIPHandler;
            uint16_t _port = 80;
            struct wifiCreds{
                String mode = "AP";
                String ap_ssid = Q_WIFI_DEFAULT_AP_SSID;
                String ap_pass = Q_WIFI_DEFAULT_AP_PASS;
                String sta_ssid[2] = {"",""};
                String sta_pass[2] = {"",""};
            } wc;
            struct flags{
                bool _ap_server_started = false;
                bool _main_server = false;
                uint8_t _conn_count = 0;
            } _fl;
            uint8_t _retr = 0;
            //functions
            bool _validateCreds(String ssid, String pass);
            bool _getCreds(void);
            bool _saveCreds(void);
            bool _parseCreds(uint8_t * data, String * ssid, String * pass);
            
            void _StartWiFiEvents(void);
            void _APServerDefinition(void);
            void _startAP(void);
            void _startMainServer(void);
        public:
            ESP_QWiFi(uint16_t port);
            void begin(AsyncWebServer * server); 
    };


#endif _ESP_QWIFI_