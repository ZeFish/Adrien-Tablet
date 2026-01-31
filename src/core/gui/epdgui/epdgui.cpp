#include <stack>
#include <map>
#include <list>
#include "epdgui.h"

StatusBar* g_status_bar = NULL;
M5EPD_Canvas* g_epd_canvas = NULL;
M5EPD_Canvas* g_client_canvas = NULL;
int STATUS_BAR_HEIGHT = 56;

typedef struct {
    Frame_Base* frame;
    epdgui_args_vector_t args;
} frame_struct_t;

void EPDGUI_Init(void)
{
    g_status_bar = new StatusBar();
    g_epd_canvas = new M5EPD_Canvas(&M5.EPD);
    g_epd_canvas->createCanvas(EPDGUI_SCREEN_WIDTH, EPDGUI_SCREEN_HEIGHT);
    g_client_canvas = new M5EPD_Canvas(&M5.EPD);
    g_client_canvas->createCanvas(EPDGUI_SCREEN_WIDTH, EPDGUI_SCREEN_HEIGHT - STATUS_BAR_HEIGHT);
}

std::list<EPDGUI_Base*> epdgui_object_list;
uint32_t obj_id             = 1;
Frame_Base* wait_for_delete = NULL;
std::stack<Frame_Base*> frame_stack;
std::map<String, frame_struct_t> frame_map;
uint8_t frame_switch_count = 0;
bool _is_auto_update       = true;

uint16_t _is_last_finger_up = 0xFFFF;
uint16_t _last_pos_x = 0xFFFF, _last_pos_y = 0xFFFF;

uint32_t g_last_active_time_millis = 0;

void EPDGUI_AddObject(EPDGUI_Base* object) {
    object->SetID(obj_id);
    obj_id++;
    epdgui_object_list.push_back(object);
}

void EPDGUI_Draw(m5epd_update_mode_t mode) {
    for (std::list<EPDGUI_Base*>::iterator p = epdgui_object_list.begin();
         p != epdgui_object_list.end(); p++) {
        (*p)->Draw(mode);
    }
}

void EPDGUI_Process(void) {
    for (std::list<EPDGUI_Base*>::iterator p = epdgui_object_list.begin();
         p != epdgui_object_list.end(); p++) {
        (*p)->UpdateState(-1, -1);
    }
}

void EPDGUI_Process(int16_t x, int16_t y) {
    for (std::list<EPDGUI_Base*>::iterator p = epdgui_object_list.begin();
         p != epdgui_object_list.end(); p++) {
        (*p)->UpdateState(x, y);
    }
}

void EPDGUI_Clear(void) {
    epdgui_object_list.clear();
}

void EPDGUI_MainLoop(void) {
    if ((!frame_stack.empty()) && (frame_stack.top() != NULL)) {
        Frame_Base* frame = frame_stack.top();
        log_d("Run %s", frame->GetFrameName().c_str());
        EPDGUI_Clear();
        _is_auto_update = true;

        // Initial draw
        // g_status_bar->draw(g_epd_canvas); // Moved below init
        
        // Check if Full Screen
        bool isFullScreen = frame->isFullScreen();

        // Initial draw
        if (isFullScreen) {
            // Full screen app uses the full EPD canvas
            g_epd_canvas->fillCanvas(0); // Clear to black (or white?) - let app decide in init
            frame->init(g_epd_canvas, frame_map[frame->GetFrameName()].args);
            g_epd_canvas->pushCanvas(0, 0, UPDATE_MODE_GC16);
        } else {
            // Standard app with status bar
            g_epd_canvas->fillCanvas(15); // Ensure clean slate (White)
            g_epd_canvas->pushCanvas(0, 0, UPDATE_MODE_GC16); 
            
            frame->init(g_client_canvas, frame_map[frame->GetFrameName()].args);
            g_status_bar->draw(g_epd_canvas);
            g_client_canvas->pushToCanvas(0, STATUS_BAR_HEIGHT, g_epd_canvas);
            g_epd_canvas->pushCanvas(0, 0, UPDATE_MODE_GC16);
        }
        
        // Main loop for the frame
        while (1) {
            if (frame->isRun() == 0) {
                frame->exit();
                log_d("Exit %s", frame->GetFrameName().c_str());
                if (wait_for_delete != NULL) {
                    log_d("delete %s", wait_for_delete->GetFrameName().c_str());
                    delete wait_for_delete;
                    wait_for_delete = NULL;
                }
                // Draw the previous frame
                if((!frame_stack.empty()) && (frame_stack.top() != NULL)) {
                    Frame_Base* prev_frame = frame_stack.top();
                    prev_frame->init(g_client_canvas, frame_map[prev_frame->GetFrameName()].args);
                    g_status_bar->draw(g_epd_canvas);
                    g_client_canvas->pushToCanvas(0, STATUS_BAR_HEIGHT, g_epd_canvas);
                    g_epd_canvas->pushCanvas(0, 0, UPDATE_MODE_GC16);
                }
                return;
            }

            frame->run();

            if (M5.TP.available()) {
                M5.TP.update();
                uint16_t is_finger_up = M5.TP.isFingerUp() ? 1 : 0;
                int16_t pos_x        = M5.TP.readFingerX(0);
                int16_t pos_y        = M5.TP.readFingerY(0);
                
                // Adjust touch coordinates
                if (!isFullScreen) {
                    pos_y -= STATUS_BAR_HEIGHT;
                }
                
                // Avoid duplicate events
                if (_is_last_finger_up != is_finger_up || _last_pos_x != pos_x ||
                    _last_pos_y != pos_y) {
                    EPDGUI_UpdateGlobalLastActiveTime();
                    _last_pos_x        = pos_x;
                    _last_pos_y        = pos_y;
                    _is_last_finger_up = is_finger_up;
                    
                    if (is_finger_up) {
                        // Check for Status Bar Back Button (Top-Left) on Release
                        if(pos_y < 0) {
                             if (pos_x < 200 && frame->GetFrameName() != "Frame_Main") {
                                frame->exit();
                                EPDGUI_PopFrame(false); // Do not delete frame, it is reused
                                
                                if (wait_for_delete != NULL) {
                                    delete wait_for_delete;
                                    wait_for_delete = NULL;
                                }
                                if((!frame_stack.empty()) && (frame_stack.top() != NULL)) {
                                    Frame_Base* prev_frame = frame_stack.top();
                                    prev_frame->init(g_client_canvas, frame_map[prev_frame->GetFrameName()].args);
                                    g_status_bar->draw(g_epd_canvas);
                                    g_client_canvas->pushToCanvas(0, STATUS_BAR_HEIGHT, g_epd_canvas);
                                    g_epd_canvas->pushCanvas(0, 0, UPDATE_MODE_GC16);
                                }
                                return;
                            }
                        } else {
                            EPDGUI_Process();
                        }
                    } else {
                        // Finger down/drag
                        if (pos_y >= 0) {
                            EPDGUI_Process(pos_x, pos_y);
                        }
                    }
                }
                M5.TP.flush();
            }

            if (M5.BtnP.wasPressed()) { // Button B (Right-Middle) - Exit to Launcher
                if (frame->GetFrameName() != "Frame_Main") {
                    frame->exit();
                    EPDGUI_PopFrame(false); // Do not delete frame

                    if (wait_for_delete != NULL) {
                        delete wait_for_delete;
                        wait_for_delete = NULL;
                    }
                    if((!frame_stack.empty()) && (frame_stack.top() != NULL)) {
                        Frame_Base* prev_frame = frame_stack.top();
                        prev_frame->init(g_client_canvas, frame_map[prev_frame->GetFrameName()].args);
                        g_status_bar->draw(g_epd_canvas);
                        g_client_canvas->pushToCanvas(0, STATUS_BAR_HEIGHT, g_epd_canvas);
                        g_epd_canvas->pushCanvas(0, 0, UPDATE_MODE_GC16);
                    }
                    return;
                }
            }

            if (!isFullScreen) {
                g_status_bar->update();
            }
            M5.update(); // Update button states
        }
    }
    vTaskDelay(1);
}

void EPDGUI_AddFrame(String name, Frame_Base* frame) {
    frame_struct_t f;
    f.frame = frame;
    frame_map.insert(std::pair<String, frame_struct_t>(name, f));
    vTaskDelay(1);
}

void EPDGUI_AddFrameArg(String name, int n, void* arg) {
    if (frame_map.count(name) == 0) {
        return;
    }
    if (frame_map[name].args.size() > n) {
        frame_map[name].args[n] = arg;
    } else {
        frame_map[name].args.push_back(arg);
    }
    log_d("%d", frame_map[name].args.size());
}

Frame_Base* EPDGUI_GetFrame(String name) {
    if (frame_map.count(name) > 0) {
        return frame_map[name].frame;
    }
    return NULL;
}

void EPDGUI_PushFrame(Frame_Base* frame) {
    frame_stack.push(frame);
}

void EPDGUI_PopFrame(bool isDelete) {
    if (isDelete) {
        wait_for_delete = frame_stack.top();
    }
    frame_stack.pop();
}

void EPDGUI_OverwriteFrame(Frame_Base* frame) {
    while (!frame_stack.empty()) {
        frame_stack.pop();
    }
    frame_stack.push(frame);
}

void EPDGUI_SetAutoUpdate(bool isAuto) {
    _is_auto_update = isAuto;
}

void EPDGUI_UpdateGlobalLastActiveTime() {
    g_last_active_time_millis = millis();
}