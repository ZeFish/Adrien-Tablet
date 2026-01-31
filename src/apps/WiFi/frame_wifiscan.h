#ifndef _FRAME_WIFISCAN_H_
#define _FRAME_WIFISCAN_H_

#include "core/gui/frame_base.h"
#include "core/gui/epdgui/epdgui.h"

class Frame_WifiScan : public Frame_Base {
   public:
    Frame_WifiScan();
    ~Frame_WifiScan();
    int init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args);
    int scan();
    int run();
    void Connect();
    void DrawItem(EPDGUI_Button *btn, String ssid, int rssi);
    void SetConnected(String ssid, int rssi);

   private:
    EPDGUI_Button *_key_wifi[14];
    uint8_t _language;
    uint32_t _scan_count = 0;
    bool _connect;
    uint8_t _connected;
    String _connect_ssid;
    String _connect_password;
};

#endif  //_FRAME_SETTING_H_