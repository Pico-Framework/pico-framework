#include "GpioEventManager.h"
#include "hardware/gpio.h"
#include "Event.h"

GpioEventManager& GpioEventManager::getInstance() {
    static GpioEventManager instance;
    return instance;
}

void GpioEventManager::enableInterrupt(uint pin, uint32_t edgeMask) {
    gpio_set_irq_enabled_with_callback(pin, edgeMask, true, gpio_event_handler);
}

void GpioEventManager::disableInterrupt(uint pin) {
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    listeners.erase(pin);
}

void GpioEventManager::registerCallback(uint pin, GpioCallback cb) {
    listeners[pin].push_back(cb);
}

void GpioEventManager::unregisterAll(uint pin) {
    listeners.erase(pin);
}

void GpioEventManager::gpio_event_handler(uint gpio, uint32_t events) {
    GpioEventData data{gpio, events};

    // Dispatch to listeners
    auto it = listeners.find(gpio);
    if (it != listeners.end()) {
        for (auto& cb : it->second) {
            cb(data);
        }
    }

    // Also send an Event to EventManager if anyone wants to subscribe
    Event evt(EventType::GpioChange, &data, sizeof(data));
    EventManager::getInstance().postEvent(evt); // EventManager knows whther it's an ISR or not
}
