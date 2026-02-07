#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>

// Mock millis and delay
extern unsigned long _mock_millis;
inline unsigned long millis() { return _mock_millis; }
inline void delay(unsigned long ms) {
    // For benchmark, we want to measure time, so we sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    _mock_millis += ms;
}

// Mock String
class String {
public:
    std::string s;
    String() {}
    String(const char* str) : s(str ? str : "") {}
    String(std::string str) : s(str) {}
    String(int v) : s(std::to_string(v)) {}

    int length() const { return s.length(); }
    int indexOf(const String& str, int from = 0) const {
        size_t pos = s.find(str.s, from);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    int indexOf(char c, int from = 0) const {
        size_t pos = s.find(c, from);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    String substring(int start, int end) const {
        if (start >= s.length()) return "";
        return String(s.substr(start, end - start));
    }
    String substring(int start) const {
        if (start >= s.length()) return "";
        return String(s.substr(start));
    }

    const char* c_str() const { return s.c_str(); }

    String operator+(const String& other) const { return String(s + other.s); }
    String operator+(const char* other) const { return String(s + other); }
    bool operator==(const String& other) const { return s == other.s; }
    bool operator==(const char* other) const { return s == other; }
    bool operator!=(const char* other) const { return s != other; }

    char operator[](int index) const {
        if (index >= 0 && index < s.length()) return s[index];
        return 0;
    }
};

inline String operator+(const char* lhs, const String& rhs) { return String(lhs) + rhs; }

// Mock Serial
class MockSerial {
public:
    void print(const String& s) { std::cout << s.s; }
    void print(int n) { std::cout << n; }
    void print(const char* s) { std::cout << s; }
    void println(const String& s) { std::cout << s.s << std::endl; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(int n) { std::cout << n << std::endl; }
    void println() { std::cout << std::endl; }
};
extern MockSerial Serial;

// Mock WiFi
#define WL_CONNECTED 3
#define WL_DISCONNECTED 6
#define WL_IDLE_STATUS 0

class MockWiFi {
public:
    int _status = WL_DISCONNECTED;
    int status() { return _status; }
    void reconnect() {
        std::cout << "[Mock] WiFi.reconnect() called" << std::endl;
    }
    void begin(const char* ssid, const char* pass) {
         std::cout << "[Mock] WiFi.begin(" << ssid << ") called" << std::endl;
    }
};
extern MockWiFi WiFi;
