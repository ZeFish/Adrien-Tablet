#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <chrono>
#include <algorithm>

class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s) {}
    String(const std::string& s) : std::string(s) {}
    String(int i) : std::string(std::to_string(i)) {}
    String(long i) : std::string(std::to_string(i)) {}

    int indexOf(const String& str, int fromIndex = 0) const {
        size_t pos = this->find(str, fromIndex);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    int indexOf(char c, int fromIndex = 0) const {
        size_t pos = this->find(c, fromIndex);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    String substring(int beginIndex, int endIndex = -1) const {
        if (beginIndex < 0) beginIndex = 0;
        if (beginIndex >= (int)length()) return "";
        if (endIndex == -1 || endIndex > (int)length()) endIndex = length();
        return this->substr(beginIndex, endIndex - beginIndex);
    }
};

inline unsigned long millis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

inline void delay(unsigned long ms) {
    usleep(ms * 1000);
}

class SerialMock {
public:
    void begin(long) {}
    void print(const String& s) { std::cout << s; }
    void print(const char* s) { std::cout << s; }
    void print(int s) { std::cout << s; }
    void println(const String& s) { std::cout << s << std::endl; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(int s) { std::cout << s << std::endl; }
    void println() { std::cout << std::endl; }
};

static SerialMock Serial;
