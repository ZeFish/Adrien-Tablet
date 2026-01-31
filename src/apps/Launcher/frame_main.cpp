#include "apps/Launcher/frame_main.h"
#include <WiFi.h>
#include <SD.h>
#include "core/system/AppLoader.h"
#include "core/config/SystemConfig.h"
#include "core/system/global_setting.h"

#define KEY_W 92
#define KEY_H 92

void key_app_launch_cb(epdgui_args_vector_t &args) {
    AppBase* app = (AppBase*)(args[0]);
    Frame_Base* target = app->GetTargetFrame();
    EPDGUI_PushFrame(target);
    *((int *)(args[1])) = 0; // Stop Frame_Main
}

Frame_Main::Frame_Main(void) : Frame_Base() {
    _frame_name = "Frame_Main";
    _frame_id   = 1;

    _names = new M5EPD_Canvas(&M5.EPD);
    _names->createCanvas(540, 32);
    _names->setTextDatum(CC_DATUM);

    // Load Apps
    _apps = AppRegistry::GetApps();

    // Create Buttons Grid
    int col = 0;
    int row = 0;
    for (int i = 0; i < _apps.size(); i++) {
        EPDGUI_Button* btn = new EPDGUI_Button(20 + col * 136, 90 + row * 150, KEY_W, KEY_H);
        
        btn->CanvasNormal()->pushImage(0, 0, 92, 92, _apps[i]->GetIcon());
        *(btn->CanvasPressed()) = *(btn->CanvasNormal());
        btn->CanvasPressed()->ReverseColor();
        
        // Bind Launch Callback
        btn->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, _apps[i]);
        btn->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, (void *)(&_is_run));
        btn->Bind(EPDGUI_Button::EVENT_RELEASED, key_app_launch_cb);
        
        _home_keys.push_back(btn);

        col++;
        if (col >= 4) {
            col = 0;
            row++;
        }
    }
}

Frame_Main::~Frame_Main(void) {
    for (int i = 0; i < _home_keys.size(); i++) {
        delete _home_keys[i];
    }
    // We own the app instances created by registry?
    // Yes, typical ownership transfer.
    for (int i = 0; i < _apps.size(); i++) {
        delete _apps[i];
    }
}

void Frame_Main::AppName(m5epd_update_mode_t mode) {
    // Disabled to remove white bars (backgrounds) under icons.
    // The user requested no app names and no background strips.
    return;
}



int Frame_Main::init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) {
    _is_run = 1;
    
#ifdef SYSTEM_CONFIG_MAIN_WALLPAPER_PATH
    if (!DrawCustomWallpaper(canvas, SYSTEM_CONFIG_MAIN_WALLPAPER_PATH, 0, -STATUS_BAR_HEIGHT)) {
        canvas->pushImage(0, -STATUS_BAR_HEIGHT, 540, 960, GetWallpaper());
    }
#else
    canvas->pushImage(0, -STATUS_BAR_HEIGHT, 540, 960, GetWallpaper());
#endif

    if (g_status_bar) {
        g_status_bar->SetTransparent(false);
        g_status_bar->SetAppMode(false);
    }

    for (int i = 0; i < _home_keys.size(); i++) {
        EPDGUI_AddObject(_home_keys[i]);
        _home_keys[i]->Draw(canvas, 0, -STATUS_BAR_HEIGHT);
    }
    return 9;
}
