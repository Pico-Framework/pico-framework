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
    router.addRoute("GET", "/api/v1/programs", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &)
                    {
        json arr = json::array();
        for (const auto& prog : programModel->getPrograms()) {
            arr.push_back(prog.toJson());
        }
        res.json(arr); });

    router.addRoute("GET", "/api/v1/programs/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    {
        auto name = match.getParam("name");
        if (name.has_value()) {
            const SprinklerProgram* prog = programModel->get(name.value());
            if (prog) {
                res.json(prog->toJson());
                return;
            }
        }
        res.status(404).json({ { "success", false }, { "message", "Program not found" }}); });

    router.addRoute("POST", "/api/v1/programs", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &)
                    {
        auto json = req.json();
    
        if (!json.contains("name") || !json.contains("start") || !json.contains("days") || !json.contains("zones")) {
            res.status(400).json({ { "success", false }, { "message", "Missing required fields" }});
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
        rescheduleAll();
        res.status(200).json({ { "success", true }, { "message", "Program saved" }}); });

    router.addRoute("PUT", "/api/v1/programs/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    {
        auto name = match.getParam("name");
        auto json = req.json();
    
        if (!name.has_value()) {
            res.status(400).json({ { "success", false }, { "message", "Missing program name" }});
            return;
        }
    
        if (!json.contains("start") || !json.contains("days") || !json.contains("zones")) {
            res.status(400).json({ { "success", false }, { "message", "Missing required fields" }});
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
        rescheduleAll();
        res.status(200).json({ { "success", true }, { "message", "Program updated" }}); });

    router.addRoute("DELETE", "/api/v1/programs/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    {
        auto name = match.getParam("name");
        if (name.has_value()) {
            programModel->remove(name.value());
            rescheduleAll();
            res.status(200).json({ { "success", true }, { "message", "Program deleted" }});
        } else {
            res.status(400).json({ { "success", false }, { "message", "Missing program name" }});
        } });

    router.addRoute("GET", "/api/v1/next-schedule", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &)
                    {
        auto next = getNextScheduledProgram();
        if (next.has_value()) {
            res.json({
                {"status", "scheduled"},
                {"name", next->first},
                {"time", PicoTime::formatIso8601(next->second)}
            });
        } else {
            res.json({
                {"status", "none"}
            });
        } });

    // Test route to schedule a program for 15 seconds from now    
    router.addRoute("GET", "/api/v1/test-program", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &) {
        SprinklerProgram test;
        test.name = "TestRun";
        test.zones.push_back({ "Front Lawn", 5 });
        test.zones.push_back({ "Back Garden", 8 });
        test.days = 0x7F;
    
        // Schedule for the next full minute
        struct tm t = PicoTime::nowTm();
        t.tm_sec = 0;
        t.tm_min += 1;
        time_t ts = mktime(&t);
        test.start = PicoTime::toTimeOfDay(ts);
    
        programModel->saveOrUpdate(test);
        rescheduleAll();
    
        char buf[64];
        strftime(buf, sizeof(buf), "%H:%M:%S", &t);
        res.json({ {"scheduled", buf}, {"success", true} });
    });
    

    router.addCatchAllGetRoute([this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                               { this->router.serveStatic(req, res, match); });
}

void SprinklerScheduler::rescheduleAll()
{
    // No cancel needed — jobId reuse overwrites
    scheduleAllPrograms();
}


void SprinklerScheduler::scheduleAllPrograms()
{
    if (!programModel) return;

    auto& timerService = *AppContext::get<TimerService>();
    time_t now = PicoTime::now();

    for (const auto& program : programModel->getPrograms())
    {
        for (int i = 0; i < 7; ++i)
        {
            Day day = static_cast<Day>((static_cast<int>(PicoTime::dayOfWeek(now)) + i) % 7);
            if (!(program.days & static_cast<uint8_t>(day)))
                continue;

            struct tm t = PicoTime::nowTm();
            t.tm_hour = program.start.hour;
            t.tm_min = program.start.minute;
            t.tm_sec = 0;
            t.tm_mday += i;

            time_t ts = mktime(&t);
            if (ts > now)
            {
                Event e(static_cast<uint8_t>(UserNotification::RunProgram), &program.name, sizeof(program.name));
                printf("[Scheduler] Scheduling program: %s at %s\n", program.name.c_str(), PicoTime::formatIso8601(ts).c_str());
                timerService.scheduleAt(ts, e, program.name);  // use name as job ID
                break;
            }
        }
    }
}

void SprinklerScheduler::activateProgram(const SprinklerProgram* program)
{
    runningProgramName = program->name;

    // Load zones into queue
    while (!zoneQueue.empty()) zoneQueue.pop();
    for (const auto &zone : program->zones)
        zoneQueue.push(&zone);

    // Post ProgramStarted once
    Event e(static_cast<uint8_t>(UserNotification::ProgramStarted), &runningProgramName, sizeof(runningProgramName));
    AppContext::get<EventManager>()->postEvent(e);
    // Post RunZoneStarted for each zone

    // Start first zone
    if (!zoneQueue.empty())
    {
        const RunZone* next = zoneQueue.front();
        Event e(static_cast<uint8_t>(UserNotification::RunZoneStart), next, sizeof(RunZone));
        AppContext::get<EventManager>()->postEvent(e);
    }
}

void SprinklerScheduler::onEvent(const Event &evt)
{
    if (evt.notification.kind == NotificationKind::User)

        switch (static_cast<UserNotification>(evt.notification.user_code))
        {

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
        {
            if (!zoneQueue.empty()) zoneQueue.pop();
        
            if (!zoneQueue.empty())
            {
                const RunZone* next = zoneQueue.front();
                Event e(static_cast<uint8_t>(UserNotification::RunZoneStart), next, sizeof(RunZone));
                AppContext::get<EventManager>()->postEvent(e);
            }
            else
            {
                printf("[Scheduler] Program %s completed\n", runningProgramName.c_str());
                lastProgramRunName = runningProgramName;
                // Event e(static_cast<uint8_t>(UserNotification::ProgramCompleted), &lastProgramRunName, sizeof(lastProgramRunName));
                // AppContext::get<EventManager>()->postEvent(e);
                //AppContext::get<EventManager>()->postEvent(Event(static_cast<uint8_t>(UserNotification::ProgramCompleted, &lastProgramRunName, sizeof(lastProgramRunName))));
                AppContext::get<EventManager>()->postEvent(
                    Event(static_cast<uint8_t>(UserNotification::ProgramCompleted),
                          &lastProgramRunName,
                          sizeof(lastProgramRunName))
                );
                
                printf("[Scheduler] Posted ProgramCompleted, last program run: %s\n", runningProgramName.c_str());
                runningProgramName.clear();
            }
            break;
        }
            
        case UserNotification::RunProgram:
        {
            const SprinklerProgram *prog = static_cast<const SprinklerProgram *>(evt.data);
            if (prog)
            {
                printf("[Scheduler] Activating program: %s\n", prog->name.c_str());
                activateProgram(prog);
            }
            break;
        }
        case UserNotification::ProgramStarted:
            // Handle ProgramStarted event
            break;

        case UserNotification::ProgramCompleted:
            // Handle ProgramCompleted event
            break;

        default:
            break;
        }
}

void SprinklerScheduler::onStart()
{
    printf("\n[SprinklerScheduler] Started\n");
    auto *eventManager = AppContext::get<EventManager>();
    eventManager->subscribe(eventMask(UserNotification::RunZoneStart, UserNotification::RunZoneStop,
                                       UserNotification::RunZoneStarted, UserNotification::RunZoneCompleted,
                                       UserNotification::ProgramStarted, UserNotification::ProgramCompleted,
                                       UserNotification::RunProgram),
                             this);
    while (!zoneQueue.empty()) zoneQueue.pop();
    runningProgramName.clear();
    scheduleAllPrograms();
}

void SprinklerScheduler::poll()
{
    // Polling logic here
    vTaskDelay(pdMS_TO_TICKS(10));
}

std::optional<std::pair<std::string, time_t>> SprinklerScheduler::getNextScheduledProgramToday() const
{
    if (!programModel)
        return std::nullopt;

    uint32_t now = PicoTime::now();
    const ProgramEvent *next = programModel->getNextEvent(now);
    if (!next)
        return std::nullopt;

    struct tm t = PicoTime::nowTm();
    t.tm_hour = next->start.hour;
    t.tm_min = next->start.minute;
    t.tm_sec = 0;
    time_t ts = mktime(&t);
    return std::make_pair(next->programName, ts);
}

std::optional<std::pair<std::string, time_t>> SprinklerScheduler::getNextScheduledProgram() const
{
    if (!programModel)
        return std::nullopt;

    time_t now = PicoTime::now();
    const ProgramEvent *next = programModel->getNextEvent(now);
    if (!next)
        return std::nullopt;

    for (int i = 0; i < 7; ++i)
    {
        Day day = static_cast<Day>((static_cast<int>(PicoTime::dayOfWeek(now)) + i) % 7);
        if (!(programModel->get(next->programName)->days & static_cast<uint8_t>(day)))
            continue;

        struct tm t = PicoTime::nowTm();
        t.tm_hour = next->start.hour;
        t.tm_min = next->start.minute;
        t.tm_sec = 0;
        t.tm_mday += i;
        time_t ts = mktime(&t);
        if (ts > now)
            return std::make_pair(next->programName, ts);
    }

    return std::nullopt;
}