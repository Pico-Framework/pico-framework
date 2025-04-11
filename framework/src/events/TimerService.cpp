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
 * @todo Planned for v0.2+:
 * - Persistent job tracking (scheduled jobs stored and reloaded)
 * - Named job handles for cancellation or status checks
 * - Automatic recovery of missed events after reboot
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
 * @todo Planned for v0.2+:
 * - Persistent job tracking (scheduled jobs stored and reloaded)
 * - Named job handles for cancellation or status checks
 * - Automatic recovery of missed events after reboot
 */

#include "TimerService.h"
#include "PicoTime.h"
#include "EventManager.h"
#include <cstdio>

TimeOfDay TimeOfDay::fromString(const char *hhmm)
{
    TimeOfDay tod{0, 0, 0};
    int parsed = sscanf(hhmm, "%2hhu:%2hhu:%2hhu", &tod.hour, &tod.minute, &tod.second);
    if (parsed < 2)
    {
        tod = {0, 0, 0};
    }
    return tod;
}

// --- Helpers ---

static uint32_t toSeconds(const TimeOfDay &tod)
{
    return tod.hour * 3600 + tod.minute * 60 + tod.second;
}

static uint32_t secondsUntilNextMatch(const TimeOfDay &tod, DaysOfWeek mask, time_t currentTime)
{
    struct tm now;
    localtime_r(&currentTime, &now);
    uint8_t today = now.tm_wday;
    uint32_t nowSec = toSeconds({(uint8_t)now.tm_hour, (uint8_t)now.tm_min, (uint8_t)now.tm_sec});
    uint32_t targetSec = toSeconds(tod);

    for (int offset = 0; offset < 7; ++offset)
    {
        uint8_t checkDay = (today + offset) % 7;
        if (mask & (1 << checkDay))
        {
            if (offset == 0 && targetSec <= nowSec)
                continue;
            return offset * 86400 + ((offset == 0) ? (targetSec - nowSec) : (targetSec + 86400 - nowSec));
        }
    }

    return 86400;
}

/// @copydoc TimerService::instance
TimerService &TimerService::instance()
{
    static TimerService inst;
    return inst;
}

/// @copydoc TimerService::scheduleAt
void TimerService::scheduleAt(time_t unixTime, const Event &event)
{
    std::string defaultId = "at_" + std::to_string(unixTime);
    scheduleAt(unixTime, event, defaultId);
}

void TimerService::scheduleAt(time_t unixTime, const Event &event, const std::string &jobId)
{
    time_t now = PicoTime::now();
    uint32_t delaySeconds = (unixTime > now) ? (unixTime - now) : 0;

    TimerHandle_t handle = xTimerCreate("AtTimer", pdMS_TO_TICKS(delaySeconds * 1000), pdFALSE, new Event(event),
            [](TimerHandle_t xTimer)
            {
                Event *evt = static_cast<Event *>(pvTimerGetTimerID(xTimer));
                if (evt)
                {
                    EventManager::getInstance().postEvent(*evt);
                    delete evt;
                }
                xTimerDelete(xTimer, 0);
            });

    if (handle)
    {
        scheduledJobs[jobId] = handle;
        xTimerStart(handle, 0);
    }
}

/// @copydoc TimerService::scheduleEvery
void TimerService::scheduleEvery(uint32_t intervalMs, const Event &event)
{
    std::string defaultId = "interval_" + std::to_string((uint32_t)event.type);
    scheduleEvery(intervalMs, event, defaultId);
}

void TimerService::scheduleEvery(uint32_t intervalMs, const Event &event, const std::string &jobId)
{
    TimerHandle_t handle = xTimerCreate("EveryTimer", pdMS_TO_TICKS(intervalMs), pdTRUE, new Event(event),
                                        [](TimerHandle_t xTimer)
                                        {
                                            Event *evt = static_cast<Event *>(pvTimerGetTimerID(xTimer));
                                            if (evt)
                                            {
                                                EventManager::getInstance().postEvent(*evt);
                                            }
                                        });

    if (handle)
    {
        scheduledJobs[jobId] = handle;
        xTimerStart(handle, 0);
    }
}

/// @copydoc TimerService::scheduleDailyAt
void TimerService::scheduleDailyAt(TimeOfDay time, DaysOfWeek days, const Event &event)
{
    std::string defaultId = "daily_" + std::to_string((uint32_t)event.type) + "_" + std::to_string(toSeconds(time));
    scheduleDailyAt(time, days, event, defaultId);
}

void TimerService::scheduleDailyAt(TimeOfDay time, DaysOfWeek days, const Event &event, const std::string &jobId)
{
    time_t now = PicoTime::now();
    uint32_t delaySeconds = secondsUntilNextMatch(time, days, now);

    TimerHandle_t handle = xTimerCreate("DailyJob", pdMS_TO_TICKS(delaySeconds * 1000), pdFALSE, new Event(event),
                                        [](TimerHandle_t xTimer)
                                        {
                                            Event *evt = static_cast<Event *>(pvTimerGetTimerID(xTimer));
                                            if (evt)
                                            {
                                                EventManager::getInstance().postEvent(*evt);
                                                delete evt;
                                            }
                                            xTimerDelete(xTimer, 0);
                                        });

    if (handle)
    {
        scheduledJobs[jobId] = handle;
        xTimerStart(handle, 0);
        TimerJob job{time, days, 0, event, {}, true};
        rescheduleDailyJob(job);
    }
}

/// @copydoc TimerService::scheduleDuration
void TimerService::scheduleDuration(TimeOfDay start, DaysOfWeek days, uint32_t durationMs,
                                    const Event &startEvent, const Event &stopEvent)
{
    std::string baseId = "duration_" + std::to_string((uint32_t)startEvent.type);
    scheduleDuration(start, days, durationMs, startEvent, stopEvent, baseId);
}

void TimerService::scheduleDuration(TimeOfDay start, DaysOfWeek days, uint32_t durationMs,
                                    const Event &startEvent, const Event &stopEvent, const std::string &baseId)
{
    time_t now = PicoTime::now();
    uint32_t startDelay = secondsUntilNextMatch(start, days, now);

    std::string startId = baseId + "_start";
    std::string stopId = baseId + "_stop";

    TimerHandle_t startHandle = xTimerCreate("StartJob", pdMS_TO_TICKS(startDelay * 1000), pdFALSE, new Event(startEvent),
                                             [](TimerHandle_t xTimer)
                                             {
                                                 Event *evt = static_cast<Event *>(pvTimerGetTimerID(xTimer));
                                                 if (evt)
                                                 {
                                                     EventManager::getInstance().postEvent(*evt);
                                                     delete evt;
                                                 }
                                                 xTimerDelete(xTimer, 0);
                                             });

    if (startHandle)
    {
        scheduledJobs[startId] = startHandle;
        xTimerStart(startHandle, 0);
    }

    uint32_t stopDelay = startDelay + durationMs / 1000;

    TimerHandle_t stopHandle = xTimerCreate("StopJob", pdMS_TO_TICKS(stopDelay * 1000), pdFALSE, new Event(stopEvent),
                                            [](TimerHandle_t xTimer)
                                            {
                                                Event *evt = static_cast<Event *>(pvTimerGetTimerID(xTimer));
                                                if (evt)
                                                {
                                                    EventManager::getInstance().postEvent(*evt);
                                                    delete evt;
                                                }
                                                xTimerDelete(xTimer, 0);
                                            });

    if (stopHandle)
    {
        scheduledJobs[stopId] = stopHandle;
        xTimerStart(stopHandle, 0);
    }

    TimerJob job{start, days, durationMs, startEvent, stopEvent, true};
    rescheduleDailyJob(job);
}

/// @copydoc TimerService::cancel
bool TimerService::cancel(const std::string &jobId)
{
    auto it = scheduledJobs.find(jobId);
    if (it != scheduledJobs.end())
    {
        xTimerStop(it->second, 0);
        xTimerDelete(it->second, 0);
        scheduledJobs.erase(it);
        return true;
    }
    return false;
}

/// @copydoc TimerService::checkMissedEvents
void TimerService::checkMissedEvents(time_t)
{
    // Not implemented in v0.2
}

/// @copydoc TimerService::rescheduleDailyJob
void TimerService::rescheduleDailyJob(const TimerJob &)
{
    // Not implemented in v0.2
}
