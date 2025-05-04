
# TimerService in PicoFramework

TimerService is a high-level scheduling component designed for embedded applications. It enables your application to schedule future actions using simple time-based declarations, ranging from fixed intervals to day-of-week schedules and start/stop durations.

This guide walks through the design, usage, and best practices for using `TimerService`.

---

## 1. What TimerService Provides

Unlike raw FreeRTOS timers, `TimerService` integrates with the `EventManager` to deliver high-level `Event` objects instead of raw callbacks. This enables non-blocking, event-driven scheduling using familiar application semantics.

Key features:

- One-shot timers (`scheduleAt`)
- Repeating timers (`scheduleEvery`)
- Day-of-week + time-of-day schedules (`scheduleDailyAt`)
- Paired duration-based events (`scheduleDuration`)
- Job ID tracking and cancellation
- Future: persistent job reloading and missed event recovery

---

## 2. How It Works

TimerService wraps `xTimerCreate` from FreeRTOS and internally manages a map of active jobs (`std::map<std::string, TimerHandle_t>`). Each job has a string ID and is automatically cleaned up when completed or canceled.

All events are posted to the `EventManager`, preserving the publish/subscribe model used across the framework.

---

## 3. One-Shot Events with `scheduleAt()`

Schedule a one-time event at a specific `time_t` Unix timestamp:

```cpp
time_t when = PicoTime::now() + 10; // 10 seconds from now
Event e = Event::user(UserNotification::StartPump);

TimerService::instance().scheduleAt(when, e);
```

This event will be delivered once, after the delay elapses.

You can also provide a custom job ID for cancellation or tracking:

```cpp
TimerService::instance().scheduleAt(when, e, "pump_start_10s");
```

---

## 4. Repeating Events with `scheduleEvery()`

Schedule a repeating timer every N milliseconds:

```cpp
TimerService::instance().scheduleEvery(1000, Event::user(UserNotification::Heartbeat));
```

This timer will post an event every second.

Custom job IDs are supported:

```cpp
TimerService::instance().scheduleEvery(30000, Event::user(UserNotification::CheckStatus), "check_status");
```

Note: These timers persist until explicitly canceled.

---

## 5. Daily Scheduling with `scheduleDailyAt()`

You can run a job at a specific time of day, on specific days of the week:

```cpp
TimeOfDay tod = TimeOfDay::fromString("06:30:00");
DaysOfWeek days = Days::Mon | Days::Wed | Days::Fri;

TimerService::instance().scheduleDailyAt(tod, days, Event::user(UserNotification::Irrigate));
```

The service calculates how many seconds remain until the next match and schedules a one-shot. It then reschedules the job for the next day match after it runs.

---

## 6. Duration-Based Start/Stop Events

You can pair two events—a start and a stop—by scheduling a start event followed by a delayed stop:

```cpp
TimeOfDay start = TimeOfDay::fromString("07:00:00");
uint32_t duration = 30 * 60 * 1000; // 30 minutes

TimerService::instance().scheduleDuration(start, Days::All,
    Event::user(UserNotification::ZoneOn),
    Event::user(UserNotification::ZoneOff),
    "morning_watering");
```

This schedules a start event at the next match and a stop event `duration` ms later.

---

## 7. Canceling a Job

To cancel any job created via a schedule call with a `jobId`:

```cpp
bool removed = TimerService::instance().cancel("check_status");
```

This stops and deletes the underlying FreeRTOS timer and removes the job ID from the map.

---

## 8. Thread Safety

TimerService uses a `StaticSemaphore_t`-backed mutex to protect internal state. This ensures:

- You can safely schedule from any task
- Jobs are removed safely when canceled or completed

All access to `scheduledJobs` is protected.

---

## 9. Current Limitations and Roadmap

**Limitations (v0.2):**

- Daily jobs do not persist across reboots
- Missed events (e.g. after power loss) are not restored
- Job metadata is not queryable at runtime

**Planned Enhancements (v0.3+):**

- Persistent job store in JSON
- Rehydration of daily jobs on boot
- Query API for job state and next run time
- Web UI for job scheduling and monitoring

---

## 10. Summary

Use `TimerService` when you need:

| Schedule Type    | Method                  |
|------------------|--------------------------|
| One-shot at time | `scheduleAt()`           |
| Fixed interval   | `scheduleEvery()`        |
| Time of day      | `scheduleDailyAt()`      |
| Start + stop     | `scheduleDuration()`     |

All events are delivered through `EventManager`, and all jobs can be canceled via their ID.

---

## Example: Irrigation Scheduling

```cpp
void MyApp::scheduleIrrigation() {
    TimeOfDay start = TimeOfDay::fromString("06:00");
    TimerService::instance().scheduleDuration(start, Days::Mon | Days::Wed,
        Event::user(UserNotification::Zone1Start),
        Event::user(UserNotification::Zone1Stop),
        "zone1_mw_6am");
}
```

This will automatically turn on zone 1 at 6:00 AM on Monday and Wednesday and turn it off after the configured duration.

