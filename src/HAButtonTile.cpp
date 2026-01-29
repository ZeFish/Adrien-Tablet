/*
 * Adrien-Tablet/src/HAButtonTile.cpp
 *
 * Implementation for HAButtonTile - a small, self-contained UI tile that
 * encapsulates a visual tile + toggle/button behavior + optional numeric
 * parameter controls + Home Assistant service calls.
 *
 * Usage:
 *   HAButtonTile::Config cfg;
 *   cfg.mode = HAButtonTile::MODE_TOGGLE_ENTITY;
 *   cfg.title = "Ceiling";
 *   cfg.subtitle = "Living Room";
 *   cfg.entity = "light.ceiling";
 *   cfg.icon_off = ImageResource_home_icon_light_off_92x92;
 *   cfg.icon_on  = ImageResource_home_icon_light_on_92x92;
 *   cfg.x = 2; cfg.y = 20; cfg.w = 228; cfg.h = 228;
 *
 *   HAButtonTile* tile = new HAButtonTile();
 *   tile->init(cfg);
 *   tile->attach();
 *
 */

#include "HAButtonTile.h"
#include "epdgui/epdgui.h"
#include "ha_api.h"
#include "resources/ImageResource.h"
#include <Arduino.h>

HAButtonTile::HAButtonTile() {
}

HAButtonTile::~HAButtonTile() {
    // Remove GUI objects from drawing list is not strictly necessary;
    // just delete allocated objects to free memory.
    if (_plus) {
        delete _plus;
        _plus = nullptr;
    }
    if (_minus) {
        delete _minus;
        _minus = nullptr;
    }
    if (_sw) {
        delete _sw;
        _sw = nullptr;
    }
}

void HAButtonTile::init(const Config &cfg) {
    _cfg = cfg;
    _param = cfg.paramInit;

    // Create the main switch that acts as toggle or one-shot button
    _sw = new EPDGUI_Switch(2, cfg.x, cfg.y, cfg.w, cfg.h);

    // Draw base background and labels (copy of previous InitSwitch style)
    // Background
    memcpy(_sw->Canvas(0)->frameBuffer(),
           ImageResource_home_button_background_228x228, cfg.w * cfg.h / 2);
    _sw->Canvas(0)->setTextSize(36);
    _sw->Canvas(0)->setTextDatum(TC_DATUM);
    _sw->Canvas(0)->drawString(_cfg.title, cfg.w / 2, 136);

    _sw->Canvas(0)->setTextSize(26);
    _sw->Canvas(0)->drawString(_cfg.subtitle, cfg.w / 2, 183);

    // Copy to second canvas and add icons for on/off
    memcpy(_sw->Canvas(1)->frameBuffer(), _sw->Canvas(0)->frameBuffer(), cfg.w * cfg.h / 2);

    // Draw icons (if provided)
    const uint8_t *iconOff = _cfg.icon_off ? _cfg.icon_off : ImageResource_home_icon_light_off_92x92;
    const uint8_t *iconOn  = _cfg.icon_on  ? _cfg.icon_on  : ImageResource_home_icon_light_on_92x92;
    _sw->Canvas(0)->pushImage(68, 20, 92, 92, iconOff);
    _sw->Canvas(1)->pushImage(68, 20, 92, 92, iconOn);

    // If numeric parameter is enabled, create plus/minus buttons and render their visual
    if (_cfg.paramIsNumeric) {
        // minus: left bottom
        int minusW = 116;
        int minusH = 44;
        int minusX = cfg.x;
        int minusY = cfg.y + cfg.h - minusH;
        _minus = new EPDGUI_Button(minusX, minusY, minusW, minusH);
        // draw normal background
        memcpy(_minus->CanvasNormal()->frameBuffer(), ImageResource_home_air_background_l_116x44, minusW * minusH / 2);
        memcpy(_minus->CanvasPressed()->frameBuffer(), _minus->CanvasNormal()->frameBuffer(), minusW * minusH / 2);
        _minus->CanvasPressed()->ReverseColor();
        _minus->SetCustomString("0"); // not used, but keep parity

        // plus: right bottom
        int plusW = 112;
        int plusH = 44;
        int plusX = cfg.x + cfg.w - plusW;
        int plusY = minusY;
        _plus = new EPDGUI_Button(plusX, plusY, plusW, plusH);
        memcpy(_plus->CanvasNormal()->frameBuffer(), ImageResource_home_air_background_r_112x44, plusW * plusH / 2);
        memcpy(_plus->CanvasPressed()->frameBuffer(), _plus->CanvasNormal()->frameBuffer(), plusW * plusH / 2);
        _plus->CanvasPressed()->ReverseColor();
        _plus->SetCustomString("0");

        // Small numeric display area - we will render param on canvas(1) in redraw()
        // Bind the plus/minus callbacks
        _minus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, _minus);
        _minus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, this);
        _minus->Bind(EPDGUI_Button::EVENT_RELEASED, _minus_cb);

        _plus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, _plus);
        _plus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, this);
        _plus->Bind(EPDGUI_Button::EVENT_RELEASED, _plus_cb);
    }

    // Bind switch events: pass _sw pointer and this pointer for instance methods
    _sw->AddArgs(0, 0, _sw);
    _sw->AddArgs(0, 1, this);
    _sw->Bind(0, _sw_cb);

    _sw->AddArgs(1, 0, _sw);
    _sw->AddArgs(1, 1, this);
    _sw->Bind(1, _sw_cb);

    // Initial draw (incl. parameter)
    redraw();
}

void HAButtonTile::attach() {
    if (_sw) {
        EPDGUI_AddObject(_sw);
    }
    if (_minus) {
        EPDGUI_AddObject(_minus);
    }
    if (_plus) {
        EPDGUI_AddObject(_plus);
    }
}

void HAButtonTile::setParam(int v) {
    if (!_cfg.paramIsNumeric) return;
    if (v < _cfg.paramMin) v = _cfg.paramMin;
    if (v > _cfg.paramMax) v = _cfg.paramMax;
    _param = v;
    redraw();
}

int HAButtonTile::getParam() const {
    return _param;
}

void HAButtonTile::redraw() {
    if (!_sw) return;

    // Redraw subtitle and param info on the active canvas (canvas 1)
    _sw->Canvas(1)->setTextSize(26);
    _sw->Canvas(1)->setTextDatum(TC_DATUM);

    // Clear subtitle area and redraw subtitle
    _sw->Canvas(1)->fillRect(0, 160, _cfg.w, 40, 0); // black background area
    _sw->Canvas(1)->drawString(_cfg.subtitle, _cfg.w / 2, 183);

    // If numeric param exists, show it prominently
    if (_cfg.paramIsNumeric) {
        _sw->Canvas(1)->setTextSize(36);
        _sw->Canvas(1)->setTextDatum(TC_DATUM);
        char buf[16];
        snprintf(buf, sizeof(buf), \"%d\", _param);
        _sw->Canvas(1)->drawString(String(buf), _cfg.w / 2, 108);
    }

    // Force update of canvas (no full refresh required)
    _sw->Canvas(1)->pushCanvas(_cfg.x, _cfg.y, UPDATE_MODE_A2);
}

void HAButtonTile::onPlusPressed() {
    setParam(_param + _cfg.paramStep);
}

void HAButtonTile::onMinusPressed() {
    setParam(_param - _cfg.paramStep);
}

void HAButtonTile::onSwitchChanged(int state) {
    // state is 0 or 1
    switch (_cfg.mode) {
    case MODE_TOGGLE_ENTITY: {
        // parse domain from entity: <domain>.<object_id>
        String entity = _cfg.entity;
        int dot = entity.indexOf('.');
        String domain = (dot > 0) ? entity.substring(0, dot) : String(\"light\");
        String service = (state == 1) ? String(\"turn_on\") : String(\"turn_off\");
        Serial.print(\"HAButtonTile: toggling entity \");
        Serial.print(entity);
        Serial.print(\" -> \");
        Serial.println(service);
        ha_call_service(domain, service, entity);
        break;
    }

    case MODE_SCRIPT_TOGGLE: {
        // Build variables JSON for ON/OFF
        String vars = (state == 1) ? _cfg.onVars : _cfg.offVars;
        if (vars.length() == 0 && _cfg.paramIsNumeric) {
            // Build numeric variable JSON using paramName
            vars = String(\"{\") + \"\\\"\" + _cfg.paramName + \"\\\":\" + String(_param) + String(\"}\");
        }

        String payload = String(\"{\\\"entity_id\\\":\\\"\") + _cfg.entity + String(\"\\\"\") ;
        if (vars.length()) {
            payload += String(\", \\\"variables\\\": \") + vars;
        }
        payload += String(\"}\");
        Serial.print(\"HAButtonTile: script toggle payload: \");
        Serial.println(payload);
        ha_call_service_payload(\"script\", \"turn_on\", payload);
        break;
    }

    case MODE_SCRIPT_BUTTON: {
        // One-shot: run script.turn_on when the switch is toggled ON, then reset switch back to OFF
        if (state == 1) {
            String vars = _cfg.onVars;
            if (vars.length() == 0 && _cfg.paramIsNumeric) {
                vars = String(\"{\") + \"\\\"\" + _cfg.paramName + \"\\\":\" + String(_param) + String(\"}\");
            }
            String payload = String(\"{\\\"entity_id\\\":\\\"\") + _cfg.entity + String(\"\\\"\") ;
            if (vars.length()) {
                payload += String(\", \\\"variables\\\": \") + vars;
            }
            payload += String(\"}\");
            Serial.print(\"HAButtonTile: script button payload: \");
            Serial.println(payload);
            ha_call_service_payload(\"script\", \"turn_on\", payload);

            // Revert switch state to 0 to act as a one-shot button
            if (_sw) {
                _sw->setState(0);
            }
        }
        break;
    }

    default:
        break;
    }

    // After an action, update visuals (e.g., show param)
    redraw();
}

/* ---------- static callbacks ---------- */

void HAButtonTile::_sw_cb(epdgui_args_vector_t &args) {
    // args[0] -> EPDGUI_Switch *, args[1] -> HAButtonTile *
    EPDGUI_Switch *sw = (EPDGUI_Switch *)(args[0]);
    HAButtonTile *self = (HAButtonTile *)(args[1]);
    if (!sw || !self) return;
    int state = sw->getState();
    self->onSwitchChanged(state);
}

void HAButtonTile::_plus_cb(epdgui_args_vector_t &args) {
    // args[0] -> btn, args[1] -> this
    HAButtonTile *self = (HAButtonTile *)(args[1]);
    if (!self) return;
    self->onPlusPressed();
}

void HAButtonTile::_minus_cb(epdgui_args_vector_t &args) {
    HAButtonTile *self = (HAButtonTile *)(args[1]);
    if (!self) return;
    self->onMinusPressed();
}