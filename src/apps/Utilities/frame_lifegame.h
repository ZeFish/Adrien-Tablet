#ifndef _FRAME_LIFEGAME_H_
#define _FRAME_LIFEGAME_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"

class Frame_Lifegame : public Frame_Base {
   public:
    Frame_Lifegame();
    ~Frame_Lifegame();
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args);
    int run();
    void exit();

   private:


};

#endif  //_FRAME_LIFEGAME_H_