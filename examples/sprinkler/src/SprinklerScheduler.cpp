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
    // This function is called by the base class to initialize HTTP routes.
    printf("[SprinklerScheduler] Initializing routes\n");
    router.addRoute("GET", "/api/v1/programs", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &) {
        json arr = json::array();
        for (const auto& prog : programModel->getPrograms()) {
            arr.push_back(prog.toJson());
        }
        res.json(arr);
    });
    
    router.addRoute("GET", "/api/v1/programs/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (name.has_value()) {
            const SprinklerProgram* prog = programModel->get(name.value());
            if (prog) {
                res.json(prog->toJson());
                return;
            }
        }
        res.status(404).text("Program not found");
    });
    
    router.addRoute("POST", "/api/v1/programs", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &) {
        auto json = req.json();
    
        if (!json.contains("name") || !json.contains("start") || !json.contains("days") || !json.contains("zones")) {
            res.status(400).text("Missing required fields");
            return;
        }
    
        SprinklerProgram prog;
        prog.name = json["name"];
        prog.start = TimeOfDay::fromString(json["start"].get<std::string>().c_str());
        prog.days = json["days"].get<DaysOfWeek>();
    
        for (const auto &z : json["zones"]) {
            if (!z.contains("zone") || !z.contains("duration"))
                continue;
            prog.zones.push_back({ z["zone"], z["duration"] });
        }
    
        programModel->saveOrUpdate(prog);
        res.text("Program saved");
    });
    
    router.addRoute("PUT", "/api/v1/programs/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        auto json = req.json();
    
        if (!name.has_value()) {
            res.status(400).text("Missing program name");
            return;
        }
    
        if (!json.contains("start") || !json.contains("days") || !json.contains("zones")) {
            res.status(400).text("Missing required fields");
            return;
        }
    
        SprinklerProgram prog;
        prog.name = name.value();  // enforce URL name
        prog.start = TimeOfDay::fromString(json["start"].get<std::string>().c_str());
        prog.days = json["days"].get<DaysOfWeek>();
    
        for (const auto &z : json["zones"]) {
            if (!z.contains("zone") || !z.contains("duration"))
                continue;
            prog.zones.push_back({ z["zone"], z["duration"] });
        }
    
        programModel->saveOrUpdate(prog);
        res.text("Program updated");
    });
   
    router.addRoute("DELETE", "/api/v1/programs/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (name.has_value()) {
            programModel->remove(name.value());
            res.text("Program deleted");
        } else {
            res.status(400).text("Missing program name");
        }
    });

    router.addRoute("GET", "/(.*)", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        this->router.serveStatic(req, res, match);
    });
    
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

void SprinklerScheduler::onStart()
{
    printf("\n[SprinklerScheduler] Started\n");
}

void SprinklerScheduler::poll()
{
    // Polling logic here
    vTaskDelay(pdMS_TO_TICKS(10));
}