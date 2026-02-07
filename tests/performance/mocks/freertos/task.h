#pragma once
#include "FreeRTOS.h"
#include <thread>
#include <iostream>
#include <unistd.h>

inline void vTaskDelay(TickType_t xTicksToDelay) {
    // 1 tick = 1 ms usually
    usleep(xTicksToDelay * 1000);
}

typedef void (*TaskFunction_t)( void * );

inline void xTaskCreate(TaskFunction_t pxTaskCode, const char * const pcName, const uint32_t usStackDepth, void * const pvParameters, unsigned long uxPriority, TaskHandle_t * const pxCreatedTask) {
    // We detach the thread so it runs in background
    std::thread t(pxTaskCode, pvParameters);
    t.detach();
}
