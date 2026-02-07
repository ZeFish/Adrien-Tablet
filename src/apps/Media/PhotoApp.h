#ifndef _PHOTO_APP_H_
#define _PHOTO_APP_H_

#include "core/gui/AppBase.h"
#include "core/config/SystemConfig.h"
#include "core/resources/ImageResource.h"
#include <vector>
#include <map>
#include <string>

class PhotoApp : public AppBase {
public:
    PhotoApp();
    ~PhotoApp();

    const uint8_t* GetIcon() override;

    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) override;
    int run() override;

    bool isFullScreen() override { return true; }

private:
    void LoadPhotos();
    void DrawPhoto(bool update = true);
    
    std::vector<String> _photos;
    int _current_photo_index = 0;
    
    std::map<std::string, std::pair<uint16_t, uint16_t>> _image_size_cache;

    // UI elements
    M5EPD_Canvas* _canvas = NULL; // We use the passed canvas (g_epd_canvas for fullscreen)
    
    // Button states
    uint32_t _btn_press_start_time = 0;
    bool _long_press_triggered = false;
    bool _touch_was_down = false;
};

#endif
