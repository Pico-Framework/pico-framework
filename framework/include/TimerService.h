#pragma once

#include <cstdint>
#include "Event.h"  // Your framework's Event type
#include <ctime>

// Bitmask for days of the week
enum class Day : uint8_t {
    Sunday    = 1 << 0,
    Monday    = 1 << 1,
    Tuesday   = 1 << 2,
    Wednesday = 1 << 3,
    Thursday  = 1 << 4,
    Friday    = 1 << 5,
    Saturday  = 1 << 6,
};

using DaysOfWeek = uint8_t; // Combine with bitwise OR of Day

// Represents a clock time without a date
struct TimeOfDay {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    static TimeOfDay fromString(const char* hhmm);
};

// Represents a scheduled job (internal handle could be added later)
struct TimerJob {
    TimeOfDay startTime;
    DaysOfWeek repeatDays;         // Bitmask
    uint32_t durationMs = 0;       // Optional duration for stop event
    Event startEvent;
    Event stopEvent;
    bool recurring = true;
};

// TimerService class
class TimerService {
public:
    static TimerService& instance();

    // Schedule a one-time event at an absolute Unix timestamp
    void scheduleAt(time_t unixTime, const Event& event);

    // Schedule a repeating event at fixed intervals (ms)
    void scheduleEvery(uint32_t intervalMs, const Event& event);

    // Schedule an event daily (or on specified days) at a time of day
    void scheduleDailyAt(TimeOfDay time, DaysOfWeek days, const Event& event);

    // Schedule start + stop events based on duration
    void scheduleDuration(TimeOfDay start, DaysOfWeek days, uint32_t durationMs,
                          const Event& startEvent, const Event& stopEvent);

    // Trigger any missed events after a reboot (e.g., if within grace period)
    void checkMissedEvents(time_t now);

private:
    TimerService() = default;
    void rescheduleDailyJob(const TimerJob& job);
};
