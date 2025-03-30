#pragma once
#include <cstdint>

enum class FrameworkNotification : uint8_t {
    // Reserved for system/framework use
    None              = 0, 
    NetworkReady      = 1,
    TimeSync          = 2,
    ConfigUpdated     = 3,
    OTAAvailable      = 4,

    // Users are free to add their own from here
    UserBase          = 16,

    // Example user-defined notifications
    ControllerReady   = UserBase,
    ProgramEnd        = 17,
    RunProgram        = 18,
    ZoneEndTime       = 19,
    ProgramStartTime  = 20,
    TimerTick         = 21,
};

// Helper for bitmask subscription
constexpr uint32_t mask(FrameworkNotification n) {
    return 1u << static_cast<uint8_t>(n);
}