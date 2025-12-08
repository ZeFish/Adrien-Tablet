#ifndef _FRAME_MAIN_H_
#define _FRAME_MAIN_H_

#include "../apps/apps.h"
#include <vector>

class Frame_Main : public Frame_Base {
   public:
    Frame_Main(void);
    ~Frame_Main(void);
    int run();
    int init(epdgui_args_vector_t &args);
    void AppName(m5epd_update_mode_t mode);
    void StatusBar(m5epd_update_mode_t mode);

   private:
    std::vector<AppBase*> _apps;
    M5EPD_Canvas *_bar   = NULL;
    M5EPD_Canvas *_names = NULL;
    std::vector<EPDGUI_Button*> _home_keys;
    uint32_t _time             = 0;
    uint32_t _next_update_time = 0;
};

#endif  //_FRAME_MAIN_H_