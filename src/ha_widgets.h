#ifndef _HA_WIDGETS_H_
#define _HA_WIDGETS_H_

#include <Arduino.h>

/*
 * HA Widget helpers
 *
 * Small collection of helpers to bind EPDGUI switches/buttons to Home Assistant
 * services. The design favors convention-over-configuration:
 *
 * - For entity switches, we store a binding in the switch's custom string:
 *     sw->SetCustomString("domain.entity")
 *   Example: "light.living_room"
 *
 * - Script toggles / script parameters can be encoded in the custom string:
 *     "script.some_script|{\"on\": { <vars> }}|{\"off\": { <vars> }}"
 *   That is: script_entity | onVarsJson | offVarsJson, where onVarsJson and
 *   offVarsJson are JSON objects containing variables to send to the script.
 *
 * - For one-shot service calls (buttons), we provide helpers that store the
 *   payload or script entity on the button and call the appropriate service
 *   when the button is released.
 *
 * Notes:
 * - These helpers call `ha_call_service(...)` or `ha_call_service_payload(...)`
 *   (from `ha_api.h`) to perform the actual HTTP calls. Make sure you have
 *   `HOME_ASSISTANT_URL` and `HOME_ASSISTANT_TOKEN` configured in
 *   `src/wifi_secrets.h`.
 *
 * Example usage (in InitComponents()):
 *
 *   // Bind a light (toggle)
 *   _sw_light1->SetCustomString("light.ceiling");
 *   ha_bind_entity_switch(_sw_light1);
 *
 *   // Bind a script as toggle with parameters for on/off
 *   _sw_light2->SetCustomString("script.goodnight|{\"mode\":\"night\"}|{\"mode\":\"day\"}");
 *   ha_bind_entity_switch(_sw_light2);
 *
 *   // Bind a one-shot script button:
 *   ha_bind_script_button(_btn_goodnight, "script.goodnight", "{\"room\":\"all\"}");
 *
 */

class EPDGUI_Switch;
class EPDGUI_Button;
struct epdgui_args_vector_t;

/**
 * Bind a switch to a Home Assistant entity/service.
 *
 * Behavior:
 *  - Reads binding from sw->GetCustomString()
 *  - If binding is "domain.entity" -> domain.turn_on / domain.turn_off
 *  - If binding starts with "script." and includes | separated JSON parts:
 *       "script.entity|onVarsJson|offVarsJson"
 *    then toggling ON will call script.turn_on with the onVarsJson variables,
 *    toggling OFF will call script.turn_on with the offVarsJson variables.
 *
 * After calling the service, the helper will attempt a lightweight state
 * confirmation (when an entity_id is available and the service is turn_on/off).
 */
void ha_bind_entity_switch(EPDGUI_Switch *sw);

/**
 * Convenience: set the switch custom string and bind it.
 * `entityAndParams` can be either "domain.entity" or the script form above.
 */
void ha_bind_entity_switch_to(EPDGUI_Switch *sw, const String &entityAndParams);

/**
 * Bind a button to call an arbitrary Home Assistant service with a JSON payload.
 *
 * The payloadJson should be a ready-to-send JSON object string, e.g.:
 *   "{\"entity_id\":\"script.goodnight\",\"variables\":{\"mode\":\"night\"}}"
 *
 * When the button is released, this will POST to:
 *   POST <HOME_ASSISTANT_URL>/api/services/<domain>/<service>
 * with the body `payloadJson`.
 */
void ha_bind_service_button_payload(EPDGUI_Button *btn,
                                    const String &domain,
                                    const String &service,
                                    const String &payloadJson);

/**
 * Bind a button to call a script (convenience wrapper).
 *
 * This will produce a payload like:
 *   {"entity_id":"<scriptEntity>", "variables": <variablesJson> }
 *
 * If variablesJson is empty, it will call:
 *   {"entity_id":"<scriptEntity>"}
 */
void ha_bind_script_button(EPDGUI_Button *btn,
                           const String &scriptEntity,
                           const String &variablesJson);

#endif // _HA_WIDGETS_H_