#pragma once

#define WL_CONNECTED 3

class WiFiClass {
public:
    int status() { return WL_CONNECTED; }
    void begin(const char* ssid, const char* password) {}
    void reconnect() {}
};

static WiFiClass WiFi;
