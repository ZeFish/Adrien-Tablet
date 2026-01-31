#ifndef _FRAME_FILEINDEX_H_
#define _FRAME_FILEINDEX_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"
#include <SD.h>

class Frame_FileIndex : public Frame_Base {
   public:
    Frame_FileIndex(String path);
    ~Frame_FileIndex();
    void listDir(fs::FS &fs, const char *dirname);
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args);

   private:
    std::vector<EPDGUI_Button *> _key_files;
    String _path;
};

#endif  //_FRAME_FILEINDEX_H_