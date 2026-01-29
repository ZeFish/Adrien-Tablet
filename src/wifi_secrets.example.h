#ifndef WIFI_SECRETS_EXAMPLE_H
#define WIFI_SECRETS_EXAMPLE_H

/*
 * Example wifi_secrets file for Adrien-Tablet
 *
 * Instructions:
 *  - Copy this file to `src/wifi_secrets.h`
 *  - Replace the placeholder values with your Wi‑Fi credentials and Home Assistant info.
 *  - Do NOT commit `src/wifi_secrets.h` (it's already in .gitignore).
 *
 * Notes:
 *  - HOME_ASSISTANT_URL is typically "http://homeassistant.local:8123" for local installs,
 *    or the full URL if you use a different hostname / https.
 *  - HOME_ASSISTANT_TOKEN must be a Long-Lived Access Token from your HA profile.
 *  - Entity IDs are the entity_id values from Home Assistant (e.g., light.kitchen).
 *  - For script toggles you can encode a special binding in the switch custom string:
 *      "script.entity|<onVarsJson>|<offVarsJson>"
 *    Example (commented below shows proper escaping for a literal C string).
 */

//
// Wi-Fi (optional): define these if you want to hardcode SSID/PSK instead of using the UI
//
// #define HARDCODED_WIFI_SSID "YourNetworkSSID"
// #define HARDCODED_WIFI_PASSWORD "YourNetworkPassword"

//
// Home Assistant connection
//
#define HOME_ASSISTANT_URL   "http://homeassistant.local:8123"   // replace if needed
#define HOME_ASSISTANT_TOKEN "REPLACE_WITH_LONG_LIVED_ACCESS_TOKEN"

//
// Example entity bindings you can define here.
// You can either define a single fallback entity macro (HOME_ASSISTANT_LIGHT_ENTITY)
// or per-tile macros (HOME_ASSISTANT_LIGHT1_ENTITY, HOME_ASSISTANT_LIGHT2_ENTITY, etc.)
//

#define HOME_ASSISTANT_LIGHT_ENTITY    "light.example_light"     // fallback (used by older code)
#define HOME_ASSISTANT_LIGHT1_ENTITY   "light.ceiling"
#define HOME_ASSISTANT_LIGHT2_ENTITY   "light.bedside_lamp"

#define HOME_ASSISTANT_SOCKET1_ENTITY  "switch.pc"
#define HOME_ASSISTANT_SOCKET2_ENTITY  "switch.dehumidifier"

//
// Scripts:
//  - If you want a switch to toggle a script and provide different variables for ON/OFF,
//    encode it as: "script.my_script|<onVarsJson>|<offVarsJson>"
//
//  Example (as a macro you could paste into wifi_secrets.h). Note the escaped quotes:
//
// #define HOME_ASSISTANT_SCRIPT_TOGGLE "script.goodnight|{\\\"mode\\\":\\\"night\\\"}|{\\\"mode\\\":\\\"day\\\"}"
//
//  Or for a one-shot script button payload (payload is a JSON string):
//
// #define HOME_ASSISTANT_SCRIPT_PAYLOAD "{\"entity_id\":\"script.goodnight\",\"variables\":{\"room\":\"all\"}}"
//
// Usage examples in your code:
//  - For a toggleable script bound to a switch:
//      ha_bind_entity_switch_to(_sw_x, "script.goodnight|{\"mode\":\"night\"}|{\"mode\":\"day\"}");
//  - For a one-shot script button:
//      ha_bind_script_button(_btn, "script.goodnight", "{\"room\":\"all\"}");
//

//
// Optional: define any additional entities you want to reference by macro names:
// #define HOME_ASSISTANT_SCENE_X "scene.evening_mode"
// #define HOME_ASSISTANT_ALARM "alarm_control_panel.house"
//

// Security reminder:
// - Keep `src/wifi_secrets.h` private. Do NOT commit it to source control.
// - Use Home Assistant Long‑Lived Access Tokens instead of your account password.

#endif // WIFI_SECRETS_EXAMPLE_H