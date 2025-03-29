#include "TimerService.h"
#include <cstdio>
#include <ctime>
#include "FreeRTOS.h"
#include "timers.h"
#include "PicoTime.h"  // Assuming you have a PicoTime utility for current time
#include "EventManager.h"

// Parses a string like "06:30" or "06:30:00"
TimeOfDay TimeOfDay::fromString(const char* hhmm) {
    TimeOfDay tod{0, 0, 0};
    int parsed = sscanf(hhmm, "%2hhu:%2hhu:%2hhu", &tod.hour, &tod.minute, &tod.second);
    if (parsed < 2) {
        tod = {0, 0, 0};  // fallback to midnight
    }
    return tod;
}

// Converts to seconds since midnight
uint32_t toSeconds(const TimeOfDay& tod) {
    return tod.hour * 3600 + tod.minute * 60 + tod.second;
}

// Returns seconds until next occurrence of this time
uint32_t secondsUntil(const TimeOfDay& tod, time_t currentTime) {
    struct tm now;
    localtime_r(&currentTime, &now);

    uint32_t nowSec = now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec;
    uint32_t targetSec = toSeconds(tod);

    if (targetSec > nowSec) {
        return targetSec - nowSec;
    } else {
        return 86400 - nowSec + targetSec; // next day
    }
}

// Returns seconds until next matching day+time based on mask
uint32_t secondsUntilNextMatch(const TimeOfDay& tod, DaysOfWeek mask, time_t currentTime) {
    struct tm now;
    localtime_r(&currentTime, &now);

    uint8_t today = now.tm_wday; // 0 = Sunday
    uint32_t nowSec = now.tm_hour * 3600 + now.tm_min * 60 + now.tm_sec;
    uint32_t targetSec = toSeconds(tod);

    for (int offset = 0; offset < 7; ++offset) {
        uint8_t checkDay = (today + offset) % 7;
        if (mask & (1 << checkDay)) {
            if (offset == 0 && targetSec <= nowSec) {
                continue; // today, but too late
            }
            uint32_t dayDelay = offset * 86400;
            return dayDelay + ((offset == 0) ? (targetSec - nowSec) : (targetSec + 86400 - nowSec));
        }
    }

    return 86400; // fallback (should not happen)
}

void TimerService::scheduleDailyAt(TimeOfDay time, DaysOfWeek days, const Event& event) {
    time_t now = PicoTime::now(); // You may already have a time util for this
    uint32_t delaySeconds = secondsUntilNextMatch(time, days, now);

    TimerHandle_t handle = xTimerCreate(
        "DailyJob",
        pdMS_TO_TICKS(delaySeconds * 1000),
        pdFALSE,  // one-shot
        new Event(event),
        [](TimerHandle_t xTimer) {
            Event* evt = static_cast<Event*>(pvTimerGetTimerID(xTimer));
            if (evt) {
                EventManager::getInstance().postEvent(*evt);
                delete evt;
            }
            xTimerDelete(xTimer, 0);
        }
    );

    if (handle) {
        xTimerStart(handle, 0);

        // Optionally store TimerJob if rescheduling needed
        TimerJob job{time, days, 0, event, {}, true};
        rescheduleDailyJob(job);
    }
}

void TimerService::scheduleDuration(TimeOfDay start, DaysOfWeek days, uint32_t durationMs,
                                    const Event& startEvent, const Event& stopEvent) {
    time_t now = PicoTime::now();
    uint32_t startDelay = secondsUntilNextMatch(start, days, now);

    // Schedule start
    TimerHandle_t startHandle = xTimerCreate(
        "StartJob",
        pdMS_TO_TICKS(startDelay * 1000),
        pdFALSE,
        new Event(startEvent),
        [](TimerHandle_t xTimer) {
            Event* evt = static_cast<Event*>(pvTimerGetTimerID(xTimer));
            if (evt) {
                EventManager::getInstance().postEvent(*evt);
                delete evt;
            }
            xTimerDelete(xTimer, 0);
        }
    );

    if (startHandle) {
        xTimerStart(startHandle, 0);
    }

    // Schedule stop
    uint32_t stopDelay = startDelay + durationMs / 1000;

    TimerHandle_t stopHandle = xTimerCreate(
        "StopJob",
        pdMS_TO_TICKS(stopDelay * 1000),
        pdFALSE,
        new Event(stopEvent),
        [](TimerHandle_t xTimer) {
            Event* evt = static_cast<Event*>(pvTimerGetTimerID(xTimer));
            if (evt) {
                EventManager::getInstance().postEvent(*evt);
                delete evt;
            }
            xTimerDelete(xTimer, 0);
        }
    );

    if (stopHandle) {
        xTimerStart(stopHandle, 0);
    }

    // Store job for re-scheduling or persistence if needed
    TimerJob job{start, days, durationMs, startEvent, stopEvent, true};
    rescheduleDailyJob(job);
}

void TimerService::rescheduleDailyJob(const TimerJob& job) {
    // Stub for future use:
    // - Save job to in-memory list
    // - Schedule next occurrence after this one fires
    // - Or persist to config store for reboot recovery
}

void TimerService::checkMissedEvents(time_t now) {
    // Pseudocode for v2 when job list exists:
    //
    // for (const auto& job : jobList_) {
    //     if (!job.recurring) continue;
    //
    //     auto lastTime = computeLastScheduledTime(job, now);
    //     if (lastTime + GRACE_WINDOW > now) {
    //         EventManager::instance().postEvent(job.startEvent);
    //         // Optionally also post stopEvent if within duration
    //     }
    // }

    // For now: stub
}