/**
 * @file SprinklerScheduler.cpp
 * @author Ian Archbell
 * @brief Scheduler implementation for activating sprinkler programs.
 * @version 0.1
 * @date 2025-05-09
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "SprinklerScheduler.h"
#include "framework/AppContext.h"
#include "events/EventManager.h"
#include "UserNotification.h"


void SprinklerScheduler::initRoutes()
{
    router.addRoute("GET", "/api/v1/programs", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &)
                    { res.json(programModel->toJson()); });

    router.addRoute("GET", "/api/v1/programs/:name", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    {
                 auto name = match.getParam("name");
                 if (name.has_value()) {
                     if (auto* program = programModel->get(name.value())) {
                         res.json(program->toJson());
                     } else {
                         res.status(404).text("Program not found");
                     }
                 } else {
                     res.status(400).text("Missing program name");
                 } });

    router.addRoute("PUT", "/api/v1/programs/:name", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    {
                    auto name = match.getParam("name");
                    auto json = req.json();
                    if (name.has_value() && programModel->save(name.value(), json)) {
     
                        res.text("Program updated");
                    } else {
                        res.status(400).text("Invalid program format");
                     } });

    router.addRoute("DELETE", "/api/v1/programs/:name", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    {
                     auto name = match.getParam("name");
                     if (name.has_value())
                     {
                         programModel->remove(name.value());
                         res.text("Program deleted");
                     }
                     else
                     {
                         res.status(400).text("Missing program name");
                     } });
}

void SprinklerScheduler::onStart()
{
    while (true)
    {
        checkPrograms();
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

void SprinklerScheduler::checkPrograms()
{
    auto now = PicoTime::now();
    auto tod = PicoTime::toTimeOfDay(now);
    uint32_t today = PicoTime::dayOfWeekBitmask(now);

    // Only run once per actual minute
    uint32_t currentMinute = tod.hour * 60 + tod.minute;
    if (currentMinute == lastCheckMinute)
        return;
    lastCheckMinute = currentMinute;

    for (const auto &program : programModel->getPrograms())
    {
        if ((program.days & today) == 0)
            continue;
        if (program.start.hour == tod.hour && program.start.minute == tod.minute)
        {
            activateProgram(program);
        }
    }
}

void SprinklerScheduler::activateProgram(const SprinklerProgram &program)
{
    for (const auto &zone : program.zones)
    {
        Event e(static_cast<uint8_t>(UserNotification::RunZoneStart), &zone, sizeof(zone));
        AppContext::get<EventManager>()->postEvent(e);
    }
}

void SprinklerScheduler::onEvent(const Event &evt)
{
    if (evt.notification.kind == NotificationKind::User)

        switch (static_cast<UserNotification>(evt.notification.user_code))
        {
        case UserNotification::SchedulerCheck:
            checkSchedule();
            break;

        case UserNotification::RunZoneStart:
            // Handle RunZoneStart event
            break;

        case UserNotification::RunZoneStop:
            // Handle RunZoneStop event
            break;

        case UserNotification::RunZoneStarted:
            // Handle RunZoneStarted event
            break;

        case UserNotification::ZoneStopped:
            // Handle RunZoneStopped event
            break;

        case UserNotification::RunZoneCompleted:
            if (--pendingZones <= 0) {
                Event e(static_cast<uint8_t>(UserNotification::ProgramCompleted));
                AppContext::get<EventManager>()->postEvent(e);
            }
            break;

        case UserNotification::ProgramStarted:
            // Handle ProgramStarted event
            break;

        case UserNotification::ProgramCompleted:
            // Handle ProgramCompleted event
            checkSchedule();
            break;

        default:
            break;
        }
}

void SprinklerScheduler::checkSchedule()
{
    if (!programModel)
        return;

    static time_t lastCheckTime = 0;
    time_t now = PicoTime::now();
    Day currentDay = PicoTime::dayOfWeek(now);
    TimeOfDay currentTime = PicoTime::toTimeOfDay(now);

    auto programs = programModel->all();
    for (const auto &progJson : programs)
    {
        if (!progJson.contains("name") || !progJson.contains("start") || !progJson.contains("zones"))
            continue;

        std::string name = progJson["name"];
        TimeOfDay startTime = TimeOfDay::fromString(progJson["start"].get<std::string>().c_str());

        // Check if today is in the days list
        if (progJson.contains("days"))
        {
            const auto &days = progJson["days"];
            if (std::find(days.begin(), days.end(), currentDay) == days.end())
                continue;
        }

        // Skip if current time hasn't reached startTime
        if (currentTime < startTime)
            continue;

        // Optional: check if this program already ran today (not implemented yet)
        runningProgramName = name;
        pendingZones = progJson["zones"].size();

        // Post events for each zone
        for (const auto &zoneEntry : progJson["zones"])
        {
            const RunZone run = zoneEntry.get<RunZone>();
            Event e(static_cast<uint8_t>(UserNotification::RunZoneStart), &run, sizeof(run));
            AppContext::get<EventManager>()->postEvent(e);
        }
    }
}
