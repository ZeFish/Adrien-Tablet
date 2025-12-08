#include "frame_home_callbacks.h"
#include "../ha_api.h"
#include "../wifi_secrets.h"

void key_home_air_adjust_cb(epdgui_args_vector_t &args) {
    int operation     = ((EPDGUI_Button *)(args[0]))->GetCustomString().toInt();
    EPDGUI_Switch *sw = ((EPDGUI_Switch *)(args[1]));
    if (sw->getState() == 0) {
        return;
    }
    int temp = sw->GetCustomString().toInt();
    char buf[10];
    if (operation == 1) {
        temp++;

    } else {
        temp--;
    }
    sprintf(buf, "%d", temp);
    sw->SetCustomString(buf);
    sprintf(buf, "%d℃", temp);
    sw->Canvas(1)->setTextSize(36);
    sw->Canvas(1)->setTextDatum(TC_DATUM);
    sw->Canvas(1)->fillRect(114 - 100, 108, 200, 38, 0);
    sw->Canvas(1)->drawString(buf, 114, 108);
    sw->Canvas(1)->pushCanvas(sw->getX(), sw->getY(), UPDATE_MODE_A2);
}

void key_home_air_state0_cb(epdgui_args_vector_t &args) {
    EPDGUI_Button *b1 = ((EPDGUI_Button *)(args[0]));
    EPDGUI_Button *b2 = ((EPDGUI_Button *)(args[1]));
    b1->SetEnable(false);
    b2->SetEnable(false);
}

void key_home_light_cb(epdgui_args_vector_t &args) {
    EPDGUI_Switch *sw = ((EPDGUI_Switch *)(args[0]));
    int state = sw->getState();
    #ifdef HOME_ASSISTANT_LIGHT_ENTITY
    if (state == 1) {
        ha_call_service("light", "turn_on", HOME_ASSISTANT_LIGHT_ENTITY);
    } else {
        ha_call_service("light", "turn_off", HOME_ASSISTANT_LIGHT_ENTITY);
    }
    #endif
}

void key_home_air_state1_cb(epdgui_args_vector_t &args) {
    EPDGUI_Button *b1 = ((EPDGUI_Button *)(args[0]));
    EPDGUI_Button *b2 = ((EPDGUI_Button *)(args[1]));
    b1->SetEnable(true);
    b2->SetEnable(true);
}
