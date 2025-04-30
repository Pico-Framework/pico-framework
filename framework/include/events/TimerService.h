/**
 * @file TimerService.h
 * @author Ian Archbell
 * @brief Time-of-day and interval-based scheduler for embedded events.
 *
 * Partof the PicoFramework application framework.
 * Provides scheduling of one-shot and repeating events with optional job IDs
 * for tracking and cancellation. Supports scheduling by absolute time,
 * fixed intervals, specific time-of-day across days of the week, and paired
 * start/stop event durations. Integrates with FreeRTOS timers and the
 * EventManager to deliver events in an event-driven, non-blocking manner.
 *
 * @version 0.1
 * @date 2025-03-31
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

/**
 * @note Memory & Timer Lifetime (v0.1):
 *
 * - One-shot timers (`scheduleAt`, `scheduleDailyAt`, `scheduleDuration`) dynamically allocate an `Event` using `new`
 *   and clean it up automatically after posting (inside the timer callback).
 *
 * - Repeating timers (`scheduleEvery`) also allocate an `Event`, which is reused on each invocation.
 *   It is not deleted automatically and is assumed to remain valid for the lifetime of the timer.
 *
 *  It is now possible to cancel scheduled jobs using a job ID which will delete the timer and free the associated event.
 *
 * @todo Possibles for v0.2+:
 * - Persistent job tracking (scheduled jobs stored and reloaded)
 * - Named job handles for cancellation or status checks
 * - Automatic recovery of missed events after reboot
 */

#pragma once

#include <cstdint>
#include <ctime>
#include <string>
#include <unordered_map>
#include <functional>
#include <FreeRTOS.h>
#include <semphr.h>
#include <timers.h>

#include "events/Event.h"


/**
 * @brief Enum for days of the week as bitmask flags.
 */
enum class Day : uint8_t
{
    Sunday = 1 << 0,
    Monday = 1 << 1,
    Tuesday = 1 << 2,
    Wednesday = 1 << 3,
    Thursday = 1 << 4,
    Friday = 1 << 5,
    Saturday = 1 << 6,
};

using DaysOfWeek = uint8_t; ///< Bitmask combining Day flags

/**
 * @brief Represents a time-of-day without a specific date.
 */
struct TimeOfDay
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    /**
     * @brief Parse from a string like "06:30" or "06:30:00".
     *
     * @param hhmm Time string.
     * @return Parsed TimeOfDay.
     */
    static TimeOfDay fromString(const char *hhmm);
};

/**
 * @brief Represents a job scheduled by the TimerService.
 */
struct TimerJob
{
    TimeOfDay startTime;     ///< When the event should start
    DaysOfWeek repeatDays;   ///< Days the job runs (bitmask)
    uint32_t durationMs = 0; ///< Optional run duration in ms
    Event startEvent;        ///< Event to post at start
    Event stopEvent;         ///< Optional stop event
    bool recurring = true;   ///< If true, job repeats
};

/**
 * @brief Central service for time-driven scheduling of framework events.
 */
class TimerService
{
public:


    TimerService();
    ~TimerService() = default;

    void withLock(const std::function<void()>& fn);


    /**
     * @brief Access the singleton instance.
     */
    static TimerService &instance();

    /**
     * @brief Schedule a one-time event at an absolute UNIX timestamp.
     *
     * @param unixTime Epoch time in seconds.
     * @param event Event to trigger.
     */
    void scheduleAt(time_t unixTime, const Event &event);
    void scheduleAt(time_t unixTime, const Event &event, const std::string &jobId);

    /**
     * @brief Schedule a repeating event at fixed intervals.
     *
     * @param intervalMs Interval in milliseconds.
     * @param event Event to trigger repeatedly.
     */
    void scheduleEvery(uint32_t intervalMs, const Event &event);
    void scheduleEvery(uint32_t intervalMs, const Event &event, const std::string &jobId);

    /**
     * @brief Schedule a recurring event based on time-of-day and day mask.
     *
     * @param time Time of day to trigger.
     * @param days Days to run (bitmask).
     * @param event Event to trigger.
     */
    void scheduleDailyAt(TimeOfDay time, DaysOfWeek days, const Event &event);
    void scheduleDailyAt(TimeOfDay time, DaysOfWeek days, const Event &event, const std::string &jobId);

    /**
     * @brief Schedule a start event and stop event with a delay between them.
     *
     * @param start Start time of day.
     * @param days Days to run.
     * @param durationMs Duration after start to trigger stop.
     * @param startEvent Event to post at start.
     * @param stopEvent Event to post after duration.
     */
    void scheduleDuration(TimeOfDay start, DaysOfWeek days, uint32_t durationMs,
                          const Event &startEvent, const Event &stopEvent);
    void scheduleDuration(TimeOfDay start, DaysOfWeek days, uint32_t durationMs,
                          const Event &startEvent, const Event &stopEvent, const std::string &baseJobId);

    /**
     * @brief Detect and fire any missed events after a reboot (TBD).
     *
     * @param now Current UNIX timestamp.
     */
    void checkMissedEvents(time_t now);

    bool cancel(const std::string &jobId);

private:

    SemaphoreHandle_t lock_;
    static StaticSemaphore_t lockBuffer_;

    std::unordered_map<std::string, TimerHandle_t> scheduledJobs; ///< Map of job IDs to TimerHandles
    // Note: TimerHandles are created and managed by FreeRTOS.
    // They will automatically clean up after firing if pdFALSE is passed to xTimerDelete.
    // If pdTRUE is passed, the timer will be deleted immediately after firing.
    // This is useful for one-shot timers.

    /**
     * @brief Placeholder for persistence/rescheduling in the future.
     */
    void rescheduleDailyJob(const TimerJob &job);
};
