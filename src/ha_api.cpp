#include "ha_api.h"
#include <HTTPClient.h>
#include "wifi_secrets.h"

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

            // If the service call succeeded, try to confirm the entity state via the States API
            if (httpResponseCode >= 200 && httpResponseCode < 300) {
                String expected = "";
                if (service == "turn_on") expected = "on";
                else if (service == "turn_off") expected = "off";

                Serial.println("Service call succeeded, checking entity state...");
                bool confirmed = false;
                String lastState = "";

                const int tries = 5;
                const int delayMs = 500;
                for (int i = 0; i < tries; ++i) {
                    HTTPClient stateHttp;
                    String stateUrl = String(HOME_ASSISTANT_URL) + "/api/states/" + entity_id;
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
                                        lastState = state;
                                        if (expected.length() && state == expected) {
                                            Serial.println("State confirmed.");
                                            confirmed = true;
                                            stateHttp.end();
                                            break;
                                        } else {
                                            Serial.println("State not yet as expected.");
                                        }
                                    } else {
                                        Serial.println("Failed to parse state value.");
                                        Serial.println(stateResp);
                                    }
                                } else {
                                    Serial.println("Failed to find opening quote for state.");
                                    Serial.println(stateResp);
                                }
                            } else {
                                Serial.println("Failed to find ':' after state.");
                                Serial.println(stateResp);
                            }
                        } else {
                            Serial.println("State response (no 'state' field found):");
                            Serial.println(stateResp);
                        }
                    } else {
                        Serial.print("Error fetching state (HTTP ");
                        Serial.print(stateCode);
                        Serial.println(")");
                    }
                    stateHttp.end();
                    delay(delayMs);
                }

                if (!confirmed) {
                    Serial.print("State not confirmed after ");
                    Serial.print(tries);
                    Serial.println(" attempts. Last known state: ");
                    Serial.println(lastState);
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

                Serial.println("Service call succeeded, checking entity state...");
                bool confirmed = false;
                String lastState = "";

                const int tries = 5;
                const int delayMs = 500;
                for (int i = 0; i < tries; ++i) {
                    HTTPClient stateHttp;
                    String stateUrl = String(HOME_ASSISTANT_URL) + "/api/states/" + entity;
                    stateHttp.begin(stateUrl);
                    stateHttp.addHeader("Authorization", String("Bearer ") + HOME_ASSISTANT_TOKEN);

                    int stateCode = stateHttp.GET();
                    if (stateCode == 200) {
                        String stateResp = stateHttp.getString();

                        // Lightweight extraction of the "state" field from the JSON response
                        int sidx = stateResp.indexOf("\"state\"");
                        if (sidx >= 0) {
                            int colon2 = stateResp.indexOf(":", sidx);
                            if (colon2 >= 0) {
                                int firstQuote = stateResp.indexOf("\"", colon2);
                                if (firstQuote >= 0) {
                                    int start = firstQuote + 1;
                                    int end = stateResp.indexOf("\"", start);
                                    if (end > start) {
                                        String state = stateResp.substring(start, end);
                                        Serial.print("Entity state: ");
                                        Serial.println(state);
                                        lastState = state;
                                        if (expected.length() && state == expected) {
                                            Serial.println("State confirmed.");
                                            confirmed = true;
                                            stateHttp.end();
                                            break;
                                        } else {
                                            Serial.println("State not yet as expected.");
                                        }
                                    } else {
                                        Serial.println("Failed to parse state value.");
                                        Serial.println(stateResp);
                                    }
                                } else {
                                    Serial.println("Failed to find opening quote for state.");
                                    Serial.println(stateResp);
                                }
                            } else {
                                Serial.println("Failed to find ':' after state.");
                                Serial.println(stateResp);
                            }
                        } else {
                            Serial.println("State response (no 'state' field found):");
                            Serial.println(stateResp);
                        }
                    } else {
                        Serial.print("Error fetching state (HTTP ");
                        Serial.print(stateCode);
                        Serial.println(")");
                    }
                    stateHttp.end();
                    delay(delayMs);
                }

                if (!confirmed) {
                    Serial.print("State not confirmed after ");
                    Serial.print(tries);
                    Serial.print(" attempts. Last known state: ");
                    Serial.println(lastState);
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
