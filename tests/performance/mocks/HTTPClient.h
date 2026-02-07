#pragma once
#include "Arduino.h"
#include "WiFi.h"

class HTTPClient {
public:
    void begin(String url) {}
    void addHeader(String key, String value) {}
    int GET() {
        // Simulate network delay
        delay(200);
        return 200;
    }
    int POST(String payload) {
        // Simulate network delay
        delay(200);
        return 200;
    }
    String getString() {
        return "{\"state\": \"on\"}";
    }
    void end() {}
};
