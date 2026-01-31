#ifndef _HA_API_H_
#define _HA_API_H_

#include <Arduino.h>

void ha_call_service(String domain, String service, String entity_id);
// Call Home Assistant service with an arbitrary JSON payload (e.g. to run scripts with parameters)
void ha_call_service_payload(String domain, String service, String payloadJson);
// Fetch current state of an entity. Returns empty string on failure.
// Fetch current state of an entity. Returns empty string on failure.
String ha_get_state(String entity_id);
// Fetch a specific attribute from an entity
String ha_get_attribute(String entity_id, String attribute);

#endif // _HA_API_H_
