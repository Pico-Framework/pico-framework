#include "FreeRTOSTask.h"

Task::Task(const char* name, uint16_t stackSize, UBaseType_t priority)
    : _name(name), _stackSize(stackSize), _priority(priority) {}

Task::~Task() {
    if (_handle) {
        vTaskDelete(_handle);
    }
    if (_queue) {
        vQueueDelete(_queue);
    }
}

bool Task::start() {
    return xTaskCreate(taskEntry, _name, _stackSize, this, _priority, &_handle) == pdPASS;
}

void Task::taskEntry(void* pvParams) {
    static_cast<Task*>(pvParams)->run();
    vTaskDelete(nullptr); // Clean up after run ends
}

void Task::suspend() {
    if (_handle) vTaskSuspend(_handle);
}

void Task::resume() {
    if (_handle) vTaskResume(_handle);
}

TaskHandle_t Task::getHandle() const {
    return _handle;
}

// --- Notifications ---
void Task::notify(uint32_t value) {
    xTaskNotify(_handle, value, eSetValueWithOverwrite);
}

// --- Notifications from ISR ---
void Task::notifyFromISR(uint32_t value, BaseType_t* pxHigherPriorityTaskWoken) {
    xTaskNotifyFromISR(_handle, value, eSetValueWithOverwrite, pxHigherPriorityTaskWoken);
}

uint32_t Task::waitForNotification(TickType_t timeout) {
    uint32_t value = 0;
    xTaskNotifyWait(0, 0, &value, timeout);
    return value;
}

// --- Queue Support ---
bool Task::createQueue(size_t itemSize, size_t length) {
    _queue = xQueueCreate(length, itemSize);
    return _queue != nullptr;
}

bool Task::sendToQueue(const void* item, TickType_t timeout) {
    return xQueueSend(_queue, item, timeout) == pdTRUE;
}

bool Task::receiveFromQueue(void* item, TickType_t timeout) {
    return xQueueReceive(_queue, item, timeout) == pdTRUE;
}
