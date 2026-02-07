#pragma once
#include "Arduino.h"

class HTTPClient {
public:
    void begin(String url) {
        std::cout << "[Mock] HTTPClient begin: " << url.s << std::endl;
    }
    void addHeader(String name, String value) {
        // std::cout << "[Mock] Header: " << name.s << ": " << value.s << std::endl;
    }
    int GET() { return 200; }
    int POST(String payload) {
        std::cout << "[Mock] POST payload: " << payload.s << std::endl;
        return 200;
    }
    String getString() { return "{\"state\":\"on\", \"attributes\": {}}"; }
    void end() {}
};
