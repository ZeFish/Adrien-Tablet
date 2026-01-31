#include "StatusBar.h"
#include "core/system/global_setting.h"

StatusBar::StatusBar() {
    _canvas = new M5EPD_Canvas(&M5.EPD);
    _canvas->createCanvas(540, getHeight());
    _canvas->setTextSize(26);

    _canvas_bg = new M5EPD_Canvas(&M5.EPD);
    _canvas_bg->createCanvas(540, getHeight());

    _time = 0;
    _next_update_time = 0;

    bool custom_wallpaper = false;
    
    // Check runtime custom wallpaper
    if (global_custom_wallpaper_path.length() > 0) {
        if (DrawCustomWallpaper(_canvas_bg, global_custom_wallpaper_path.c_str())) {
            custom_wallpaper = true;
        }
    }

    #ifdef SYSTEM_CONFIG_SLEEP_IMAGE_PATH
    if (!custom_wallpaper && DrawCustomWallpaper(_canvas_bg, SYSTEM_CONFIG_SLEEP_IMAGE_PATH)) {
        custom_wallpaper = true;
    }
    #endif

    if (!custom_wallpaper) {
        _canvas_bg->pushImage(0, 0, 540, 960, GetWallpaper());
    }
}

StatusBar::~StatusBar() {
    delete _canvas;
    delete _canvas_bg;
}

void StatusBar::draw(M5EPD_Canvas* canvas) {
    if ((millis() - _time) < _next_update_time) {
        _canvas->pushCanvas(0, 0, UPDATE_MODE_NONE);
        return;
    }

    char buf[20];
    
    _canvas_bg->pushToCanvas(0, 0, _canvas);
    
    // Check if we are transparent (wallpaper) or opaque (white bg)
    // A simple check is to look at the first pixel of canvas_bg/framebuffer, but setting a member var is cleaner.
    // For now, let's assume if it is NOT wallpaper, it is white.
    // Actually, we can just set text color to 15 (white) for wallpaper and 0 (black) for white bg.
    // However, the cleanest way without refactoring too much is to check the Draw mode.
    // Easier: We added SetTransparent. Let's add a member bool _is_transparent; to track.
    
    
    // Check if we are transparent (wallpaper) or opaque (black bg)
    // Check if we are transparent (wallpaper) or opaque (black bg)
    // Check if we are transparent (wallpaper) or opaque (black bg)
    // Check if we are transparent (wallpaper) or opaque (black bg)
    if (_is_app_mode) {
        _canvas->setTextColor(15); // White text on black bg
        // Ensure strictly black background if opaque
        _canvas->fillRect(0, 0, 540, 56, 0);
        
        _canvas->setTextDatum(CL_DATUM);
        _canvas->setTextSize(26); // Revert to known working size
        _canvas->drawString("Back", 64, 28); // Vertically centered (56/2 = 28)
        _canvas->pushImage(20, 12, 32, 32, ImageResource_item_icon_arrow_l_32x32); // Centered (56-32)/2 = 12
    } else {
        _canvas->setTextColor(15); 
        _canvas->setTextDatum(CL_DATUM);
        _canvas->setTextSize(26); 
        _canvas->drawString(SYSTEM_CONFIG_DEVICE_NAME, 20, 28);
    }

    // Battery
    _canvas->setTextDatum(CR_DATUM);
    _canvas->pushImage(488, 12, 32, 32, 0, ImageResource_status_bar_battery_32x32); // Right margin 20px -> 540-20-32 = 488
    uint32_t vol = M5.getBatteryVoltage();

    if (vol < 3300) {
        vol = 3300;
    } else if (vol > 4350) {
        vol = 4350;
    }
    float battery = (float)(vol - 3300) / (float)(4350 - 3300);
    if (battery <= 0.01) {
        battery = 0.01;
    }
    if (battery > 1) {
        battery = 1;
    }
    uint8_t px = battery * 25;
    sprintf(buf, "%d%%", (int)(battery * 100));
    _canvas->fillRect(488 + 3, 12 + 10, px, 13, 15); // Adjusted to match battery icon

    // WiFi Icon
    if (WiFi.status() == WL_CONNECTED) {
        _canvas->pushImage(440, 12, 32, 32, 0, ImageResource_item_icon_wifi_3_32x32); // Shifted left to maintain spacing
    }

    // Time
    rtc_time_t time_struct;
    rtc_date_t date_struct;
    M5.RTC.getTime(&time_struct);
    M5.RTC.getDate(&date_struct);
    sprintf(buf, "%2d:%02d", time_struct.hour, time_struct.min);
    _canvas->setTextDatum(CC_DATUM);
    _canvas->setTextSize(26);
    _canvas->drawString(buf, 270, 28); // Centered
    _canvas->pushCanvas(0, 0, UPDATE_MODE_DU); 

    _time = millis();
    _next_update_time = (60 - time_struct.sec) * 1000;
    
    if (canvas != nullptr) {
        _canvas->pushToCanvas(0, 0, canvas);
    }
}

void StatusBar::update(void) {
    draw(nullptr);
}

void StatusBar::SetTransparent(bool transparent) {
    _is_transparent = transparent;
    if (transparent) {
        bool custom_wallpaper = false;
        
        // Priority 1: Runtime Custom Wallpaper
        if (global_custom_wallpaper_path.length() > 0) {
            if (DrawCustomWallpaper(_canvas_bg, global_custom_wallpaper_path.c_str())) {
                custom_wallpaper = true;
            }
        }
        
        // Priority 2: System Config Main Wallpaper
        #ifdef SYSTEM_CONFIG_MAIN_WALLPAPER_PATH
        if (!custom_wallpaper && DrawCustomWallpaper(_canvas_bg, SYSTEM_CONFIG_MAIN_WALLPAPER_PATH)) {
            custom_wallpaper = true;
        }
        #elif defined(SYSTEM_CONFIG_SLEEP_IMAGE_PATH)
        if (!custom_wallpaper && DrawCustomWallpaper(_canvas_bg, SYSTEM_CONFIG_SLEEP_IMAGE_PATH)) {
            custom_wallpaper = true;
        }
        #endif
        
        if (!custom_wallpaper) {
            _canvas_bg->pushImage(0, 0, 540, 960, GetWallpaper());
        }
    } else {
        _canvas_bg->fillCanvas(0); // Black background
        _canvas_bg->drawFastHLine(0, 55, 540, 15); // Separator line (White) at bottom
    }
    _next_update_time = 0; // Force immediate update
}
void StatusBar::SetAppMode(bool isAppMode) {
    _is_app_mode = isAppMode;
    _next_update_time = 0; // Force immediate update
}
