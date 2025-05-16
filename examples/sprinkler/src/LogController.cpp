#include "LogController.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "framework/AppContext.h"
#include "framework/FrameworkController.h"
#include "events/EventManager.h"
#include "events/Notification.h"
#include "events/Event.h"
#include "RunZone.h" // if used for zone event data
#include "ZoneModel.h"

LogController::LogController(Router &r)
    : FrameworkController("LogController", r) {}

    
void LogController::initRoutes() {
    router.addRoute("GET", "/api/v1/logs/summary", [this](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
        handleSummary(req, res);
    });
    router.addRoute("GET", "/api/v1/logs/summaryJson", [this](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
        handleSummaryJson(req, res);
    });
}   

void LogController::onStart()
{
    printf("[LogController] Starting\n"); 
    EventManager *eventManager = AppContext::get<EventManager>();
 
    eventManager->subscribe(
        eventMask(
            UserNotification::ProgramStarted,
            UserNotification::ProgramCompleted,
            UserNotification::RunZoneStart,
            UserNotification::RunZoneCompleted,
            UserNotification::ZoneStarted, // manually started
            UserNotification::ZoneStopped, // manually stopped,
            UserNotification::RunZoneStarted,
            UserNotification::RunZoneCompleted,
            UserNotification::RunProgram),
        this);       
    AppContext::get<Logger>()->enableFileLogging("log.txt");
}

void LogController::onEvent(const Event& event) {
    if (!event.isUser()) return;
    
    char msg[128] = {};
    const char* ts = "";  // Timestamp from Logger, or could add PicoTime if needed

    switch (static_cast<UserNotification>(event.userCode())) {
        case UserNotification::ProgramStarted: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "[ProgramStarted] Program \"%s\" started", name->c_str());
            break;
        }
        case UserNotification::ProgramCompleted: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "[ProgramCompleted] Program \"%s\" completed", name->c_str());
            break;
        }
        case UserNotification::RunZoneStart: {
            const RunZone* rz = static_cast<const RunZone*>(event.data);
            snprintf(msg, sizeof(msg), "[RunZoneStart] Zone \"%s\" begun", rz->zone.c_str());
            break;
        }
        case UserNotification::RunZoneCompleted: {
            const RunZone* rz = static_cast<const RunZone*>(event.data);
            snprintf(msg, sizeof(msg), "[RunZoneCompleted] Zone \"%s\" completed", rz->zone.c_str());
            break;
        }
        case UserNotification::ZoneStarted: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "[ZoneStarted] Zone \"%s\" started", name->c_str());
            break;
        }
        case UserNotification::ZoneStopped: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "[ZoneStopped] Zone \"%s\" stopped", name->c_str());
            break;
        }
        case UserNotification::RunZoneStarted: {
            const RunZone* zone = static_cast<const RunZone*>(event.data);
            snprintf(msg, sizeof(msg), "[RunZoneStarted] RunZone \"%s\" started for %d seconds", zone->zone.c_str(), zone->duration);
            break;
        }
        case UserNotification::RunProgram: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "[RunProgram] Run program \"%s\"", name->c_str());
            break;  
        }
        default:
            return;
    }
    AppContext::get<Logger>()->info(msg); 
}

void LogController::handleSummary(HttpRequest& req, HttpResponse& res) {
    auto* storage = AppContext::get<StorageManager>();
    auto reader = storage->openReader("log.txt");
    if (!reader) {
        res.status(500).send("Unable to open log file");
        return;
    }

    std::string logText;
    char line[128];
    while (reader->readLine(line, sizeof(line))) {
        logText += line;
        logText += '\n';
    }

    reader->close();
    res.setContentType("text/plain").send(logText);
}

void LogController::handleSummaryJson(HttpRequest& req, HttpResponse& res) {
    auto* storage = AppContext::get<StorageManager>();
    auto reader = storage->openReader("log.txt");
    if (!reader) {
        res.status(500).json({{"error", "Unable to open log file"}});
        return;
    }

    std::map<std::string, std::pair<std::string, std::string>> zoneEvents;

    char line[128];
    while (reader->readLine(line, sizeof(line))) {
        const char* firstBracket = strchr(line, '[');
        const char* firstClose   = strchr(firstBracket, ']');
        if (!firstBracket || !firstClose) continue;

        const char* secondBracket = strchr(firstClose + 1, '[');
        const char* secondClose   = strchr(secondBracket, ']');
        if (!secondBracket || !secondClose) continue;

        std::string timestamp(firstBracket + 1, firstClose);
        std::string view(secondClose + 2);  // skip space after second [INFO]

        // Only interested in ZoneStarted and ZoneStopped
        if (view.find("ZoneStarted") == std::string::npos &&
            view.find("ZoneStopped") == std::string::npos)
            continue;

        // Now extract the zone name: e.g., Zone "Front Lawn" started
        if (view.find("Zone \"") != std::string::npos) {
            size_t q1 = view.find('"');
            size_t q2 = view.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                std::string name = view.substr(q1 + 1, q2 - q1 - 1);
                std::string status = (view.find("ZoneStarted") != std::string::npos) ? "started" : "stopped";
                zoneEvents[name] = {timestamp, status};
            }
        }
    }

    reader->close();

    json obj = {
        {"zones", json::object()},
        {"programs", json::object()}  // included for consistency even if unused
    };

    for (const auto& [name, entry] : zoneEvents) {
        obj["zones"][name] = {
            {"time", entry.first},
            {"status", entry.second}
        };
    }

    res.json(obj);
}
