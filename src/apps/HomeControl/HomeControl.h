#ifndef _HOME_CONTROL_APP_H_
#define _HOME_CONTROL_APP_H_

#include "core/gui/AppBase.h"
#include "core/system/ha_api.h"
#include "core/system/wifi_secrets.h"
#include "ha_widgets.h"
#include "HAButtonTile.h"
#include "core/config/SystemConfig.h"
#include "core/gui/epdgui/epdgui_switch.h"

class HomeControlApp : public AppBase {
public:
    HomeControlApp() : AppBase(SYSTEM_CONFIG_HOME_APP_NAME) {}

    ~HomeControlApp() {
        if (_sw_air_1) delete _sw_air_1;
        if (_sw_air_2) delete _sw_air_2;
        if (_key_air_1_plus) delete _key_air_1_plus;
        if (_key_air_1_minus) delete _key_air_1_minus;
        if (_key_air_2_plus) delete _key_air_2_plus;
        if (_key_air_2_minus) delete _key_air_2_minus;
        if (_tile1) delete _tile1;
        if (_tile2) delete _tile2;
        if (_tile3) delete _tile3;
        if (_tile4) delete _tile4;
        if (_tile5) delete _tile5;
        if (_tile6) delete _tile6;
    }

    const uint8_t* GetIcon() override {
        return ImageResource_main_icon_home_92x92;
    }

    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) override {
        _is_run = 1;
        canvas->fillCanvas(15);
        if (g_status_bar) {
            g_status_bar->SetTransparent(false);
            g_status_bar->SetAppMode(true);
        }

        // Fix: Create render for font size 36, which is used by buttons
        M5EPD_Canvas canvas_temp(&M5.EPD);
        canvas_temp.createRender(36);
        
        // Initialize Components
        InitComponents();

        // Attach HA tiles (they create and register their own underlying GUI objects)
        if (_tile1) { _tile1->attach(); if (_tile1->getSwitch()) _tile1->getSwitch()->Draw(canvas, 0, -STATUS_BAR_HEIGHT); }
        if (_tile2) { _tile2->attach(); if (_tile2->getSwitch()) _tile2->getSwitch()->Draw(canvas, 0, -STATUS_BAR_HEIGHT); }
        if (_tile3) { _tile3->attach(); if (_tile3->getSwitch()) _tile3->getSwitch()->Draw(canvas, 0, -STATUS_BAR_HEIGHT); }
        if (_tile4) { _tile4->attach(); if (_tile4->getSwitch()) _tile4->getSwitch()->Draw(canvas, 0, -STATUS_BAR_HEIGHT); }
        if (_tile5) { _tile5->attach(); if (_tile5->getSwitch()) _tile5->getSwitch()->Draw(canvas, 0, -STATUS_BAR_HEIGHT); }
        if (_tile6) { _tile6->attach(); if (_tile6->getSwitch()) _tile6->getSwitch()->Draw(canvas, 0, -STATUS_BAR_HEIGHT); }

        EPDGUI_AddObject(_sw_air_1);
        _sw_air_1->Draw(canvas, 0, -STATUS_BAR_HEIGHT);
        
        EPDGUI_AddObject(_sw_air_2);
        _sw_air_2->Draw(canvas, 0, -STATUS_BAR_HEIGHT);
        
        EPDGUI_AddObject(_key_air_1_plus);
        _key_air_1_plus->Draw(canvas, 0, -STATUS_BAR_HEIGHT);
        
        EPDGUI_AddObject(_key_air_1_minus);
        _key_air_1_minus->Draw(canvas, 0, -STATUS_BAR_HEIGHT);
        
        EPDGUI_AddObject(_key_air_2_plus);
        _key_air_2_plus->Draw(canvas, 0, -STATUS_BAR_HEIGHT);
        
        EPDGUI_AddObject(_key_air_2_minus);
        _key_air_2_minus->Draw(canvas, 0, -STATUS_BAR_HEIGHT);
        
        // EPDGUI_AddObject(_key_exit);
        // _key_exit->Draw(canvas, 0, -STATUS_BAR_HEIGHT);
        
        return 3;
    }

private:
    EPDGUI_Switch *_sw_light1 = NULL;
    EPDGUI_Switch *_sw_light2 = NULL;
    EPDGUI_Switch *_sw_socket1 = NULL;
    EPDGUI_Switch *_sw_socket2 = NULL;
    EPDGUI_Switch *_sw_air_1 = NULL;
    EPDGUI_Switch *_sw_air_2 = NULL;
    EPDGUI_Button *_key_air_1_plus = NULL;
    EPDGUI_Button *_key_air_1_minus = NULL;
    EPDGUI_Button *_key_air_2_plus = NULL;
    EPDGUI_Button *_key_air_2_minus = NULL;

    // Object-oriented tiles for Home Assistant controls
    HAButtonTile *_tile1 = NULL;
    HAButtonTile *_tile2 = NULL;
    HAButtonTile *_tile3 = NULL;
    HAButtonTile *_tile4 = NULL;
    HAButtonTile *_tile5 = NULL;
    HAButtonTile *_tile6 = NULL;

    void InitSwitch(EPDGUI_Switch *sw, String title, String subtitle,
                            const uint8_t *img1, const uint8_t *img2) {
        memcpy(sw->Canvas(0)->frameBuffer(),
            ImageResource_home_button_background_228x228, 228 * 228 / 2);
        sw->Canvas(0)->setTextSize(36);
        sw->Canvas(0)->setTextDatum(TC_DATUM);
        sw->Canvas(0)->drawString(title, 114, 136);
        sw->Canvas(0)->setTextSize(26);
        sw->Canvas(0)->drawString(subtitle, 114, 183);
        memcpy(sw->Canvas(1)->frameBuffer(), sw->Canvas(0)->frameBuffer(),
            228 * 228 / 2);
        sw->Canvas(0)->pushImage(68, 20, 92, 92, img1);
        sw->Canvas(1)->pushImage(68, 20, 92, 92, img2);
    }

    void InitComponents() {
        if (_tile1) return; // Already initialized

        // Create six HA tiles (2 columns x 3 rows)
        HAButtonTile::Config cfg;
        cfg.w = 228;
        cfg.h = 228;

        // Tile 1 (top-left)
        cfg.x = 20;
        cfg.y = 72;
        cfg.title = "Ceiling Light";
        cfg.subtitle = "Living Room";
        cfg.icon_off = ImageResource_home_icon_light_off_92x92;
        cfg.icon_on  = ImageResource_home_icon_light_on_92x92;
#ifdef HOME_ASSISTANT_LIGHT1_ENTITY
        cfg.entity = String(HOME_ASSISTANT_LIGHT1_ENTITY);
#endif
        _tile1 = new HAButtonTile();
        _tile1->init(cfg);

        // Tile 2 (top-right)
        cfg.x = 288;
        cfg.title = "Table Lamp";
        cfg.subtitle = "Bedroom";
#ifdef HOME_ASSISTANT_LIGHT2_ENTITY
        cfg.entity = String(HOME_ASSISTANT_LIGHT2_ENTITY);
#elif defined(HOME_ASSISTANT_LIGHT_ENTITY)
        cfg.entity = String(HOME_ASSISTANT_LIGHT_ENTITY);
#endif
        _tile2 = new HAButtonTile();
        _tile2->init(cfg);

        // Tile 3 (middle-left)
        cfg.x = 20;
        cfg.y = 324;
        cfg.title = "Computer";
        cfg.subtitle = "Bedroom";
        cfg.icon_off = ImageResource_home_icon_socket_off_92x92;
        cfg.icon_on  = ImageResource_home_icon_socket_on_92x92;
#ifdef HOME_ASSISTANT_SOCKET1_ENTITY
        cfg.entity = String(HOME_ASSISTANT_SOCKET1_ENTITY);
#endif
        _tile3 = new HAButtonTile();
        _tile3->init(cfg);

        // Tile 4 (middle-right)
        cfg.x = 288;
        cfg.title = "Dehumidifier";
        cfg.subtitle = "Bathroom";
#ifdef HOME_ASSISTANT_SOCKET2_ENTITY
        cfg.entity = String(HOME_ASSISTANT_SOCKET2_ENTITY);
#endif
        _tile4 = new HAButtonTile();
        _tile4->init(cfg);

        // Tile 5 (bottom-left) - script toggle example (smaller height)
        cfg.x = 20;
        cfg.y = 604;
        cfg.w = 228;
        cfg.h = 184;
        cfg.mode = HAButtonTile::MODE_SCRIPT_TOGGLE;
        cfg.title = "Goodnight";
        cfg.subtitle = "Script Toggle";
#ifdef HOME_ASSISTANT_SCRIPT1_ENTITY
        cfg.entity = String(HOME_ASSISTANT_SCRIPT1_ENTITY);
#else
        cfg.entity = String("script.goodnight");
#endif
        cfg.onVars  = String("{\"mode\":\"night\"}");
        cfg.offVars = String("{\"mode\":\"day\"}");
        cfg.paramIsNumeric = false;
        _tile5 = new HAButtonTile();
        _tile5->init(cfg);

        // Tile 6 (bottom-right) - script button example
        cfg.x = 288;
        cfg.title = "Wake Up";
        cfg.subtitle = "Script Button";
        cfg.mode = HAButtonTile::MODE_SCRIPT_BUTTON;
#ifdef HOME_ASSISTANT_SCRIPT2_ENTITY
        cfg.entity = String(HOME_ASSISTANT_SCRIPT2_ENTITY);
#else
        cfg.entity = String("script.wakeup");
#endif
        cfg.onVars = String("{\"room\":\"all\"}");
        _tile6 = new HAButtonTile();
        _tile6->init(cfg);

        // Keep air conditioner tiles (bottom row controls)
        _sw_air_1        = new EPDGUI_Switch(2, 20, 604, 228, 184);
        _sw_air_2        = new EPDGUI_Switch(2, 288, 604, 228, 184);
        _key_air_1_plus  = new EPDGUI_Button(20 + 116, 604 + 184, 112, 44);
        _key_air_1_minus = new EPDGUI_Button(20, 604 + 184, 116, 44);
        _key_air_2_plus  = new EPDGUI_Button(288 + 116, 604 + 184, 112, 44);
        _key_air_2_minus = new EPDGUI_Button(288, 604 + 184, 116, 44);

        // -- Setup Air Conditioner Buttons --
        _key_air_1_plus->SetCustomString("1");
        _key_air_1_minus->SetCustomString("0");
        _key_air_2_plus->SetCustomString("1");
        _key_air_2_minus->SetCustomString("0");
        
        _key_air_1_plus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, _key_air_1_plus);
        _key_air_1_plus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, _sw_air_1);
        _key_air_1_plus->Bind(EPDGUI_Button::EVENT_RELEASED, key_air_adjust_cb);

        _key_air_1_minus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, _key_air_1_minus);
        _key_air_1_minus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, _sw_air_1);
        _key_air_1_minus->Bind(EPDGUI_Button::EVENT_RELEASED, key_air_adjust_cb);

        _key_air_2_plus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, _key_air_2_plus);
        _key_air_2_plus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, _sw_air_2);
        _key_air_2_plus->Bind(EPDGUI_Button::EVENT_RELEASED, key_air_adjust_cb);

        _key_air_2_minus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, _key_air_2_minus);
        _key_air_2_minus->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, _sw_air_2);
        _key_air_2_minus->Bind(EPDGUI_Button::EVENT_RELEASED, key_air_adjust_cb);

        // -- HA Tiles (replaces manual switch setup) --
        // The tiles are created in InitComponents() and attached in init().
        // See src/HAButtonTile.h for the small object-oriented tile class.
        // If you want to adjust titles/images/entities, edit the Config passed in InitComponents().


        // Socket bindings are handled by the HA tiles (tile 3 and tile 4)

        // If you prefer to bind programmatically, here's how:
        // ha_bind_entity_switch_to(_sw_light1, "light.ceiling");
        // ha_bind_entity_switch_to(_sw_light2, "light.bedroom");
        // ha_bind_entity_switch_to(_sw_socket1, "switch.computer");
        // ha_bind_entity_switch_to(_sw_socket2, "switch.dehumidifier");

        // Examples for scripts:
        // 1) Toggle-able script using a switch (on/off variables):
        //    _sw_light2->SetCustomString("script.goodnight|{\"mode\":\"night\"}|{\"mode\":\"day\"}");
        //    ha_bind_entity_switch(_sw_light2);
        //
        // 2) One-shot script button (create a button and bind it):
        //    EPDGUI_Button *btn_script = new EPDGUI_Button(20, 20, 228, 228);
        //    // initialize visuals as needed...
        //    ha_bind_script_button(btn_script, "script.goodnight", "{\"room\":\"all\"}");
        //    EPDGUI_AddObject(btn_script);

        // -- Setup Air Conditioners --
        SetupAirCon(_sw_air_1, "Bedroom");
        SetupAirCon(_sw_air_2, "Living Room");

        // Bind Air Con State Callbacks
        BindAirState(_sw_air_1, _key_air_1_plus, _key_air_1_minus);
        BindAirState(_sw_air_2, _key_air_2_plus, _key_air_2_minus);

        // exitbtn("Home"); // Removed per user request (handled by status bar/G38)

        // _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
        // _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);
    }

    void SetupAirCon(EPDGUI_Switch* sw, String name) {
        memcpy(sw->Canvas(0)->frameBuffer(),
            ImageResource_home_air_background_228x184, 228 * 184 / 2);
        sw->Canvas(0)->setTextDatum(TC_DATUM);
        sw->Canvas(0)->setTextSize(26);
        sw->Canvas(0)->drawString(name, 114, 152);
        
        memcpy(sw->Canvas(1)->frameBuffer(),
            sw->Canvas(0)->frameBuffer(), 228 * 184 / 2);
        sw->Canvas(0)->setTextSize(36);
        sw->Canvas(0)->drawString("OFF", 114, 108);
        sw->Canvas(1)->setTextSize(36);
        sw->Canvas(1)->setTextDatum(TC_DATUM);
        sw->Canvas(1)->drawString("26C", 114, 108);
        sw->SetCustomString("26"); // Default temp

        sw->Canvas(0)->pushImage(68, 12, 92, 92, ImageResource_home_icon_conditioner_off_92x92);
        sw->Canvas(1)->pushImage(68, 12, 92, 92, ImageResource_home_icon_conditioner_on_92x92);
    }

    void BindAirState(EPDGUI_Switch* sw, EPDGUI_Button* plus, EPDGUI_Button* minus) {
        sw->AddArgs(0, 0, plus);
        sw->AddArgs(0, 1, minus);
        sw->AddArgs(0, 2, sw);
        sw->Bind(0, key_air_state0_cb); // Turned OFF
        
        sw->AddArgs(1, 0, plus);
        sw->AddArgs(1, 1, minus);
        sw->AddArgs(1, 2, sw);
        sw->Bind(1, key_air_state1_cb); // Turned ON
        
        // Initial state disable buttons
        plus->SetEnable(false);
        minus->SetEnable(false);
        
        // Button Graphics
        memcpy(plus->CanvasNormal()->frameBuffer(), ImageResource_home_air_background_r_112x44, 112 * 44 / 2);
        memcpy(plus->CanvasPressed()->frameBuffer(), plus->CanvasNormal()->frameBuffer(), 112 * 44 / 2);
        plus->CanvasPressed()->ReverseColor();
        
        memcpy(minus->CanvasNormal()->frameBuffer(), ImageResource_home_air_background_l_116x44, 116 * 44 / 2);
        memcpy(minus->CanvasPressed()->frameBuffer(), minus->CanvasNormal()->frameBuffer(), 116 * 44 / 2);
        minus->CanvasPressed()->ReverseColor();
    }


    // --- Callbacks (Static) ---

    static void key_air_adjust_cb(epdgui_args_vector_t &args) {
        int operation     = ((EPDGUI_Button *)(args[0]))->GetCustomString().toInt();
        EPDGUI_Switch *sw = ((EPDGUI_Switch *)(args[1]));
        if (sw->getState() == 0) return;
        
        int temp = sw->GetCustomString().toInt();
        char buf[10];
        if (operation == 1) temp++; else temp--;
        
        sprintf(buf, "%d", temp);
        sw->SetCustomString(buf);
        sprintf(buf, "%dC", temp);
        sw->Canvas(1)->setTextSize(36);
        sw->Canvas(1)->setTextDatum(TC_DATUM);
        sw->Canvas(1)->fillRect(114 - 100, 108, 200, 38, 0);
        sw->Canvas(1)->drawString(buf, 114, 108);
        // sw->Canvas(1)->pushCanvas(sw->getX(), sw->getY(), UPDATE_MODE_A2);
    }

    static void key_air_state0_cb(epdgui_args_vector_t &args) {
        ((EPDGUI_Button *)(args[0]))->SetEnable(false);
        ((EPDGUI_Button *)(args[1]))->SetEnable(false);
    }

    static void key_air_state1_cb(epdgui_args_vector_t &args) {
        ((EPDGUI_Button *)(args[0]))->SetEnable(true);
        ((EPDGUI_Button *)(args[1]))->SetEnable(true);
    }
    
    static void key_light_cb(epdgui_args_vector_t &args) {
        EPDGUI_Switch *sw = ((EPDGUI_Switch *)(args[0]));
        int state = sw->getState();

        // Read entity binding from this switch's custom string. If none is set,
        // fall back to the global HOME_ASSISTANT_LIGHT_ENTITY (if defined).
        String entity = sw->GetCustomString();
        #ifdef HOME_ASSISTANT_LIGHT_ENTITY
        if (entity.length() == 0) {
            entity = String(HOME_ASSISTANT_LIGHT_ENTITY);
        }
        #endif

        if (entity.length() > 0) {
            if (state == 1) {
                ha_call_service("light", "turn_on", entity);
            } else {
                ha_call_service("light", "turn_off", entity);
            }
        } else {
            Serial.println("No Home Assistant entity configured for this switch.");
        }
    }
};

#endif
