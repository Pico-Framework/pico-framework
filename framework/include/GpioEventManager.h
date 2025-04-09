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
 
 /**
  * @brief Event payload for GpioChange events.
  */
 struct GpioEventData {
     uint pin;         ///< GPIO pin number
     uint32_t edge;    ///< GPIO_IRQ_EDGE_RISE or GPIO_IRQ_EDGE_FALL
 };
 
 /**
  * @brief GpioEventManager registers interrupts and posts GpioChange events to multiple listeners per pin.
  */
 class GpioEventManager {
 public:
     static GpioEventManager& getInstance();
 
     using GpioCallback = std::function<void(const GpioEventData&)>;
 
     void enableInterrupt(uint pin, uint32_t edgeMask);
     void disableInterrupt(uint pin);
     void registerCallback(uint pin, GpioCallback cb);
     void unregisterAll(uint pin);
 
 private:
     GpioEventManager() = default;
     static void gpio_event_handler(uint gpio, uint32_t events);
 
     static inline std::map<uint, std::vector<GpioCallback>> listeners;
 };
 