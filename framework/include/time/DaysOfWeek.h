#pragma once

#include <cstdint>

/**
 * @file DaysOfWeek.h
 * @brief Defines a bitmask enum for days of the week.
 * @author Ian Archbell
 * @version 1.0
 * @date 2025-05-10
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 */

/**
 * @brief Enum for days of the week as bitmask flags.
 */
enum class Day : uint8_t {
    Sunday    = 1 << 0,
    Monday    = 1 << 1,
    Tuesday   = 1 << 2,
    Wednesday = 1 << 3,
    Thursday  = 1 << 4,
    Friday    = 1 << 5,
    Saturday  = 1 << 6,
};

/**
 * @brief Type alias for a set of days (bitmask).
 */
using DaysOfWeek = uint8_t;
