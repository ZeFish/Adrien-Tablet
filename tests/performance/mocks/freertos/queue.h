#pragma once
#include "FreeRTOS.h"
#include <queue>
#include <mutex>
#include <cstring>
#include <vector>
#include <condition_variable>

struct MockQueue {
    size_t itemSize;
    size_t maxCount;
    std::queue<std::vector<uint8_t>> q;
    std::mutex m;
    std::condition_variable cv;
};

inline QueueHandle_t xQueueCreate(size_t count, size_t size) {
    MockQueue* mq = new MockQueue();
    mq->itemSize = size;
    mq->maxCount = count;
    return (QueueHandle_t)mq;
}

inline int xQueueSend(QueueHandle_t xQueue, const void * pvItemToQueue, TickType_t xTicksToWait) {
    if (!xQueue) return pdFAIL;
    MockQueue* mq = (MockQueue*)xQueue;
    std::unique_lock<std::mutex> lock(mq->m);
    // Not implementing full blocking send if full, just push
    std::vector<uint8_t> item(mq->itemSize);
    if (pvItemToQueue) {
        memcpy(item.data(), pvItemToQueue, mq->itemSize);
    }
    mq->q.push(item);
    lock.unlock();
    mq->cv.notify_one();
    return pdPASS;
}

inline int xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait) {
    if (!xQueue) return pdFAIL;
    MockQueue* mq = (MockQueue*)xQueue;
    std::unique_lock<std::mutex> lock(mq->m);

    if (xTicksToWait > 0) {
        // Wait for data
        bool dataAvailable = mq->cv.wait_for(lock, std::chrono::milliseconds(xTicksToWait == portMAX_DELAY ? 1000000 : xTicksToWait), [mq]{ return !mq->q.empty(); });
        if (!dataAvailable) return pdFAIL;
    } else {
        if (mq->q.empty()) return pdFAIL;
    }

    std::vector<uint8_t> item = mq->q.front();
    mq->q.pop();
    if (pvBuffer) {
        memcpy(pvBuffer, item.data(), mq->itemSize);
    }
    return pdPASS;
}
