#ifndef _HA_API_H_
#define _HA_API_H_

#include <Arduino.h>

void ha_call_service(String domain, String service, String entity_id);

#endif // _HA_API_H_
