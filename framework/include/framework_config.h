#pragma once
#pragma once

/**
 * @file framework_config.h
 * @brief Includes user configuration first, then system defaults.
 *
 * The user may override any default setting by defining it in
 * `framework_config_user.h` before `framework_config_system.h` is processed.
 */

#ifdef __has_include
  #if __has_include("framework_config_user.h")
    #include "framework_config_user.h"
  #endif
#endif

#include "framework/framework_config_system.h"
