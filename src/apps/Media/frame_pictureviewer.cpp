#include "apps/Media/frame_pictureviewer.h"
#include "SD.h"

void key_pictureviewer_exit_cb(epdgui_args_vector_t &args) {
    EPDGUI_PopFrame(true);
    *((int *)(args[0])) = 0;
}

Frame_PictureViewer::Frame_PictureViewer(String path) : Frame_Base() {
    _frame_name = "Frame_PictureViewer";
    _pic_path   = path;

    _canvas_picture = new M5EPD_Canvas(&M5.EPD);
    _canvas_picture->createCanvas(540, 888);
    _canvas_picture->setTextSize(26);
    _canvas_picture->setTextColor(0);
    _canvas_picture->setTextDatum(CC_DATUM);

    uint8_t language = GetLanguage();
    if (language == LANGUAGE_JA) {
        exitbtn("戻る");
    } else if (language == LANGUAGE_ZH) {
        exitbtn("返回");
    } else {
        exitbtn("Back");
    }

    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &key_pictureviewer_exit_cb);

    _is_first = true;
}

Frame_PictureViewer::~Frame_PictureViewer(void) {
    delete _canvas_picture;
}

void Frame_PictureViewer::err(String info) {
    _canvas_picture->fillCanvas(0);
    _canvas_picture->fillRect(254 - 150, 500 - 50, 300, 100, 15);
    _canvas_picture->drawString(info, 150, 55);
}

// Helper to get jpeg size
bool getJpegSize(fs::FS &fs, const char *path, uint16_t *width,
                 uint16_t *height) {
    File file = fs.open(path);
    if (!file) return false;

    // Very basic jpeg parser
    if (file.read() != 0xFF || file.read() != 0xD8) {
        file.close();
        return false;
    }  // Not JPEG

    while (file.available()) {
        if (file.read() != 0xFF) continue;
        uint8_t marker = file.read();
        uint16_t len   = file.read() << 8 | file.read();

        // SOF0 (Baseline) or SOF2 (Progressive)
        if (marker == 0xC0 || marker == 0xC2) {
            file.read();  // Precision
            *height = file.read() << 8 | file.read();
            *width  = file.read() << 8 | file.read();
            file.close();
            return true;
        }
        file.seek(len - 2, SeekCur);
    }
    file.close();
    return false;
}

int Frame_PictureViewer::init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) {
    _is_run = 1;
    // M5.EPD.Clear();

    EPDGUI_AddObject(_key_exit);

    LoadingAnime_32x32_Start(254, 500);
    String suffix = _pic_path.substring(_pic_path.lastIndexOf("."));
    if ((suffix.indexOf("bmp") >= 0) || (suffix.indexOf("BMP") >= 0)) {
        bool ret =
            canvas->drawBmpFile(SD, _pic_path.c_str(), 0, 0);
        if (ret == 0) {
            err("Error opening " +
                _pic_path.substring(_pic_path.lastIndexOf("/")));
        }
    } else if ((suffix.indexOf("png") >= 0) ||
                (suffix.indexOf("PNG") >= 0)) {
        bool ret = canvas->drawPngFile(SD, _pic_path.c_str());
        if (ret == 0) {
            err("Error opening " +
                _pic_path.substring(_pic_path.lastIndexOf("/")));
        }
    } else if ((suffix.indexOf("jpg") >= 0) ||
                (suffix.indexOf("JPG") >= 0)) {
        uint16_t w, h;
        jpeg_div_t scale = JPEG_DIV_NONE;
        int x = 0, y = 0;

        if (getJpegSize(SD, _pic_path.c_str(), &w, &h)) {
            // Calculate scale to fit 540x888
            if (w > 540 * 4 || h > 888 * 4)
                scale = JPEG_DIV_8;
            else if (w > 540 * 2 || h > 888 * 2)
                scale = JPEG_DIV_4;
            else if (w > 540 || h > 888)
                scale = JPEG_DIV_2;

            // Calculate centered position
            uint16_t sw = w >> (uint8_t)scale;
            uint16_t sh = h >> (uint8_t)scale;
            x           = (540 - sw) / 2;
            y           = (888 - sh) / 2;
            if (x < 0) x = 0;
            if (y < 0) y = 0;
        }

        bool ret = canvas->drawJpgFile(SD, _pic_path.c_str(), x, y,
                                                0, 0, 0, 0, scale);
        if (ret == 0) {
            err("Error opening " +
                _pic_path.substring(_pic_path.lastIndexOf("/")));
        }
    }
    LoadingAnime_32x32_Stop();
    // canvas->pushCanvas(0, 72, UPDATE_MODE_GC16);

    return 3;
}
