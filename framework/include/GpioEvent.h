// GpioEvent.h
#pragma once

#include <cstdint>

enum class GpioEdge : uint32_t { Rising, Falling };

struct GpioEvent {
    uint32_t pin;
    uint32_t edge;
};
