/*
 * ha_widgets.cpp
 *
 * Helpers to bind EPDGUI switches / buttons to Home Assistant services.
 *
 * See ha_widgets.h for usage notes.
 */

#include "ha_widgets.h"
#include "core/system/ha_api.h"
#include "core/gui/epdgui/epdgui.h"
#include <Arduino.h>

// Small fixed table for button service bindings (keeps things simple on embedded)
static const int MAX_BTN_BINDINGS = 16;
struct BtnBinding {
    EPDGUI_Button *btn;
    String domain;
    String service;
    String payload;
};
static BtnBinding g_btnBindings[MAX_BTN_BINDINGS];
static int g_btnBindingCount = 0;

/*
 * Callback when a bound switch changes state.
 * args[0] is expected to be the EPDGUI_Switch* (per convention used elsewhere).
 *
 * Supported binding formats in sw->GetCustomString():
 *  - "domain.entity"                  (e.g. "light.living_room") -> domain.turn_on/turn_off
 *  - "script.entity|onVars|offVars"   (onVars/offVars are JSON objects) -> calls script.turn_on with variables
 */
static void ha_entity_switch_cb(epdgui_args_vector_t &args) {
    EPDGUI_Switch *sw = ((EPDGUI_Switch *)(args[0]));
    if (!sw) return;

    int state = sw->getState(); // 0 or 1
    String binding = sw->GetCustomString();

    if (binding.length() == 0) {
        Serial.println("ha_entity_switch_cb: no binding configured on switch");
        return;
    }

    // If binding contains '|' - treat it as script entity with optional on/off payloads:
    int p1 = binding.indexOf('|');
    if (p1 >= 0) {
        // script form: script_entity | onVarsJson | offVarsJson
        String scriptEntity = binding.substring(0, p1);
        int p2 = binding.indexOf('|', p1 + 1);
        String onVars = "";
        String offVars = "";
        if (p2 >= 0) {
            onVars = binding.substring(p1 + 1, p2);
            offVars = binding.substring(p2 + 1);
        } else {
            onVars = binding.substring(p1 + 1);
        }

        // Only support script.* in this path (but allow any domain if user tries)
        String domain = "script";
        String service = "turn_on";

        String vars = (state == 1) ? onVars : offVars;
        String payload = "";
        payload += "{\"entity_id\":\"";
        payload += scriptEntity;
        payload += "\"";
        if (vars.length()) {
            payload += ", \"variables\":";
            payload += vars;
        }
        payload += "}";

        Serial.print("ha_entity_switch_cb: calling script with payload: ");
        Serial.println(payload);
        ha_call_service_payload(domain, service, payload);
        return;
    }

    // Otherwise treat as normal <domain>.<object_id>
    int dot = binding.indexOf('.');
    if (dot <= 0) {
        Serial.print("ha_entity_switch_cb: invalid entity binding: ");
        Serial.println(binding);
        return;
    }
    String domain = binding.substring(0, dot);
    String entity = binding;
    String service = (state == 1) ? "turn_on" : "turn_off";

    Serial.print("ha_entity_switch_cb: domain=");
    Serial.print(domain);
    Serial.print(" service=");
    Serial.print(service);
    Serial.print(" entity=");
    Serial.println(entity);

    ha_call_service(domain, service, entity);
}

/*
 * Convenience: set the switch custom string and bind it.
 */
void ha_bind_entity_switch_to(EPDGUI_Switch *sw, const String &entityAndParams) {
    if (!sw) return;
    sw->SetCustomString(entityAndParams);
    ha_bind_entity_switch(sw);
}

/*
 * Bind a switch to be a Home Assistant entity controller.
 * This will bind the internal callback for both ON and OFF transitions.
 */
void ha_bind_entity_switch(EPDGUI_Switch *sw) {
    if (!sw) return;
    // Ensure the callback is registered for both states (0 and 1)
    sw->AddArgs(0, 0, sw);
    sw->Bind(0, ha_entity_switch_cb);
    sw->AddArgs(1, 0, sw);
    sw->Bind(1, ha_entity_switch_cb);
}

/*
 * Button callback for generic service buttons.
 * Looks up the button in the binding table and calls ha_call_service_payload.
 */
static void ha_service_button_cb(epdgui_args_vector_t &args) {
    EPDGUI_Button *btn = ((EPDGUI_Button *)(args[0]));
    if (!btn) return;

    for (int i = 0; i < g_btnBindingCount; ++i) {
        if (g_btnBindings[i].btn == btn) {
            Serial.print("ha_service_button_cb: calling ");
            Serial.print(g_btnBindings[i].domain);
            Serial.print("/");
            Serial.print(g_btnBindings[i].service);
            Serial.print(" with payload: ");
            Serial.println(g_btnBindings[i].payload);
            ha_call_service_payload(g_btnBindings[i].domain, g_btnBindings[i].service, g_btnBindings[i].payload);
            return;
        }
    }

    Serial.println("ha_service_button_cb: no binding found for button");
}

/*
 * Bind a button to perform a Home Assistant service with a pre-built JSON payload.
 */
void ha_bind_service_button_payload(EPDGUI_Button *btn, const String &domain, const String &service, const String &payloadJson) {
    if (!btn) return;

    if (g_btnBindingCount >= MAX_BTN_BINDINGS) {
        Serial.println("ha_bind_service_button_payload: too many bindings");
        return;
    }

    g_btnBindings[g_btnBindingCount].btn = btn;
    g_btnBindings[g_btnBindingCount].domain = domain;
    g_btnBindings[g_btnBindingCount].service = service;
    g_btnBindings[g_btnBindingCount].payload = payloadJson;
    ++g_btnBindingCount;

    // store payload in custom string as well for debugging convenience
    btn->SetCustomString(payloadJson);

    btn->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, btn);
    btn->Bind(EPDGUI_Button::EVENT_RELEASED, ha_service_button_cb);
}

/*
 * Convenience wrapper to bind a button to a script.
 * variablesJson should be JSON object text (e.g. '{\"mode\":\"night\"}') or empty.
 */
void ha_bind_script_button(EPDGUI_Button *btn, const String &scriptEntity, const String &variablesJson) {
    if (!btn) return;

    String payload = "{\"entity_id\":\"";
    payload += scriptEntity;
    payload += "\"";
    if (variablesJson.length()) {
        payload += ", \"variables\":";
        payload += variablesJson;
    }
    payload += "}";

    ha_bind_service_button_payload(btn, "script", "turn_on", payload);
}