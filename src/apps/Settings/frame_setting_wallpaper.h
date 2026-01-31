#ifndef _FRAME_WALLPAPER_H_
#define _FRAME_WALLPAPER_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"
#include "core/gui/epdgui/epdgui_switch.h"
#include "core/gui/epdgui/epdgui_mutexswitch.h"

class Frame_Setting_Wallpaper : public Frame_Base {
   public:
    Frame_Setting_Wallpaper();
    ~Frame_Setting_Wallpaper();
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t& args);

   private:
    EPDGUI_Switch* _sw_wallpapers[WALLPAPER_NUM];
    EPDGUI_MutexSwitch* _sw_mutex_group;
};

#endif  //_FRAME_WALLPAPER_H_