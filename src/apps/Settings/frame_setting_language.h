#ifndef _FRAME_LANGUAGE_H_
#define _FRAME_LANGUAGE_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"
#include "core/gui/epdgui/epdgui_switch.h"
#include "core/gui/epdgui/epdgui_mutexswitch.h"

class Frame_Setting_Language : public Frame_Base {
   public:
    Frame_Setting_Language();
    ~Frame_Setting_Language();
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t& args);

   private:
    EPDGUI_Switch* _sw_en;
    EPDGUI_Switch* _sw_zh;
    EPDGUI_Switch* _sw_ja;
    EPDGUI_MutexSwitch* _sw_mutex_group;
};

#endif  //_FRAME_LANGUAGE_H_