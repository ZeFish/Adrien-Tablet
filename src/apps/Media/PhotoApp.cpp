#include "PhotoApp.h"
#include "core/gui/epdgui/epdgui.h"
#include <SD.h>

PhotoApp::PhotoApp() : AppBase("Photos") {
    
}

PhotoApp::~PhotoApp() {

}

const uint8_t* PhotoApp::GetIcon() {
    return ImageResource_main_icon_sdcard_92x92; // Use generic image icon
    // If not available, might need to find one or fallback. 
    // Assuming ImageResource_main_icon_image_92x92 exists in common set, if not I'll fix later.
}

int PhotoApp::init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) {
    _canvas = canvas;
    _is_run = 1;
    
    LoadPhotos();
    
    if (_photos.empty()) {
        _canvas->fillCanvas(0);
        _canvas->setTextSize(26);
        _canvas->setTextColor(15);
        _canvas->setTextDatum(CC_DATUM);
        _canvas->drawString("No Photos in /photos", 540/2, 960/2);
    } else {
        DrawPhoto(false); // Initial draw
    }
    
    return 0; // Run loop
}

void PhotoApp::LoadPhotos() {
    _photos.clear();
    File root = SD.open("/photos");
    if (!root) {
        root = SD.open("/"); // Fallback to root if folder missing
        if (!root) return;
    }
    
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String name = String(file.name());
            String lower = name;
            lower.toLowerCase();
            if (lower.endsWith(".jpg") || lower.endsWith(".jpeg") || lower.endsWith(".png")) {
                // Construct full path
                String fullPath = String(root.name());
                if (fullPath == "/") fullPath = "";
                fullPath += "/";
                fullPath += name;
                _photos.push_back(fullPath);
            }
        }
        file = root.openNextFile();
    }
    root.close();
    
    // simple sort? 
    // std::sort(_photos.begin(), _photos.end());
}

// Helper to get jpeg size
static bool getJpegSize(fs::FS &fs, const char *path, uint16_t *width, uint16_t *height) {
    File file = fs.open(path);
    if (!file) return false;
    if (file.read() != 0xFF || file.read() != 0xD8) { file.close(); return false; }
    while (file.available()) {
        if (file.read() != 0xFF) continue;
        uint8_t marker = file.read();
        uint16_t len   = file.read() << 8 | file.read();
        if (marker == 0xC0 || marker == 0xC2) {
            file.read(); *height = file.read() << 8 | file.read(); *width  = file.read() << 8 | file.read();
            file.close(); return true;
        }
        file.seek(len - 2, SeekCur);
    }
    file.close(); return false;
}

void PhotoApp::DrawPhoto(bool update) {
    if (_photos.empty()) return;
    
    _canvas->fillCanvas(0);
    
    String path = _photos[_current_photo_index];
    log_d("Drawing: %s", path.c_str());
    
    uint16_t w, h;
    bool found_size = false;
    std::string p_str = std::string(path.c_str());

    if (_image_size_cache.count(p_str)) {
        w = _image_size_cache[p_str].first;
        h = _image_size_cache[p_str].second;
        found_size = true;
    } else if (getJpegSize(SD, path.c_str(), &w, &h)) {
        // Simple cache eviction strategy to prevent OOM
        if (_image_size_cache.size() > 500) {
            _image_size_cache.clear();
        }
        _image_size_cache[p_str] = std::make_pair(w, h);
        found_size = true;
    }

    if (found_size) {
        jpeg_div_t scale = JPEG_DIV_NONE;
        // Fit to screen (540x960)
        // Check if we need to scale down
        if (w > 540 || h > 960) {
            if (w > 540*4 || h > 960*4) scale = JPEG_DIV_8;
            else if (w > 540*2 || h > 960*2) scale = JPEG_DIV_4;
            else if (w > 540 || h > 960) scale = JPEG_DIV_2;
        }
        
        // Center
        int sw = w >> (int)scale;
        int sh = h >> (int)scale;
        int x = (540 - sw) / 2;
        int y = (960 - sh) / 2;
        
        // M5EPD drawJpgFile arguments: (fs, path, x, y, w, h, offx, offy, scale)
        // To draw whole image scaled: w=0, h=0, offx=0, offy=0.
        // We just position it at x,y.
        _canvas->drawJpgFile(SD, path.c_str(), x, y, 0, 0, 0, 0, scale);
        
        // Debug
        char buf[64];
        sprintf(buf, "%dx%d -> %dx%d (S%d)", w, h, sw, sh, (int)scale);
        // _canvas->drawString(buf, 10, 10); // Optional debug string
    } else {
        // Fallback or PNG
        _canvas->drawPngFile(SD, path.c_str(), 0, 0); 
        // If it fails (e.g. not PNG), it does nothing.
        // Try simple drawJpg as last resort
        _canvas->drawJpgFile(SD, path.c_str(), 0, 0);
    }
    
    if (update) {
        _canvas->pushCanvas(0, 0, UPDATE_MODE_GC16);
    }
}

int PhotoApp::run() {
    M5.update();
    
    // G37 (Left Button) - Prev
    if (M5.BtnL.wasReleased()) {
        if (_photos.size() > 0) {
            _current_photo_index--;
            if (_current_photo_index < 0) _current_photo_index = _photos.size() - 1;
            DrawPhoto();
        }
    }
    
    // G39 (Right Button) - Next
    if (M5.BtnR.wasReleased()) {
        if (_photos.size() > 0) {
            _current_photo_index++;
            if (_current_photo_index >= _photos.size()) _current_photo_index = 0;
            DrawPhoto();
        }
    }

    // G38 (Middle) - Long Press for Wallpaper
    if (M5.BtnP.isPressed()) {
        if (_btn_press_start_time == 0) {
            _btn_press_start_time = millis();
            _long_press_triggered = false;
        } else {
            if (!_long_press_triggered && (millis() - _btn_press_start_time > 1000)) {
                // Long Press Triggered
                _long_press_triggered = true;
                
                if (!_photos.empty()) {
                    String path = _photos[_current_photo_index];
                    // Set as Wallpaper
                    SetCustomWallpaperPath(path);
                    
                    // Feedback UI
                    _canvas->setTextSize(36);
                    _canvas->setTextColor(15);
                    _canvas->fillRect(100, 400, 340, 100, 0);
                    _canvas->drawRect(100, 400, 340, 100, 15);
                    _canvas->setTextDatum(CC_DATUM);
                    _canvas->drawString("Wallpaper Set!", 270, 450);
                    _canvas->pushCanvas(0, 0, UPDATE_MODE_DU);
                    delay(1000);
                    DrawPhoto();
                }
            }
        }
    } else {
        if (_btn_press_start_time > 0) {
           // Released
           if (!_long_press_triggered) {
               // Manual Exit Trigger on Short Press
               _is_run = 0; 
           }
           _btn_press_start_time = 0;
        }
    }
    
    // Touch Logic (50% Screen)
    if (M5.TP.available()) {
        M5.TP.update();
        bool is_finger_down = !M5.TP.isFingerUp();
        
        if (is_finger_down) {
            _touch_was_down = true;
        } else {
            if (_touch_was_down) {
                // Finger released
                _touch_was_down = false;
                
                uint16_t x = M5.TP.readFingerX(0);
                if (x < 270) {
                     // Left 50%
                     if (_photos.size() > 0) {
                        _current_photo_index--;
                        if (_current_photo_index < 0) _current_photo_index = _photos.size() - 1;
                        DrawPhoto();
                    }
                } else {
                    // Right 50%
                     if (_photos.size() > 0) {
                        _current_photo_index++;
                        if (_current_photo_index >= _photos.size()) _current_photo_index = 0;
                        DrawPhoto();
                    }
                }
            }
        }
    }
    
    return 1;
}
