#ifndef _APP_BASE_H_
#define _APP_BASE_H_

#include "../frame/frame_base.h"
#include "../epdgui/epdgui.h"

// Base class for all "Single File" apps
class AppBase : public Frame_Base {
public:
    AppBase(String frameName) : Frame_Base(true) {
        _frame_name = frameName;
    }

    virtual ~AppBase() {}

    // Icon to display on the main launcher
    virtual const uint8_t* GetIcon() = 0;
    
    // Name to display under the icon
    virtual String GetAppName() {
        return _frame_name;
    }

    // Required by Frame_Base, but we can provide a default empty impl
    virtual int init(epdgui_args_vector_t &args) {
        _is_run = 1;
        M5.EPD.Clear();
        if (_canvas_title) _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);
        // Your custom init code here
        return 3; 
    }
};

#endif
