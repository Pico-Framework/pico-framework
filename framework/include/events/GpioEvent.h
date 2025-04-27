// GpioEvent.h
#pragma once

#include <cstdint>

/**
 * @brief Structure representing a GPIO event.
 * It is used to encapsulate information about a GPIO pin and the type of edge event (rising or falling).
 * It is condensed to 4 bytes so that it can be easily passed by value.
 */
struct GpioEvent {
    uint16_t pin;
    uint16_t edge;
};

