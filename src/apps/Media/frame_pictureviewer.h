#ifndef _FRAME_PICTUREVIEWER_H_
#define _FRAME_PICTUREVIEWER_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"

class Frame_PictureViewer : public Frame_Base {
   public:
    Frame_PictureViewer(String path);
    ~Frame_PictureViewer();
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args);
    void err(String info);

   private:
    M5EPD_Canvas *_canvas_picture;
    String _pic_path;
    bool _is_first;
};

#endif  //_FRAME_PICTUREVIEWER_H_