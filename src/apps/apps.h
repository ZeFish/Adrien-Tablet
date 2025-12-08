#ifndef _APPS_REGISTRY_H_
#define _APPS_REGISTRY_H_

#include <vector>
#include "app_base.h"
#include "system_apps.h"

// Include your app headers here
// #include "my_app.h"

class AppRegistry {
public:
    static std::vector<AppBase*> GetApps() {
        std::vector<AppBase*> apps;
        
        // Default System Apps
        apps.push_back(new SystemAppWrapper("Frame_Home", ImageResource_main_icon_home_92x92, Factory_Home));
        apps.push_back(new SystemAppWrapper("Frame_Setting", ImageResource_main_icon_setting_92x92, Factory_Setting));
        apps.push_back(new SystemAppWrapper("Frame_Keyboard", ImageResource_main_icon_keyboard_92x92, Factory_Keyboard));
        apps.push_back(new SystemAppWrapper("Frame_FactoryTest", ImageResource_main_icon_factorytest_92x92, Factory_FactoryTest));
        apps.push_back(new SystemAppWrapper("Frame_WifiScan", ImageResource_main_icon_wifi_92x92, Factory_WifiScan));
        apps.push_back(new SystemAppWrapper("Frame_Lifegame", ImageResource_main_icon_lifegame_92x92, Factory_LifeGame));
        apps.push_back(new SystemAppWrapper("Frame_FileIndex", ImageResource_main_icon_sdcard_92x92, Factory_SDFile));
        apps.push_back(new SystemAppWrapper("Frame_Compare", ImageResource_main_icon_compare_92x92, Factory_Compare));
        
        // Add your new apps here!

        
        return apps;
    }
};

#endif
