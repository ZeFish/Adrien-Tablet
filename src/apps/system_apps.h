#ifndef _SYSTEM_APPS_H_
#define _SYSTEM_APPS_H_

#include "app_base.h"
#include "../frame/frame_setting.h"
#include "../frame/frame_keyboard.h"
#include "../frame/frame_factorytest.h"
#include "../frame/frame_wifiscan.h"
#include "../frame/frame_lifegame.h"
#include "../frame/frame_fileindex.h"
#include "../frame/frame_compare.h"
#include "../frame/frame_home.h"

// Wrapper for existing frames
class SystemAppWrapper : public AppBase {
private:
    String _name;
    const uint8_t* _icon;
    Frame_Base* (*_factory)();

public:
    SystemAppWrapper(String name, const uint8_t* icon, Frame_Base* (*factory)()) 
        : AppBase(name), _name(name), _icon(icon), _factory(factory) {}

    const uint8_t* GetIcon() override { return _icon; }
    String GetAppName() override { return _name; }

    Frame_Base* GetTargetFrame() override {
        Frame_Base* frame = EPDGUI_GetFrame(_name);
        if (frame == NULL) {
            frame = _factory();
            EPDGUI_AddFrame(_name, frame);
        }
        return frame;
    }

    int init(epdgui_args_vector_t &args) override {
        return 0;
    }
};

// Factories for existing frames
inline Frame_Base* Factory_Setting() { return new Frame_Setting(); }
inline Frame_Base* Factory_Keyboard() { return new Frame_Keyboard(0); }
inline Frame_Base* Factory_FactoryTest() { return new Frame_FactoryTest(); }
inline Frame_Base* Factory_WifiScan() { return new Frame_WifiScan(); }
inline Frame_Base* Factory_LifeGame() { return new Frame_Lifegame(); }
inline Frame_Base* Factory_SDFile() { return new Frame_FileIndex("/"); }
inline Frame_Base* Factory_Compare() { return new Frame_Compare(); }
inline Frame_Base* Factory_Home() { return new Frame_Home(); }

#endif
