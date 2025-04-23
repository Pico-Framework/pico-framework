#pragma once
#include <stdint.h>

/**
 * @brief Identifies the source of a notification.
 */
enum class NotificationKind : uint8_t {
    System,
    User
};

/**
 * @brief System-defined notification types reserved by the framework.
 */
enum class SystemNotification : uint8_t {
    None = 0,
    NetworkReady,
    TimeSync,
    ConfigUpdated,
    OTAAvailable,
    GpioChange
};

/**
 * @brief A tagged union representing either a system or user-defined notification.
 */
struct Notification {
    NotificationKind kind;

    union {
        SystemNotification system;
        uint8_t user_code;
    };

    Notification() : kind(NotificationKind::System), system(SystemNotification::None) {}
    Notification(SystemNotification s) : kind(NotificationKind::System), system(s) {}
    Notification(uint8_t userCode) : kind(NotificationKind::User), user_code(userCode) {}

    uint8_t code() const {
        return (kind == NotificationKind::System)
            ? static_cast<uint8_t>(system)
            : user_code;
    }
};

template<typename Enum>
inline constexpr uint32_t mask(Enum e) {
    return 1u << static_cast<uint8_t>(e);
}
