#pragma once

/**
 * @file framework_config.h
 * @brief Delegates to user or system configuration
 *
 * Users should provide their own `framework_config_user.h` if overriding
 * defaults. Otherwise the internal `framework_config_system.h` is used.
 */
#define TRACE_TimeManager 0
#define DETECT_LOCAL_TIMEZONE 1