#include "GpioEventManager.h"
#include <cstdio>
#include "hardware/gpio.h"
#include "Event.h"
#include "EventManager.h"
#include "AppContext.h"


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
    GpioEvent gpioEvent = {
        static_cast<uint16_t>(gpio),
        static_cast<uint16_t>(events)
    };

    //GpioEvent* gpioEvent = new  GpioEvent{static_cast<uint32_t>(gpio), static_cast<uint32_t>(events)};

    // Dispatch to listeners
    auto it = listeners.find(gpio);
    if (it != listeners.end()) {
        for (auto& cb : it->second) {
            cb(gpioEvent);
        }
    }

    // Also send an Event to EventManager if anyone wants to subscribe
    Event evt = Event(SystemNotification::GpioChange, gpioEvent, sizeof(GpioEvent)); // broadcast event to anyone subscribed as target isn't specified
    AppContext::getInstance().getService<EventManager>()->postEvent(evt); // EventManager knows whther it's an ISR or not
}
