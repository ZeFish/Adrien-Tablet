#include "core/system/global_setting.h"
#include "core/resources/ImageResource.h"
#include "esp32-hal-log.h"
#include <WiFi.h>
#include "core/system/wifi_secrets.h"
#include <SD.h>
#include "core/config/SystemConfig.h"

#ifdef SYSTEM_CONFIG_DEFAULT_WALLPAPER_ID
#define DEFAULT_WALLPAPER SYSTEM_CONFIG_DEFAULT_WALLPAPER_ID
#else
#define DEFAULT_WALLPAPER 2
#endif

SemaphoreHandle_t _xSemaphore_LoadingAnime = NULL;
static uint8_t _loading_anime_eixt_flag    = false;
esp_err_t __espret__;
#define NVS_CHECK(x)            \
    __espret__ = x;             \
    if (__espret__ != ESP_OK) { \
        nvs_close(nvs_arg);     \
        log_e("Check Err");     \
        return __espret__;      \
    }

const uint8_t *wallpapers[] = {ImageResource_wallpaper_m5stack_540x960, ImageResource_wallpaper_engine_540x960,
                               ImageResource_wallpaper_penrose_triangle_540x960};

const uint8_t *kIMGLoading[16] = {
    ImageResource_item_loading_01_32x32, ImageResource_item_loading_02_32x32, ImageResource_item_loading_03_32x32,
    ImageResource_item_loading_04_32x32, ImageResource_item_loading_05_32x32, ImageResource_item_loading_06_32x32,
    ImageResource_item_loading_07_32x32, ImageResource_item_loading_08_32x32, ImageResource_item_loading_09_32x32,
    ImageResource_item_loading_10_32x32, ImageResource_item_loading_11_32x32, ImageResource_item_loading_12_32x32,
    ImageResource_item_loading_13_32x32, ImageResource_item_loading_14_32x32, ImageResource_item_loading_15_32x32,
    ImageResource_item_loading_16_32x32};
const char *wallpapers_name_en[] = {"M5Paper", "Engine", "Penrose Triangle"};
const char *wallpapers_name_zh[] = {"M5Paper", "引擎", "彭罗斯三角"};
const char *wallpapers_name_ja[] = {"M5Paper", "エンジン", "ペンローズの三角形"};
uint16_t global_wallpaper        = DEFAULT_WALLPAPER;
uint8_t global_language          = LANGUAGE_EN;
String global_wifi_ssid;
String global_wifi_password;
String global_custom_wallpaper_path = "";
uint8_t global_wifi_configed    = false;
uint16_t global_reader_textsize = 32;
uint8_t global_time_synced      = false;
uint8_t global_ttf_file_loaded  = false;
uint8_t global_init_status      = 0xFF;
#ifdef SYSTEM_CONFIG_DEFAULT_TIMEZONE
int8_t global_timezone          = SYSTEM_CONFIG_DEFAULT_TIMEZONE;
#else
int8_t global_timezone          = 8;
#endif

int8_t GetTimeZone(void) {
    return global_timezone;
}

void SetTimeZone(int8_t time_zone) {
    global_timezone = time_zone;
}

void SetInitStatus(uint8_t idx, uint8_t val) {
    global_init_status &= ~(1 << idx);
    global_init_status |= (val << idx);
}

uint8_t GetInitStatus(uint8_t idx) {
    return (global_init_status & (1 << idx)) ? true : false;
}

void SetTTFLoaded(uint8_t val) {
    global_ttf_file_loaded = val;
}

uint8_t isTTFLoaded() {
    return global_ttf_file_loaded;
}

uint8_t isTimeSynced(void) {
    return global_time_synced;
}

void SetTimeSynced(uint8_t val) {
    global_time_synced = val;
    SaveSetting();
}

void SetLanguage(uint8_t language) {
    if (language >= LANGUAGE_EN && language <= LANGUAGE_ZH) {
        global_language = language;
    }
    SaveSetting();
}

uint8_t GetLanguage(void) {
    return global_language;
}

void SetWallpaper(uint16_t wallpaper_id) {
    global_wallpaper = wallpaper_id;
    SaveSetting();
}

uint16_t GetWallpaperID(void) {
    return global_wallpaper;
}

const uint8_t *GetWallpaper(void) {
    return wallpapers[global_wallpaper];
}

const char *GetWallpaperName(uint16_t wallpaper_id) {
    switch (global_language) {
        case LANGUAGE_ZH:
            return wallpapers_name_zh[wallpaper_id];
        case LANGUAGE_JA:
            return wallpapers_name_ja[wallpaper_id];
        default:
            return wallpapers_name_en[wallpaper_id];
    }
}

esp_err_t LoadSetting(void) {
    #ifdef HARDCODED_WIFI_SSID
    global_wifi_ssid = HARDCODED_WIFI_SSID;
    global_wifi_password = HARDCODED_WIFI_PASSWORD;
    global_wifi_configed = true;
    #endif
    nvs_handle nvs_arg;
    NVS_CHECK(nvs_open("Setting", NVS_READONLY, &nvs_arg));
    NVS_CHECK(nvs_get_u16(nvs_arg, "Wallpaper", &global_wallpaper));
    NVS_CHECK(nvs_get_u8(nvs_arg, "Language", &global_language));
    NVS_CHECK(nvs_get_u8(nvs_arg, "Timesync", &global_time_synced));
    nvs_get_i8(nvs_arg, "timezone", &global_timezone);

    if (global_wallpaper >= WALLPAPER_NUM) {
        global_wallpaper = DEFAULT_WALLPAPER;
    }
    
    // Custom Wallpaper Path
    size_t path_len = 128;
    char path_buf[128];
    if (nvs_get_str(nvs_arg, "wallpath", path_buf, &path_len) == ESP_OK) {
        global_custom_wallpaper_path = String(path_buf);
    }

    #ifndef HARDCODED_WIFI_SSID
    size_t length = 128;
    char buf[128];
    NVS_CHECK(nvs_get_str(nvs_arg, "ssid", buf, &length));
    global_wifi_ssid = String(buf);
    if (global_wifi_ssid.length() < 1) {
        return ESP_FAIL;
    }
    length = 128;
    NVS_CHECK(nvs_get_str(nvs_arg, "pswd", buf, &length));
    global_wifi_password = String(buf);
    global_wifi_configed = true;
    #endif
    nvs_close(nvs_arg);
    return ESP_OK;
}

esp_err_t SaveSetting(void) {
    nvs_handle nvs_arg;
    NVS_CHECK(nvs_open("Setting", NVS_READWRITE, &nvs_arg));
    NVS_CHECK(nvs_set_u16(nvs_arg, "Wallpaper", global_wallpaper));
    NVS_CHECK(nvs_set_u8(nvs_arg, "Language", global_language));
    NVS_CHECK(nvs_set_u8(nvs_arg, "Timesync", global_time_synced));
    NVS_CHECK(nvs_set_i8(nvs_arg, "timezone", global_timezone));
    NVS_CHECK(nvs_set_str(nvs_arg, "wallpath", global_custom_wallpaper_path.c_str()));
    NVS_CHECK(nvs_set_str(nvs_arg, "ssid", global_wifi_ssid.c_str()));
    NVS_CHECK(nvs_set_str(nvs_arg, "pswd", global_wifi_password.c_str()));
    NVS_CHECK(nvs_commit(nvs_arg));
    nvs_close(nvs_arg);
    return ESP_OK;
}

void SetWifi(String ssid, String password) {
    global_wifi_ssid     = ssid;
    global_wifi_password = password;
    SaveSetting();
}

void SetCustomWallpaperPath(String path) {
    global_custom_wallpaper_path = path;
    SaveSetting();
}

uint8_t isWiFiConfiged(void) {
    return global_wifi_configed;
}

String GetWifiSSID(void) {
    return global_wifi_ssid;
}

String GetWifiPassword(void) {
    return global_wifi_password;
}

bool SyncNTPTime(void) {
    const char *ntpServer    = "time.cloudflare.com";
    const char *ntpServer_zh = "ntp.aliyun.com";
    configTime(global_timezone * 3600, 0, global_language == LANGUAGE_ZH ? ntpServer_zh : ntpServer);

    struct tm timeInfo;

    unsigned long start = millis();
    esp_task_wdt_init(12, true);
    while (!getLocalTime(&timeInfo)) {
        log_d("Time Syncing");
        if (millis() - start > 10000) {  // 超时 10 秒
            log_d("Time Sync failed, server is %s", global_language == LANGUAGE_ZH ? ntpServer_zh : ntpServer);
            return 0;
        }
        delay(100);  // 延迟以避免忙等待
        esp_task_wdt_reset();
    }

    rtc_time_t time_struct;
    time_struct.hour = timeInfo.tm_hour;
    time_struct.min  = timeInfo.tm_min;
    time_struct.sec  = timeInfo.tm_sec;
    M5.RTC.setTime(&time_struct);
    rtc_date_t date_struct;
    date_struct.week = timeInfo.tm_wday;
    date_struct.mon  = timeInfo.tm_mon + 1;
    date_struct.day  = timeInfo.tm_mday;
    date_struct.year = timeInfo.tm_year + 1900;
    M5.RTC.setDate(&date_struct);
    SetTimeSynced(1);

    return 1;
}

uint16_t GetTextSize() {
    return global_reader_textsize;
}

void SetTextSize(uint16_t size) {
    global_reader_textsize = size;
}

const uint8_t *GetLoadingIMG_32x32(uint8_t id) {
    return kIMGLoading[id];
}

void __LoadingAnime_32x32(void *pargs) {
    uint16_t *args = (uint16_t *)pargs;
    uint16_t x     = args[0];
    uint16_t y     = args[1];
    free(pargs);
    M5EPD_Canvas loading(&M5.EPD);
    loading.createCanvas(32, 32);
    
    // Attempt to set background from wallpaper to avoid white box
    bool bg_set = false;
    #ifdef SYSTEM_CONFIG_MAIN_WALLPAPER_PATH
    if (DrawCustomWallpaper(&loading, SYSTEM_CONFIG_MAIN_WALLPAPER_PATH, -x, -y)) {
        bg_set = true;
    }
    #elif defined(SYSTEM_CONFIG_SLEEP_IMAGE_PATH)
    if (DrawCustomWallpaper(&loading, SYSTEM_CONFIG_SLEEP_IMAGE_PATH, -x, -y)) {
        bg_set = true;
    }
    #endif
    
    if (!bg_set) {
         loading.fillCanvas(0); // Fallback to black if no wallpaper found
    }

    // Cache the clean background
    M5EPD_Canvas bg_cache(&M5.EPD);
    bg_cache.createCanvas(32, 32);
    loading.pushToCanvas(0, 0, &bg_cache);

    loading.pushCanvas(x, y, UPDATE_MODE_GL16);
    int anime_cnt = 0;
    uint32_t time = 0;
    while (1) {
        if (millis() - time > 200) {
            time = millis();
            // Restore background
            bg_cache.pushToCanvas(0, 0, &loading);
            // Draw frame with transparency (assuming 15/White is background of icon)
            loading.pushImage(0, 0, 32, 32, 15, GetLoadingIMG_32x32(anime_cnt));
            
            loading.pushCanvas(x, y, UPDATE_MODE_DU4);
            anime_cnt++;
            if (anime_cnt == 16) {
                anime_cnt = 0;
            }
        }

        xSemaphoreTake(_xSemaphore_LoadingAnime, portMAX_DELAY);
        if (_loading_anime_eixt_flag == true) {
            xSemaphoreGive(_xSemaphore_LoadingAnime);
            break;
        }
        xSemaphoreGive(_xSemaphore_LoadingAnime);
    }
    vTaskDelete(NULL);
}

void LoadingAnime_32x32_Start(uint16_t x, uint16_t y) {
    if (_xSemaphore_LoadingAnime == NULL) {
        _xSemaphore_LoadingAnime = xSemaphoreCreateMutex();
    }
    _loading_anime_eixt_flag = false;
    uint16_t *pos            = (uint16_t *)calloc(2, sizeof(uint16_t));
    pos[0]                   = x;
    pos[1]                   = y;
    xTaskCreatePinnedToCore(__LoadingAnime_32x32, "__LoadingAnime_32x32", 16 * 1024, pos, 1, NULL, 0);
}

void LoadingAnime_32x32_Stop() {
    xSemaphoreTake(_xSemaphore_LoadingAnime, portMAX_DELAY);
    _loading_anime_eixt_flag = true;
    xSemaphoreGive(_xSemaphore_LoadingAnime);
    delay(200);
}

// Helper to get jpeg size
static bool getJpegSize(fs::FS &fs, const char *path, uint16_t *width,
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

bool DrawCustomWallpaper(M5EPD_Canvas *canvas, const char *path, int x, int y) {
    if (!SD.exists(path)) return false;
    
    // Generate clean cache filename
    String safePath = String(path);
    if (safePath.startsWith("/")) safePath = safePath.substring(1);
    safePath.replace("/", "_");
    
    String cacheDir = "/cache";
    String cachePath = cacheDir + "/" + safePath + "_" + String(canvas->width()) + "_" + String(canvas->height()) + "_" + String(x) + "_" + String(y) + ".raw";
    
    // 1. Try Loading from Cache
    if (SD.exists(cachePath)) {
        File cFile = SD.open(cachePath);
        if (cFile) {
            if (cFile.size() == canvas->getBufferSize()) {
                // Read raw buffer directly
                cFile.read((uint8_t*)canvas->frameBuffer(1), canvas->getBufferSize());
                cFile.close();
                log_d("Loaded from cache: %s", cachePath.c_str());
                return true;
            }
            cFile.close();
            SD.remove(cachePath); // Invalid cache
        }
    }
    
    bool drawSuccess = false;
    // ... Existing Loading Logic ...
    
    // Check extension
    String p = String(path);
    if (p.endsWith(".png") || p.endsWith(".PNG")) {
         // PNGs don't support scaling in this lib easily, draw as is
        drawSuccess = canvas->drawPngFile(SD, path, x, y);
    } else {
        // JPEG Smart Scaling
        uint16_t w, h;
        jpeg_div_t scale = JPEG_DIV_NONE;

        if (getJpegSize(SD, path, &w, &h)) {
            // Aspect Fill Logic (Center Crop)
            // Goal: Scale such that image covers 540x960
            
            // Calculate ratios
            float ratio_w = (float)w / 540.0f;
            float ratio_h = (float)h / 960.0f;
            float min_ratio = (ratio_w < ratio_h) ? ratio_w : ratio_h;
            
            // Allow scale down only if we stay larger than screen
            // Available scales: 1 (1x), 2 (1/2), 4 (1/4), 8 (1/8)
            // If min_ratio >= 8, we can scale by 8.
            
            if (min_ratio >= 8.0) scale = JPEG_DIV_8;
            else if (min_ratio >= 4.0) scale = JPEG_DIV_4;
            else if (min_ratio >= 2.0) scale = JPEG_DIV_2;
            else scale = JPEG_DIV_NONE;
            
            // Calculate scaled dimensions
            uint16_t sw = w >> (uint8_t)scale;
            uint16_t sh = h >> (uint8_t)scale;
            
            // Calculate Source offsets (in scaled coordinates) to center the 540x960 crop
            uint16_t off_x = (sw > 540) ? (sw - 540) / 2 : 0;
            uint16_t off_y = (sh > 960) ? (sh - 960) / 2 : 0;
            
            // Handle negative x/y (clip top/left)
            int draw_x = x;
            int draw_y = y;
            
            if (draw_x < 0) {
                 off_x += (-draw_x); 
                 draw_x = 0;
            }
            if (draw_y < 0) {
                 off_y += (-draw_y);
                 draw_y = 0;
            }

            drawSuccess = canvas->drawJpgFile(SD, path, draw_x, draw_y, 0, 0, off_x, off_y, scale);
        } else {
            drawSuccess = canvas->drawJpgFile(SD, path, x, y);
        }
    }
    
    // 2. Save to Cache if successful
    if (drawSuccess) {
        if (!SD.exists(cacheDir)) {
            SD.mkdir(cacheDir);
        }
        File cFile = SD.open(cachePath, FILE_WRITE);
        if (cFile) {
            cFile.write((uint8_t*)canvas->frameBuffer(1), canvas->getBufferSize());
            cFile.close();
            log_d("Saved to cache: %s", cachePath.c_str());
        }
    }
    
    return drawSuccess;
}

void Shutdown() {
    log_d("Now the system is shutting down.");
    M5.EPD.Clear();
    
    bool custom_wallpaper = false;
    
    // Check for custom wallpaper defined in config
    M5EPD_Canvas canvas(&M5.EPD);
    canvas.createCanvas(540, 960);
    
#ifdef SYSTEM_CONFIG_SLEEP_IMAGE_PATH
    if (DrawCustomWallpaper(&canvas, SYSTEM_CONFIG_SLEEP_IMAGE_PATH)) {
        canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
        custom_wallpaper = true;
    }
#endif
    
    // Fallback legacy checks if config not used or file missing
    if (!custom_wallpaper) {
        if (DrawCustomWallpaper(&canvas, "/wallpaper.jpg")) {
             canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
             custom_wallpaper = true;
        } else if (DrawCustomWallpaper(&canvas, "/wallpaper.png")) {
             canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
             custom_wallpaper = true;
        }
    }

    if (!custom_wallpaper) {
        M5.EPD.WritePartGram4bpp(92, 182, 356, 300, ImageResource_logo_356x300);
        M5.EPD.UpdateFull(UPDATE_MODE_GC16);
    }
    
    M5.EPD.UpdateFull(UPDATE_MODE_GC16);
    SaveSetting();
    delay(600);
    M5.disableEPDPower();
    M5.disableEXTPower();
    M5.disableMainPower();
    esp_deep_sleep_start();
    while (1);
}