/**
 * @file GpioEventManager.h
 * @brief Posts GPIO change events (rising/falling edge) via EventManager.
 * @version 1.1
 * @date 2025-04-09
 * @copyright Copyright (c) 2025, Ian Archbell
 * @license MIT
 */

#pragma once
#include "pico/stdlib.h"
#include <map>
#include <vector>
#include <functional>
#include "EventManager.h"
#include "GpioEvent.h"

/**
 * @brief GpioEventManager registers interrupts and posts GpioChange events to multiple listeners per pin.
 */
class GpioEventManager
{
public:
    /**
     * @brief Get the singleton instance of GpioEventManager.
     *
     * This is a thread-safe singleton implementation.
     */
    static GpioEventManager &getInstance();

    /**
     * @brief Initialize the GPIO event manager.
     *
     * This declares the callback used by the pico sdk when an interrupt occurs.
     */

    using GpioCallback = std::function<void(const GpioEvent &)>;

    /**
     * @brief Enable GPIO interrupts for a specific pin and edge mask.
     *
     * This registers the pin with the pico SDK and sets up the interrupt handler.
     *
     * @param pin The GPIO pin number to enable interrupts for.
     * @param edgeMask Bitmask of edges to listen for (GPIO_IRQ_EDGE_RISE, GPIO_IRQ_EDGE_FALL).
     */
    void enableInterrupt(uint pin, uint32_t edgeMask);

    /**
     * @brief Disable GPIO interrupts for a specific pin.
     *
     * This unregisters the pin from the pico SDK and stops listening for events.
     *
     * @param pin The GPIO pin number to disable interrupts for.
     */
    void disableInterrupt(uint pin);

    /**
     * @brief Register a callback for GPIO events on a specific pin.
     *
     * This allows multiple listeners to be registered for the same pin.
     *
     * @param pin The GPIO pin number to register the callback for.
     * @param cb The callback function to call when an event occurs.
     */
    void registerCallback(uint pin, GpioCallback cb);

    /**
     * @brief Unregister listeners for a GPIO pin.
     *
     * This removes the the list of listeners for the specified pin.
     *
     * @param pin The GPIO pin number to unregister the callback from.
     * @param cb The callback function to remove.
     */
    void unregisterAll(uint pin);

private:
    GpioEventManager() = default;
    bool handler_set = false;
    
    static void gpio_event_handler(uint gpio, uint32_t events);

    static inline std::map<uint, std::vector<GpioCallback>> listeners;
};
