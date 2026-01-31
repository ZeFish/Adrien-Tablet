#include "WeatherApp.h"

const uint8_t* WeatherApp::GetIcon() {
    return ImageResource_main_icon_todo_92x92;
}

int WeatherApp::init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) {
    _is_run = 1;
    canvas->fillCanvas(0);
    
    canvas->setTextSize(36);
    canvas->setTextDatum(CC_DATUM);
    
    // Setup Status Bar
    if (g_status_bar) {
        g_status_bar->SetTransparent(false);
        g_status_bar->SetAppMode(true);
    }

    // Draw Header
    // Adjusted Y positions to avoid overlap with status bar logic and ensure visibility
    int header_y = 40; 
    canvas->drawString("Météo Adrien", 270, header_y);
    // Line removed per user inquiry "Whats the line in the weather app?" -> implied removal or confusion.
    // canvas->drawFastHLine(50, header_y + 30, 440, 15);

    // Fetch Weather
    String state = ha_get_state(HOME_ASSISTANT_WEATHER_ENTITY);
    log_d("Weather state: %s", state.c_str());
    if (state == "") {
        canvas->drawString("Chargement...", 270, 480 - STATUS_BAR_HEIGHT);
    } else {
        DrawAdvice(canvas, state);
    }
    
    // Exit Button (invisible over the whole screen for simplicity, or just a small one)
    exitbtn("Retour");
    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);

    return 3;
}

//     } else {
//         weatherText = state;
//         clothIcon = "/weather/default.png";
//     }

void WeatherApp::DrawAdvice(M5EPD_Canvas* canvas, String state) {
    // Just fetch and display temperature
    String temp = ha_get_attribute(HOME_ASSISTANT_WEATHER_ENTITY, "temperature");
    
    if (temp.length() > 0) {
        // Display big temperature number
        canvas->createRender(140);
        canvas->setTextSize(140);
        canvas->setTextDatum(CC_DATUM);
        canvas->drawString(temp + "°", 270, 480 - STATUS_BAR_HEIGHT); // Centered vertically roughly
        
        // Optional: display condition text smaller below? User said "just the current temp number".
        // Let's stick to just the number as requested for now.
    } else {
        canvas->setTextSize(26);
        canvas->drawString("Erreur Température", 270, 480 - STATUS_BAR_HEIGHT);
    }
}
