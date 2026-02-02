#include "core/system/ha_api.h"
#include <HTTPClient.h>
#include <list>
#include "core/system/wifi_secrets.h"

struct PendingCheck {
    String entity_id;
    String expected_state;
    unsigned long next_check_time;
    int attempts_left;
};

static std::list<PendingCheck> pendingChecks;

void ha_update() {
    #if defined(HOME_ASSISTANT_URL) && defined(HOME_ASSISTANT_TOKEN)
    if (pendingChecks.empty()) return;

    if (WiFi.status() != WL_CONNECTED) return;

    unsigned long now = millis();
    auto it = pendingChecks.begin();
    while (it != pendingChecks.end()) {
        if (now >= it->next_check_time) {
            bool remove = false;

            HTTPClient stateHttp;
            String stateUrl = String(HOME_ASSISTANT_URL) + "/api/states/" + it->entity_id;
            stateHttp.begin(stateUrl);
            stateHttp.addHeader("Authorization", String("Bearer ") + HOME_ASSISTANT_TOKEN);

            int stateCode = stateHttp.GET();
            if (stateCode == 200) {
                String stateResp = stateHttp.getString();
                // Robust-ish extraction of the "state" field from the JSON response
                int idx = stateResp.indexOf("\"state\"");
                if (idx >= 0) {
                    int colon = stateResp.indexOf(":", idx);
                    if (colon >= 0) {
                        int firstQuote = stateResp.indexOf("\"", colon);
                        if (firstQuote >= 0) {
                            int start = firstQuote + 1;
                            int end = stateResp.indexOf("\"", start);
                            if (end > start) {
                                String state = stateResp.substring(start, end);
                                Serial.print("Entity state: ");
                                Serial.println(state);
                                if (it->expected_state.length() && state == it->expected_state) {
                                    Serial.println("State confirmed.");
                                    remove = true;
                                } else {
                                    Serial.println("State not yet as expected.");
                                }
                            } else {
                                Serial.println("Failed to parse state value.");
                            }
                        } else {
                            Serial.println("Failed to find opening quote for state.");
                        }
                    } else {
                        Serial.println("Failed to find ':' after state.");
                    }
                } else {
                    Serial.println("State response (no 'state' field found).");
                }
            } else {
                Serial.print("Error fetching state (HTTP ");
                Serial.print(stateCode);
                Serial.println(")");
            }
            stateHttp.end();

            if (remove) {
                it = pendingChecks.erase(it);
            } else {
                it->attempts_left--;
                if (it->attempts_left <= 0) {
                    Serial.print("State not confirmed after attempts for ");
                    Serial.println(it->entity_id);
                    it = pendingChecks.erase(it);
                } else {
                    it->next_check_time = now + 500;
                    ++it;
                }
            }
        } else {
            ++it;
        }
    }
    #endif
}

void ha_call_service(String domain, String service, String entity_id) {
    #if defined(HOME_ASSISTANT_URL) && defined(HOME_ASSISTANT_TOKEN)
    int wifi_status = WiFi.status();
    Serial.print("Current WiFi Status: ");
    Serial.println(wifi_status);

    if (wifi_status != WL_CONNECTED) {
        Serial.println("WiFi was disconnected, attempting to reconnect...");
        #ifdef HARDCODED_WIFI_SSID
        WiFi.begin(HARDCODED_WIFI_SSID, HARDCODED_WIFI_PASSWORD);
        #else
        // Fallback to whatever was stored last or let WiFi library handle it
        WiFi.reconnect();
        #endif
        
        // This initial wait might still block, but it's for connection recovery
        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 20) {
            delay(500);
            timeout++;
            Serial.print(".");
        }
        Serial.println("");
    }

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(HOME_ASSISTANT_URL) + "/api/services/" + domain + "/" + service;
        
        String payload = "{\"entity_id\": \"" + entity_id + "\"}";
        
        // Debug output: show exact request
        Serial.print("HA URL: ");
        Serial.println(url);
        Serial.print("Payload: ");
        Serial.println(payload);

        http.begin(url);
        http.addHeader("Authorization", String("Bearer ") + HOME_ASSISTANT_TOKEN);
        http.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = http.POST(payload);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);

            // If the service call succeeded, queue a state check
            if (httpResponseCode >= 200 && httpResponseCode < 300) {
                String expected = "";
                if (service == "turn_on") expected = "on";
                else if (service == "turn_off") expected = "off";

                if (expected.length() > 0) {
                    Serial.println("Service call succeeded, queuing entity state check...");
                    PendingCheck check;
                    check.entity_id = entity_id;
                    check.expected_state = expected;
                    check.next_check_time = millis() + 500;
                    check.attempts_left = 5;
                    pendingChecks.push_back(check);
                }
            }
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

void ha_call_service_payload(String domain, String service, String payloadJson) {
    #if defined(HOME_ASSISTANT_URL) && defined(HOME_ASSISTANT_TOKEN)
    int wifi_status = WiFi.status();
    Serial.print("Current WiFi Status: ");
    Serial.println(wifi_status);

    if (wifi_status != WL_CONNECTED) {
        Serial.println("WiFi was disconnected, attempting to reconnect...");
        #ifdef HARDCODED_WIFI_SSID
            WiFi.begin(HARDCODED_WIFI_SSID, HARDCODED_WIFI_PASSWORD);
        #else
            // Fallback to whatever was stored last or let WiFi library handle it
            WiFi.reconnect();
        #endif

        int timeout = 0;
        while (WiFi.status() != WL_CONNECTED && timeout < 20) {
            delay(500);
            timeout++;
            Serial.print(".");
        }
        Serial.println("");
    }

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(HOME_ASSISTANT_URL) + "/api/services/" + domain + "/" + service;

        // Debug output: show exact request
        Serial.print("HA URL: ");
        Serial.println(url);
        Serial.print("Payload: ");
        Serial.println(payloadJson);

        http.begin(url);
        http.addHeader("Authorization", String("Bearer ") + HOME_ASSISTANT_TOKEN);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(payloadJson);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);

            // If possible, extract entity_id from payload to attempt state confirmation
            int idx = payloadJson.indexOf("\"entity_id\"");
            String entity = "";
            if (idx >= 0) {
                int colon = payloadJson.indexOf(':', idx);
                int q1 = payloadJson.indexOf('\"', colon);
                int q2 = payloadJson.indexOf('\"', q1 + 1);
                if (q2 > q1) {
                    entity = payloadJson.substring(q1 + 1, q2);
                }
            }

            // If we have an entity and the service was a turn_on/turn_off, try to confirm state
            if (entity.length() > 0 && (service == "turn_on" || service == "turn_off")) {
                String expected = "";
                if (service == "turn_on") expected = "on";
                else if (service == "turn_off") expected = "off";

                if (expected.length() > 0) {
                    Serial.println("Service call succeeded, queuing entity state check...");
                    PendingCheck check;
                    check.entity_id = entity;
                    check.expected_state = expected;
                    check.next_check_time = millis() + 500;
                    check.attempts_left = 5;
                    pendingChecks.push_back(check);
                }
            }
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
String ha_get_state(String entity_id) {
    #if defined(HOME_ASSISTANT_URL) && defined(HOME_ASSISTANT_TOKEN)
    if (WiFi.status() != WL_CONNECTED) return "";

    HTTPClient http;
    String url = String(HOME_ASSISTANT_URL) + "/api/states/" + entity_id;
    http.begin(url);
    http.addHeader("Authorization", String("Bearer ") + HOME_ASSISTANT_TOKEN);

    int httpCode = http.GET();
    String state = "";
    if (httpCode == 200) {
        String payload = http.getString();
        // Extract "state" field
        int idx = payload.indexOf("\"state\"");
        if (idx >= 0) {
            int colon = payload.indexOf(":", idx);
            int q1 = payload.indexOf("\"", colon);
            int q2 = payload.indexOf("\"", q1 + 1);
            if (q2 > q1) {
                state = payload.substring(q1 + 1, q2);
            }
        }
    }
    http.end();
    return state;
    #else
    return "";
    #endif
}

String ha_get_attribute(String entity_id, String attribute) {
    #if defined(HOME_ASSISTANT_URL) && defined(HOME_ASSISTANT_TOKEN)
    if (WiFi.status() != WL_CONNECTED) return "";

    HTTPClient http;
    String url = String(HOME_ASSISTANT_URL) + "/api/states/" + entity_id;
    http.begin(url);
    http.addHeader("Authorization", String("Bearer ") + HOME_ASSISTANT_TOKEN);

    int httpCode = http.GET();
    String value = "";
    if (httpCode == 200) {
        String payload = http.getString();
        
        // Find attributes block
        int attrBlock = payload.indexOf("\"attributes\"");
        if (attrBlock >= 0) {
            // Find specific attribute within attributes block
            int keyIdx = payload.indexOf("\"" + attribute + "\"", attrBlock);
            if (keyIdx >= 0) {
                int colon = payload.indexOf(":", keyIdx);
                // Check if number or string
                int valStart = colon + 1;
                while (payload[valStart] == ' ' || payload[valStart] == '\t') valStart++;
                
                if (payload[valStart] == '\"') {
                    // String value
                    int q1 = valStart;
                    int q2 = payload.indexOf("\"", q1 + 1);
                    if (q2 > q1) value = payload.substring(q1 + 1, q2);
                } else {
                    // Numeric value
                    int comma = payload.indexOf(",", valStart);
                    int brace = payload.indexOf("}", valStart);
                    int end = comma;
                    if (brace < comma || comma == -1) end = brace;
                    if (end > valStart) value = payload.substring(valStart, end);
                }
            }
        }
    }
    http.end();
    return value;
    #else
    return "";
    #endif
}
