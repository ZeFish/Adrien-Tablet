#include <M5EPD.h>
#include "core/gui/epdgui/epdgui.h"
#include "core/AppRegistry.h"
#include "core/system/systeminit.h"

void setup() {
    pinMode(M5EPD_MAIN_PWR_PIN, OUTPUT);
    M5.enableMainPower();

    SysInit_Start();
}

void loop() {
    EPDGUI_MainLoop();
}
