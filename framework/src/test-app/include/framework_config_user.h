#pragma once

/**
 * @file framework_config.h
 * @brief Delegates to user or system configuration
 *
 * Users should provide their own `framework_config_user.h` if overriding
 * defaults. Otherwise the internal `framework_config_system.h` is used.
 */
#define TRACE_HttpServer 0
#define TRACE_Router 0
#define TRACE_HttpRequest 0
#define TRACE_HttpResponse 0
#define DETECT_LOCAL_TIMEZONE 1