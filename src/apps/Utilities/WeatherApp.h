#ifndef _WEATHER_APP_H_
#define _WEATHER_APP_H_

#include "core/gui/AppBase.h"
#include "core/system/ha_api.h"
#include "core/config/SystemConfig.h"
#include "core/resources/ImageResource.h"
#include <SD.h>

class WeatherApp : public AppBase {
public:
    WeatherApp() : AppBase("Météo") {}

    const uint8_t* GetIcon() override;

    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) override;

private:
    void DrawAdvice(M5EPD_Canvas* canvas, String state);
};

#endif
