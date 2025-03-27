#ifndef FRAMEWORK_NOTIFICATION_H
#define FRAMEWORK_NOTIFICATION_H

#include <cstdint>

enum class SystemNotification : uint8_t {
    NetworkReady = 1,
    TimeSync,
    ConfigUpdated,
    OTAAvailable,

    // Reserve space for user-defined notifications after this
    MaxReserved
};

#endif
