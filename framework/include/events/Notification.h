#pragma once
#include <stdint.h>
#include "port/FreeRTOSConfig.h"

//      * @param userCode User-defined notification code

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
    NetworkDown,
    TimeValid,
    LocalTimeValid,
    TimeSync,
    TimeInvalid,
    WaitForTimeout,
    HttpServerStarted,
    GpioChange,
    Count
};

static_assert(static_cast<int>(SystemNotification::Count) <= configTASK_NOTIFICATION_ARRAY_ENTRIES,
              "Too many NotificationKind values for FreeRTOS notification slots.");

/**
 * @brief A tagged union representing either a system or user-defined notification.
 */
struct Notification {
    NotificationKind kind;

    union {
        SystemNotification system;
        uint8_t user_code;
    };

    /**
     * @brief Default constructor initializes to a system notification of None.
     * Use the other constructors to specify a system or user notification.
     */
    Notification() : kind(NotificationKind::System), system(SystemNotification::None) {}

    /**
     * @brief Construct a Notification with a specific system notification type.
     *
     * @param s The system notification type.
     */
    Notification(SystemNotification s) : kind(NotificationKind::System), system(s) {}

    /**
     * @brief Construct a Notification with a user-defined code.
     *
     * @param userCode The user-defined notification code.
     */
    Notification(uint8_t userCode) : kind(NotificationKind::User), user_code(userCode) {}

    /**
     * @brief Get the notification code.
     * If it's a system notification, returns the system code.
     * If it's a user notification, returns the user-defined code.
     *
     * @return uint8_t The notification code.
     */
    uint8_t code() const {
        return (kind == NotificationKind::System)
            ? static_cast<uint8_t>(system)
            : user_code;
    }
};

/**
 * @brief Helper function to create an event mask from an enum value.
 * This is used to generate a bitmask for event handling.
 *
 * @tparam Enum The enumeration type.
 * @param e The enum value to convert to a bitmask.
 * @return uint32_t The bitmask corresponding to the enum value.
 */
template<typename Enum>
inline constexpr uint32_t eventMask(Enum e) {
    return 1u << static_cast<uint32_t>(static_cast<std::underlying_type_t<Enum>>(e));
}


// Multi-value variadic overload
template<typename Enum, typename... Rest>
inline constexpr uint32_t eventMask(Enum first, Rest... rest) {
    return eventMask(first) | eventMask(rest...);
}

