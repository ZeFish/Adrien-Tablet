#ifndef _APP_BASE_H_
#define _APP_BASE_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"

// Base class for all "Single File" apps
class AppBase : public Frame_Base {
public:
    AppBase(String frameName) : Frame_Base() {
        _frame_name = frameName;
    }

    virtual ~AppBase() {}

    // Icon to display on the main launcher
    virtual const uint8_t* GetIcon() = 0;
    
    // Name to display under the icon
    virtual String GetAppName() {
        return _frame_name;
    }

    // Returns the frame to be pushed to the stack. 
    // Usually 'this', but wrappers can return the underlying frame.
    virtual Frame_Base* GetTargetFrame() {
        return this;
    }

    // Required by Frame_Base, but we can provide a default empty impl
    virtual int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) {
        _is_run = 1;
        if(canvas != NULL) {
            canvas->fillCanvas(15);
        }
        if (g_status_bar) {
            g_status_bar->SetTransparent(false);
            g_status_bar->SetAppMode(true);
        }
        // Your custom init code here
        return 3; 
    }
};

#endif
