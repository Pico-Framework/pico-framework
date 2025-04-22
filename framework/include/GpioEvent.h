// GpioEvent.h
#pragma once

#include <cstdint>

struct GpioEvent {
    uint32_t pin;
    uint32_t edge;
};
