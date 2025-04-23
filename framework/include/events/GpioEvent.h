// GpioEvent.h
#pragma once

#include <cstdint>

struct GpioEvent {
    uint16_t pin;
    uint16_t edge;
};

