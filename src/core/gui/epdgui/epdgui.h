#ifndef _EPDGUI_H_
#define _EPDGUI_H_

#include <M5EPD.h>
#include "epdgui_base.h"
#include "../frame_base.h"
#include "../StatusBar.h"

#define EPDGUI_SCREEN_WIDTH 540
#define EPDGUI_SCREEN_HEIGHT 960

void EPDGUI_AddObject(EPDGUI_Base* object);
void EPDGUI_Draw(m5epd_update_mode_t mode = UPDATE_MODE_GC16);
void EPDGUI_Process(void);
void EPDGUI_Process(int16_t x, int16_t y);
void EPDGUI_Clear(void);
void EPDGUI_Run(Frame_Base* frame);
void EPDGUI_MainLoop(void);
void EPDGUI_PushFrame(Frame_Base* frame);
void EPDGUI_PopFrame(bool isDelete = true);
void EPDGUI_OverwriteFrame(Frame_Base* frame);
void EPDGUI_AddFrame(String name, Frame_Base* frame);
void EPDGUI_AddFrameArg(String name, int n, void* arg);
Frame_Base* EPDGUI_GetFrame(String name);
void EPDGUI_SetAutoUpdate(bool isAuto);
void EPDGUI_UpdateGlobalLastActiveTime();
void EPDGUI_Init(void);

extern StatusBar* g_status_bar;
extern uint32_t g_last_active_time_millis;
extern int STATUS_BAR_HEIGHT;

#endif  //_EPDGUI_H_