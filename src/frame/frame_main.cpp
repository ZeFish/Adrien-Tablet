#include "frame_main.h"
#include <WiFi.h>
#include "../apps/apps.h"

#define KEY_W 92
#define KEY_H 92

void key_app_launch_cb(epdgui_args_vector_t &args) {
    AppBase* app = (AppBase*)(args[0]);
    // For SystemAppWrapper, the init() handles the frame transition logic
    // For a pure AppBase, we might need a standard way to push it. 
    // Allowing the app to handle its own launch in init() seems flexible.
    // However, init() is usually for UI setup inside the frame. 
    // Frame_Base* frame = EPDGUI_GetFrame(app->GetFrameName());
    // ...
    
    // Simplest: Just call init on the app instance? 
    // No, EPDGUI manages frames. We need to push the frame.
    // AppBase IS a Frame_Base.
    
    // Check if frame is already added? user apps might be new instances every time or singletons.
    // The registry returns new instances. 
    // Let's assume for now we just use the app instance directly.
    
    // Special handling for SystemAppWrapper which does tricky stuff.
    // But standard AppBase should work like this:
    if (app->GetFrameName().startsWith("Frame_")) {
         // It's likely a wrapper or something already managed?
         // Wrapper's init() does the push.
         app->init(args);
    } else {
        // It's a new "One File" app
        EPDGUI_PushFrame(app);
        // We might need to ensure it's added to the system if EPDGUI needs lookups?
        // EPDGUI_AddFrame(app->GetFrameName(), app); // Maybe needed?
    }
    
    // Note: The SystemAppWrapper implementation of init() handles PushFrame.
    // So for now, we just call app->init(args);
    // Wait, standard Frame_Base::init is for UI setup, NOT for pushing the frame.
    // The wrapper was a hack.
    
    // Let's rely on the init wrapper hack for now for system apps.
    // For new apps, we might need to adjust. 
    // Actually, looking at SystemAppWrapper, it uses init() to do the push.
    // So let's stick to that contract for this launcher.
    app->init(args); 
}

Frame_Main::Frame_Main(void) : Frame_Base(false) {
    _frame_name = "Frame_Main";
    _frame_id   = 1;

    _bar = new M5EPD_Canvas(&M5.EPD);
    _bar->createCanvas(540, 44);
    _bar->setTextSize(26);

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
        btn->Bind(EPDGUI_Button::EVENT_RELEASED, key_app_launch_cb);
        
        _home_keys.push_back(btn);

        col++;
        if (col >= 4) {
            col = 0;
            row++;
        }
    }

    _time             = 0;
    _next_update_time = 0;
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
    // Dynamic Name Drawing
    // We need to clear lines where names are valid.
    // The original code used 2 strips: Y=186 and Y=337.
    // 90 + 92 = 182. So 186 is below the first row. 
    // Row 0 Y=90. Name Y approx 90+92+4 = 186.
    // Row 1 Y=90 + 150 = 240. Name Y approx 240+92+4 = 336.
    
    // We can just iterate and draw names for each. 
    // Re-creating _names canvas for each row or using a full screen overlay?
    // The original used a strip canvas "createRender(20, 26)".
    
    // Let's try to replicate the original look but dynamically.
    // _names was passed 'mode'.
    
    // Drawing names is tricky with partial updates if we don't know the exact layout.
    // Let's assume standard grid rows.
    
    for (int r = 0; r <= (_apps.size() - 1) / 4; r++) {
        int y_pos = 186 + r * 150; // 186, 336...
        
        _names->createRender(20, 26); // Why this size? Original logic.
        _names->setTextSize(26);
        _names->fillCanvas(0);
        
        for (int c = 0; c < 4; c++) {
            int idx = r * 4 + c;
            if (idx >= _apps.size()) break;
            
            String name = _apps[idx]->GetAppName();
            // Localize if needed, relying on AppBase to provide localized name?
            // For now, raw name.
            
            // Fix specific system names if we want translation, 
            // but for "One File App" users will just provide string.
            // The SystemAppWrapper passes the frame name "Frame_Setting".
            // We might want to clean that up in the wrapper or here.
            name.replace("Frame_", ""); 
            
            _names->drawString(name, 20 + 46 + c * 136, 16);
        }
        _names->pushCanvas(0, y_pos, mode);
    }
}

void Frame_Main::StatusBar(m5epd_update_mode_t mode) {
    if ((millis() - _time) < _next_update_time) {
        return;
    }
    char buf[20];
    _bar->fillCanvas(0);
    _bar->drawFastHLine(0, 43, 540, 15);
    _bar->setTextDatum(CL_DATUM);
    _bar->drawString("M5Paper", 10, 27);

    // Battery
    _bar->setTextDatum(CR_DATUM);
    _bar->pushImage(498, 8, 32, 32, ImageResource_status_bar_battery_32x32);
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
    _bar->fillRect(498 + 3, 8 + 10, px, 13, 15);

    // WiFi Icon
    if (WiFi.status() == WL_CONNECTED) {
        _bar->pushImage(450, 8, 32, 32, ImageResource_item_icon_wifi_3_32x32);
    }

    // Time
    rtc_time_t time_struct;
    rtc_date_t date_struct;
    M5.RTC.getTime(&time_struct);
    M5.RTC.getDate(&date_struct);
    sprintf(buf, "%2d:%02d", time_struct.hour, time_struct.min);
    _bar->setTextDatum(CC_DATUM);
    _bar->drawString(buf, 270, 27);
    _bar->pushCanvas(0, 0, mode);

    _time             = millis();
    _next_update_time = (60 - time_struct.sec) * 1000;
}

int Frame_Main::init(epdgui_args_vector_t &args) {
    _is_run = 1;
    M5.EPD.WriteFullGram4bpp(GetWallpaper());
    for (int i = 0; i < _home_keys.size(); i++) {
        EPDGUI_AddObject(_home_keys[i]);
    }
    _time             = 0;
    _next_update_time = 0;
    StatusBar(UPDATE_MODE_NONE);
    AppName(UPDATE_MODE_NONE);
    return 9;
}

int Frame_Main::run() {
    // Calling Frame_Base::run() might duplicate status bar logic 
    // if Frame_Base has DrawStatusBar now. 
    // But Frame_Base::run() is useful for power save.
    Frame_Base::run(); 
    // Frame_Main has its own custom StatusBar implementation (different layout).
    // We should ensure they don't fight. Frame_Base::DrawStatusBar checks time.
    StatusBar(UPDATE_MODE_GL16);
    return 1;
}