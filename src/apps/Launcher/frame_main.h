#ifndef _FRAME_MAIN_H_
#define _FRAME_MAIN_H_

#include "core/system/AppLoader.h"
#include <vector>

class Frame_Main : public Frame_Base {
   public:
    Frame_Main(void);
    ~Frame_Main(void);
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args);
    void AppName(m5epd_update_mode_t mode);

   private:
    std::vector<AppBase*> _apps;
    M5EPD_Canvas *_names = NULL;
    std::vector<EPDGUI_Button*> _home_keys;
};

#endif  //_FRAME_MAIN_H_