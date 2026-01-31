#ifndef _SYSTEM_CONFIG_H_
#define _SYSTEM_CONFIG_H_

/*
 * System Config
 * Central configuration file for compile-time system settings.
 */

// =======================
// System Resources
// =======================

// Font settings
// If true, loads font from SD card "/font.ttf". Otherwise uses embedded binaryttf.
#define SYSTEM_CONFIG_USE_SD_FONT        true
#define SYSTEM_CONFIG_SD_FONT_PATH       "/FuturaBT_Book.ttf"

// Background / Wallpapers
// These IDs correspond to indices in the wallpaper array managed by global_setting
#define SYSTEM_CONFIG_DEFAULT_WALLPAPER_ID  0

// Sleeping Image (Shutdown)
// If defined, tries to load this file from SD card on shutdown.
// If commented out or file missing, falls back to default logic (logo).
#define SYSTEM_CONFIG_SLEEP_IMAGE_PATH   "/wallpaper.jpg"
#define SYSTEM_CONFIG_LOADING_IMAGE_PATH "/loading.jpg"
#define SYSTEM_CONFIG_MAIN_WALLPAPER_PATH "/wallpaper.jpg"

// =======================
// Apps Configuration
// =======================
// Comment out lines to disable specific apps from the launcher

#define ENABLE_APP_HOME_CONTROL
// #define ENABLE_APP_SETTINGS
// #define ENABLE_APP_KEYBOARD
// #define ENABLE_APP_FACTORY_TEST
// #define ENABLE_APP_WIFI_SCAN
#define ENABLE_APP_LIFEGAME
// #define ENABLE_APP_FILE_INDEX
// #define ENABLE_APP_COMPARE
#define ENABLE_APP_WEATHER

// =======================
// System Behavior
// =======================

#define SYSTEM_CONFIG_AUTO_POWER_SAVE    true
#define SYSTEM_CONFIG_SHUTDOWN_TIMEOUT   (5 * 60 * 1000) // 5 minutes
#define SYSTEM_CONFIG_DEFAULT_TIMEZONE   -5
#define SYSTEM_CONFIG_DEVICE_NAME        "Adrien"
#define SYSTEM_CONFIG_HOME_APP_NAME      "Maison"

#ifndef HOME_ASSISTANT_WEATHER_ENTITY
#define HOME_ASSISTANT_WEATHER_ENTITY    "weather.quebec_forecast"
#endif

#endif // _SYSTEM_CONFIG_H_
