#include "Arduino.h"
#include "core/system/ha_api.h"

// Define globals
MockSerial Serial;
MockWiFi WiFi;
unsigned long _mock_millis = 0;

// Include the source file directly
#include "../src/core/system/ha_api.cpp"

int main() {
    Serial.println("Starting Benchmark...");

    // Test 1: WiFi Disconnected - Measure blocking time
    WiFi._status = WL_DISCONNECTED;

    unsigned long start = millis();
    ha_call_service("light", "turn_on", "light.kitchen");
    unsigned long duration = millis() - start;

    Serial.print("Duration with WiFi Disconnected: ");
    Serial.print((int)duration);
    Serial.println(" ms");

    if (duration < 500) {
        Serial.println("Result: NO BLOCKING DETECTED (Success!)");
    } else {
        Serial.println("Result: BLOCKING DETECTED (Failure)");
    }

    // Verify Queueing
    // We cannot easily access static serviceQueue from here unless we make it extern or friend,
    // or rely on side effects.
    // The side effect is: when we connect WiFi and call ha_update(), it should print "Processing queued service call..."

    Serial.println("Testing Queue Processing...");
    WiFi._status = WL_CONNECTED;

    // ha_update calls perform_service_call which uses HTTPClient mock.
    // We expect it to process the request we just queued.
    ha_update();

    return 0;
}
