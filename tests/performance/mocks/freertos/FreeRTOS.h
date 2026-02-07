#pragma once
#include <stdint.h>

typedef uint32_t TickType_t;
#define portMAX_DELAY 0xFFFFFFFF
#define portTICK_PERIOD_MS 1
#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
#define pdFAIL 0

typedef void* QueueHandle_t;
typedef void* TaskHandle_t;
typedef void* BaseType_t;
