#ifndef _STATUS_BAR_H_
#define _STATUS_BAR_H_

#include <M5EPD.h>
#include "core/resources/ImageResource.h"
#include "core/config/SystemConfig.h"
#include <WiFi.h>

class StatusBar {
public:
    StatusBar();
    ~StatusBar();
    void draw(M5EPD_Canvas* canvas);
    void update(void);
    void SetTransparent(bool transparent);
    void SetAppMode(bool isAppMode);
    int getHeight() { return 56; }

private:
    M5EPD_Canvas *_canvas;
    M5EPD_Canvas *_canvas_bg;
    uint32_t _time;
    uint32_t _next_update_time;
    bool _is_transparent = true;
    bool _is_app_mode = false;
};

#endif //_STATUS_BAR_H_
