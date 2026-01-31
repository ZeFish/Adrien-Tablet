#ifndef _APPS_REGISTRY_H_
#define _APPS_REGISTRY_H_

#include <vector>
#include "core/gui/AppBase.h"
#include "core/system/SystemAppFactory.h"
#include "core/config/SystemConfig.h"

#ifdef ENABLE_APP_HOME_CONTROL
#include "apps/HomeControl/HomeControl.h"
#endif

#ifdef ENABLE_APP_WEATHER
#include "apps/Utilities/WeatherApp.h"
#endif

// Include your app headers here
#include "apps/media/PhotoApp.h"
// #include "my_app.h"

class AppRegistry {
public:
    static std::vector<AppBase*> GetApps() {
        std::vector<AppBase*> apps;
        
        // Default System Apps
#ifdef ENABLE_APP_HOME_CONTROL
        apps.push_back(new HomeControlApp());
#endif

#ifdef ENABLE_APP_WEATHER
        apps.push_back(new WeatherApp());
#endif

#ifdef ENABLE_APP_SETTINGS
        apps.push_back(new SystemAppWrapper("Frame_Setting", ImageResource_main_icon_setting_92x92, Factory_Setting));
#endif

#ifdef ENABLE_APP_KEYBOARD
        apps.push_back(new SystemAppWrapper("Frame_Keyboard", ImageResource_main_icon_keyboard_92x92, Factory_Keyboard));
#endif

#ifdef ENABLE_APP_FACTORY_TEST
        apps.push_back(new SystemAppWrapper("Frame_FactoryTest", ImageResource_main_icon_factorytest_92x92, Factory_FactoryTest));
#endif

#ifdef ENABLE_APP_WIFI_SCAN
        apps.push_back(new SystemAppWrapper("Frame_WifiScan", ImageResource_main_icon_wifi_92x92, Factory_WifiScan));
#endif

#ifdef ENABLE_APP_LIFEGAME
        apps.push_back(new SystemAppWrapper("Frame_Lifegame", ImageResource_main_icon_lifegame_92x92, Factory_LifeGame));
#endif

#ifdef ENABLE_APP_FILE_INDEX
        apps.push_back(new SystemAppWrapper("Frame_FileIndex", ImageResource_main_icon_sdcard_92x92, Factory_SDFile));
#endif

#ifdef ENABLE_APP_COMPARE
        apps.push_back(new SystemAppWrapper("Frame_Compare", ImageResource_main_icon_compare_92x92, Factory_Compare));
#endif

        apps.push_back(new PhotoApp());
        
        // Add your new apps here!

        
        return apps;
    }
};

#endif
