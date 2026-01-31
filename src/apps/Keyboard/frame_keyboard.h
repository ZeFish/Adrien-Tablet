#ifndef _FRAME_KEYBOARD_H_
#define _FRAME_KEYBOARD_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"
#include "core/gui/epdgui/epdgui_textbox.h"
#include "core/gui/epdgui/epdgui_keyboard.h"

class Frame_Keyboard : public Frame_Base {
   public:
    Frame_Keyboard(bool isHorizontal = false);
    ~Frame_Keyboard();
    int run();
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args);

   private:
    EPDGUI_Textbox *inputbox;
    EPDGUI_Keyboard *keyboard;
    EPDGUI_Button *key_textclear;
    EPDGUI_Button *key_textsize_plus;
    EPDGUI_Button *key_textsize_reset;
    EPDGUI_Button *key_textsize_minus;
};

#endif  //_FRAME_KEYBOARD_H_