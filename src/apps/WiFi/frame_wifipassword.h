#ifndef _FRAME_WIFIPASSWORD_H_
#define _FRAME_WIFIPASSWORD_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"
#include "core/gui/epdgui/epdgui_textbox.h"
#include "core/gui/epdgui/epdgui_keyboard.h"

class Frame_WifiPassword : public Frame_Base {
   public:
    Frame_WifiPassword(bool isHorizontal = false);
    ~Frame_WifiPassword();
    int run();
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args);

   private:
    EPDGUI_Textbox *inputbox;
    EPDGUI_Keyboard *keyboard;
    EPDGUI_Button *key_textclear;
};

#endif  //_FRAME_WIFIPASSWORD_H_