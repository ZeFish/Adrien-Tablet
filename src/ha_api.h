#ifndef _HA_API_H_
#define _HA_API_H_

#include <Arduino.h>

void ha_call_service(String domain, String service, String entity_id);
// Call Home Assistant service with an arbitrary JSON payload (e.g. to run scripts with parameters)
void ha_call_service_payload(String domain, String service, String payloadJson);

#endif // _HA_API_H_
