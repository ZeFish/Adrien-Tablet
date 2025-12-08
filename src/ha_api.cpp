#include "ha_api.h"
#include <HTTPClient.h>
#include "wifi_secrets.h"

void ha_call_service(String domain, String service, String entity_id) {
    #if defined(HOME_ASSISTANT_URL) && defined(HOME_ASSISTANT_TOKEN)
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(HOME_ASSISTANT_URL) + "/api/services/" + domain + "/" + service;
        
        http.begin(url);
        http.addHeader("Authorization", String("Bearer ") + HOME_ASSISTANT_TOKEN);
        http.addHeader("Content-Type", "application/json");
        
        String payload = "{\"entity_id\": \"" + entity_id + "\"}";
        
        int httpResponseCode = http.POST(payload);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }
        
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
    #else
    Serial.println("Home Assistant credentials not defined in wifi_secrets.h");
    #endif
}
