#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

// Mocking Arduino String
typedef std::string String;

// Mocking FS and File
int global_open_count = 0;

class File {
public:
    bool operator!() const { return false; } // Always valid
    operator bool() const { return true; }
    void close() {}
    int read() { return 0; } // Dummy
    // Add other necessary methods if needed by getJpegSize logic,
    // but for the purpose of counting opens, we might just bypass the detailed read logic
    // or implement a minimal stub.
    bool available() { return false; }
    void seek(uint32_t pos, int mode) {}
};

class FS {
public:
    File open(const char* path, const char* mode = "r") {
        global_open_count++;
        // std::cout << "Opening file: " << path << std::endl;
        return File();
    }
};

FS SD;

// Mocking Canvas
class M5EPD_Canvas {
public:
    void fillCanvas(int color) {}
    void drawJpgFile(FS &fs, const char *path, int x, int y, int w, int h, int offx, int offy, int scale) {
        // Simulate what drawJpgFile does: it opens the file to read data
        fs.open(path);
    }
    void drawPngFile(FS &fs, const char *path, int x, int y) {}
    void pushCanvas(int x, int y, int mode) {}
};

enum jpeg_div_t {
    JPEG_DIV_NONE,
    JPEG_DIV_2,
    JPEG_DIV_4,
    JPEG_DIV_8
};

// Simplified PhotoApp Logic
class PhotoApp {
public:
    M5EPD_Canvas* _canvas;
    std::vector<String> _photos;
    int _current_photo_index = 0;
    std::map<String, std::pair<uint16_t, uint16_t>> _image_size_cache;

    PhotoApp() {
        _canvas = new M5EPD_Canvas();
        _photos.push_back("/photos/image1.jpg");
    }

    // Copied and adapted getJpegSize
    // We modify it to just return true and simulate reading without actual data
    static bool getJpegSize(FS &fs, const char *path, uint16_t *width, uint16_t *height) {
        File file = fs.open(path); // This counts as 1 open
        if (!file) return false;

        // Simulate reading dimensions
        *width = 1080;
        *height = 1920;

        file.close();
        return true;
    }

    void DrawPhoto(bool update) {
        if (_photos.empty()) return;

        _canvas->fillCanvas(0);

        String path = _photos[_current_photo_index];
        // log_d("Drawing: %s", path.c_str());

        uint16_t w, h;
        bool found_size = false;

        if (_image_size_cache.count(path)) {
            w = _image_size_cache[path].first;
            h = _image_size_cache[path].second;
            found_size = true;
        } else if (getJpegSize(SD, path.c_str(), &w, &h)) {
            _image_size_cache[path] = std::make_pair(w, h);
            found_size = true;
        }

        if (found_size) {
            jpeg_div_t scale = JPEG_DIV_NONE;
            if (w > 540 || h > 960) {
               // ... scaling logic ...
               scale = JPEG_DIV_2;
            }

            // ... positioning logic ...
            int x = 0;
            int y = 0;

            // The drawing (opens file again)
            _canvas->drawJpgFile(SD, path.c_str(), x, y, 0, 0, 0, 0, scale);
        } else {
             _canvas->drawPngFile(SD, path.c_str(), 0, 0);
        }

        if (update) {
            _canvas->pushCanvas(0, 0, 0);
        }
    }
};

int main() {
    PhotoApp app;

    std::cout << "Initial Open Count: " << global_open_count << std::endl;

    // Draw first time
    app.DrawPhoto(true);
    std::cout << "Open Count after 1st draw: " << global_open_count << std::endl;

    // Draw second time (simulating coming back to the image or redraw)
    app.DrawPhoto(true);
    std::cout << "Open Count after 2nd draw: " << global_open_count << std::endl;

    return 0;
}
