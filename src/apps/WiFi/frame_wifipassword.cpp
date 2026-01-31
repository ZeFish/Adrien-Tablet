#include "apps/WiFi/frame_wifipassword.h"

void key_passwordclear_cb(epdgui_args_vector_t &args) {
    ((EPDGUI_Textbox *)(args[0]))->SetText("");
}

Frame_WifiPassword::Frame_WifiPassword(bool isHorizontal) : Frame_Base() {
    _frame_name      = "Frame_WifiPassword";
    uint8_t language = GetLanguage();
    if (isHorizontal) {
        inputbox = new EPDGUI_Textbox(84, 25, 712, 250);
        if (language == LANGUAGE_JA)
            key_textclear = new EPDGUI_Button("削除", 804, 25, 72, 120);
        else if (language == LANGUAGE_ZH)
            key_textclear = new EPDGUI_Button("清除", 804, 25, 72, 120);
        else
            key_textclear = new EPDGUI_Button("CLR", 804, 25, 72, 120);
    } else {
        const uint16_t kKeyBaseY = 176;
        inputbox                 = new EPDGUI_Textbox(4, 100, 532, 60);
        if (language == LANGUAGE_JA)
            key_textclear = new EPDGUI_Button("削除", 4, kKeyBaseY, 260, 52);
        else if (language == LANGUAGE_ZH)
            key_textclear = new EPDGUI_Button("清除", 4, kKeyBaseY, 260, 52);
        else
            key_textclear = new EPDGUI_Button("CLR", 4, kKeyBaseY, 260, 52);
    }

    inputbox->SetTextMargin(8, 15, 8, 8);
    inputbox->SetState(EPDGUI_Textbox::EVENT_PRESSED);

    keyboard = new EPDGUI_Keyboard(
        isHorizontal, EPDGUI_Keyboard::STYLE_INPUTMODE_NEEDCONFIRM);

    key_textclear->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)inputbox);
    key_textclear->Bind(EPDGUI_Button::EVENT_RELEASED, key_passwordclear_cb);

    if (language == LANGUAGE_JA) {
        exitbtn("WLAN");
    } else if (language == LANGUAGE_ZH) {
        exitbtn("无线局域网", 200);
    } else {
        exitbtn("WLAN");
    }

    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);
}

Frame_WifiPassword::~Frame_WifiPassword() {
    delete inputbox;
    delete keyboard;
    delete key_textclear;
}

int Frame_WifiPassword::init(M5EPD_Canvas* canvas, epdgui_args_vector_t &args) {
    _is_run = 1;
    canvas->fillCanvas(0);

    EPDGUI_AddObject(inputbox);
    EPDGUI_AddObject(keyboard);
    EPDGUI_AddObject(_key_exit);
    EPDGUI_AddObject(key_textclear);
    return 6;
}

int Frame_WifiPassword::run(void) {
    Frame_Base::run();
    String data = keyboard->getData();
    if (data.indexOf("\n") >= 0) {
        String *pswd = new String(inputbox->GetText());
        EPDGUI_AddFrameArg("Frame_WifiScan", 0, pswd);
        inputbox->SetText("");
        EPDGUI_PopFrame();
        _is_run = 0;
        return 0;
    }
    inputbox->AddText(data);
    return 1;
}
