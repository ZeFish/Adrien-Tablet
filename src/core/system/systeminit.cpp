#include "core/config/SystemConfig.h"
#include "core/system/systeminit.h"
#include "core/gui/epdgui/epdgui.h"
#include "core/AppRegistry.h"
#include "Free_Fonts.h"
#include "core/system/global_setting.h"
#include "core/resources/binaryttf.h"
#include <WiFi.h>
#include "core/system/ha_api.h"

M5EPD_Canvas _initcanvas(&M5.EPD);

QueueHandle_t xQueue_Info = xQueueCreate(20, sizeof(uint32_t));

void WaitForUser(void) {
    SysInit_UpdateInfo("$ERR");
    while (1) {
        M5.update();
        if (M5.BtnP.wasReleased()) {
            SysInit_UpdateInfo("$RESUME");
            return;
        }
    }
}

void Screen_Test(void) {
    Serial.println("Start Screen Life Test...");
    _initcanvas.createCanvas(540, 960);
    _initcanvas.setTextSize(4);
    delay(1000);
    float min = 0;
    while (1) {
        for (uint8_t pos = 0; pos < 2; pos++) {
            for (uint8_t index = 0; index < 16; index++) {
                _initcanvas.fillRect(0, index * 60, 540, 60, index);
            }
            int possition = random(960);
            _initcanvas.drawString("Test Time: " + String(min) + "min", 20,
                                   possition);
            _initcanvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
            delay(10000);
            for (uint8_t index = 0; index < 16; index++) {
                _initcanvas.fillRect(0, index * 60, 540, 60, (15 - index));
            }
            _initcanvas.drawString("Test Time: " + String(min) + "min", 20,
                                   possition);
            _initcanvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
            delay(10000);
        }
        min += 1;
    }
}

void SysInit_Start(void) {
    bool ret = false;
    Serial.begin(115200);
    Serial.flush();
    delay(50);
    Serial.print("M5EPD initializing...");

    pinMode(M5EPD_EXT_PWR_EN_PIN, OUTPUT);
    pinMode(M5EPD_EPD_PWR_EN_PIN, OUTPUT);
    pinMode(M5EPD_KEY_RIGHT_PIN, INPUT);
    pinMode(M5EPD_KEY_PUSH_PIN, INPUT);
    pinMode(M5EPD_KEY_LEFT_PIN, INPUT);
    delay(100);

    M5.enableEXTPower();
    // M5.disableEPDPower();
    // delay(500);
    M5.enableEPDPower();
    delay(1000);

    M5.EPD.begin(M5EPD_SCK_PIN, M5EPD_MOSI_PIN, M5EPD_MISO_PIN, M5EPD_CS_PIN,
                 M5EPD_BUSY_PIN);
    M5.EPD.Clear(true);
    M5.EPD.SetRotation(M5EPD_Driver::ROTATE_90);
    M5.TP.SetRotation(GT911::ROTATE_90);

    if (!digitalRead(39)) {
        delay(10);
        if (!digitalRead(39)) {
            Screen_Test();
        }
    }

    // Initialize SD Card BEFORE starting the UI task so it can access the wallpaper
    bool is_factory_test;
    ret = SD.begin(4, *M5.EPD.GetSPI(), 20000000);
    if (ret == false) {
        is_factory_test = true;
        SetInitStatus(0, 0);
        // log_e("Failed to initialize SD card.");
    } else {
        is_factory_test = SD.exists("/__factory_test_flag__");
    }

    SemaphoreHandle_t xSemaphore_LoadingDone = xSemaphoreCreateBinary();
    
    // Give the loading task time to read the wallpaper from SD before we allow main thread to use SD again
    if (xSemaphore_LoadingDone != NULL) {
        if (xTaskCreatePinnedToCore(SysInit_Loading, "SysInit_Loading", 4096, (void*)xSemaphore_LoadingDone, 5, NULL, 1) == pdPASS) {
            xSemaphoreTake(xSemaphore_LoadingDone, portMAX_DELAY);
        } else {
            // Task creation failed, no contention on SD, proceed immediately
            log_e("Failed to create SysInit_Loading task");
        }
        vSemaphoreDelete(xSemaphore_LoadingDone);
    } else {
        // Semaphore creation failed, fallback to delay
        xTaskCreatePinnedToCore(SysInit_Loading, "SysInit_Loading", 4096, NULL, 5, NULL, 1);
        delay(1500);
    }
    
    vTaskPrioritySet(NULL, 10);
    // SysInit_UpdateInfo("Initializing SD card...");

    SysInit_UpdateInfo("Initializing Touch pad...");
    if (M5.TP.begin(21, 22, 36) != ESP_OK) {
        SetInitStatus(1, 0);
        log_e("Touch pad initialization failed.");
        SysInit_UpdateInfo("[ERROR] Failed to initialize Touch pad.");
        WaitForUser();
    }
    taskYIELD();

    M5.BatteryADCBegin();
    LoadSetting();


    if ((!is_factory_test) && SYSTEM_CONFIG_USE_SD_FONT && SD.exists(SYSTEM_CONFIG_SD_FONT_PATH)) {
        _initcanvas.loadFont(SYSTEM_CONFIG_SD_FONT_PATH, SD);
        SetTTFLoaded(true);
    } else {
        _initcanvas.loadFont(binaryttf, sizeof(binaryttf));
        SetTTFLoaded(false);
        SetLanguage(LANGUAGE_EN);
        is_factory_test = true;
    }
    taskYIELD();

    // Always show initializing message, regardless of factory test mode
    SysInit_UpdateInfo("Initializing system...");
    taskYIELD();

    _initcanvas.createRender(26, 128);

    ha_setup();

    EPDGUI_Init();
    Frame_Main *frame_main = new Frame_Main();
    EPDGUI_PushFrame(frame_main);

    // WiFi initialization - happens regardless of factory test mode
    if (isWiFiConfiged()) {
        SysInit_UpdateInfo("Connect to " + GetWifiSSID() + "...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(GetWifiSSID().c_str(), GetWifiPassword().c_str());
        uint32_t t = millis();
        while (1) {
            if (millis() - t > 15000) {
                log_w("WiFi connection timeout");
                break;
            }

            if (WiFi.status() == WL_CONNECTED) {
                log_i("WiFi connected to %s", GetWifiSSID().c_str());
                Frame_WifiScan* frame_wifiscan = (Frame_WifiScan*)EPDGUI_GetFrame("Frame_WifiScan");
                if (frame_wifiscan != NULL) {
                    frame_wifiscan->SetConnected(GetWifiSSID(), WiFi.RSSI());
                }
                break;
            }

            vTaskDelay(500);
        }
    } else {
        log_w("WiFi not configured - skipping connection");
    }

    log_d("done");

    while (uxQueueMessagesWaiting(xQueue_Info)) {
        vTaskDelay(100);
    }

    // Always send $OK to dismiss loading screen
    SysInit_UpdateInfo("$OK");

    Serial.println("OK");

    delay(500);
}

void SysInit_Loading(void *pvParameters) {
    const uint16_t kPosy = 548;
    const uint8_t *kLD[] = {
        ImageResource_loading_01_96x96, ImageResource_loading_02_96x96,
        ImageResource_loading_03_96x96, ImageResource_loading_04_96x96,
        ImageResource_loading_05_96x96, ImageResource_loading_06_96x96,
        ImageResource_loading_07_96x96, ImageResource_loading_08_96x96,
        ImageResource_loading_09_96x96, ImageResource_loading_10_96x96,
        ImageResource_loading_11_96x96, ImageResource_loading_12_96x96,
        ImageResource_loading_13_96x96, ImageResource_loading_14_96x96,
        ImageResource_loading_15_96x96, ImageResource_loading_16_96x96};

    M5EPD_Canvas LoadingIMG(&M5.EPD);
    M5EPD_Canvas Info(&M5.EPD);
    LoadingIMG.createCanvas(96, 96);
    Info.createCanvas(540, 50);
    Info.setFreeFont(FF18);
    Info.setTextSize(1);
    Info.setTextDatum(CC_DATUM);
    Info.setTextColor(15);

    // Try to load custom loading image from SD
    bool customLoaded = false;
    #ifdef SYSTEM_CONFIG_LOADING_IMAGE_PATH
    M5EPD_Canvas BG(&M5.EPD);
    BG.createCanvas(540, 960);
    if (DrawCustomWallpaper(&BG, SYSTEM_CONFIG_LOADING_IMAGE_PATH)) {
            BG.pushCanvas(0, 0, UPDATE_MODE_GC16);
            customLoaded = true;
    }
    #endif

    if (!customLoaded) {
        M5.EPD.WritePartGram4bpp(92, 182, 356, 300, ImageResource_logo_356x300);
        M5.EPD.UpdateFull(UPDATE_MODE_GC16);
    }

    SemaphoreHandle_t xSemaphore_LoadingDone = (SemaphoreHandle_t)pvParameters;
    if (xSemaphore_LoadingDone != NULL) {
        xSemaphoreGive(xSemaphore_LoadingDone);
    }

    int i = 0;
    char *p;
    uint32_t time = 0;
    while (1) {
        vTaskPrioritySet(NULL, 15);
        if (millis() - time > 250) {
            time = millis();
            LoadingIMG.pushImage(0, 0, 96, 96, kLD[i]);
            LoadingIMG.pushCanvas(220, kPosy + 80, UPDATE_MODE_DU4);
            i++;
            if (i == 16) {
                i = 0;
            }
        }

        if (xQueueReceive(xQueue_Info, &p, 0)) {
            String str(p);
            free(p);
            if (str.indexOf("$OK") >= 0) {
                LoadingIMG.pushImage(0, 0, 96, 96,
                                     ImageResource_loading_success_96x96);
                LoadingIMG.pushCanvas(220, kPosy + 80, UPDATE_MODE_GL16);
                break;
            } else if (str.indexOf("$ERR") >= 0) {
                LoadingIMG.pushImage(0, 0, 96, 96,
                                     ImageResource_loading_error_96x96);
                LoadingIMG.pushCanvas(220, kPosy + 80, UPDATE_MODE_GL16);
                LoadingIMG.fillCanvas(0);
                while (1) {
                    if (xQueueReceive(xQueue_Info, &p, 0)) {
                        String str(p);
                        free(p);
                        if (str.indexOf("$RESUME") >= 0) {
                            LoadingIMG.pushCanvas(220, kPosy + 80,
                                                  UPDATE_MODE_GC16);
                            break;
                        }
                    }
                }
            } else {
                Info.fillCanvas(0);
                Info.drawString(str, 270, 20);
                Info.pushCanvas(0, kPosy, UPDATE_MODE_DU);
            }
        }
        vTaskPrioritySet(NULL, 5);
        vTaskDelay(10);
    }
    vTaskDelete(NULL);
}

void SysInit_UpdateInfo(String info) {
    char *p = (char *)malloc(info.length() + 1);
    memcpy(p, info.c_str(), info.length());
    p[info.length()] = '\0';
    if (xQueueSend(xQueue_Info, &p, 0) == 0) {
        free(p);
    }
}