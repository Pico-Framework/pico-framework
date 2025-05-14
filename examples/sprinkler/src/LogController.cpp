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
            UserNotification::ZoneStopped), // manually stopped,
        this);       
    AppContext::get<Logger>()->enableFileLogging("log.txt");
}

void LogController::onEvent(const Event& event) {
    if (!event.isUser()) return;
    printf("[LogController] Event: %d\n", event.userCode());
    
    char msg[128] = {};
    const char* ts = "";  // Timestamp from Logger, or could add PicoTime if needed

    switch (static_cast<UserNotification>(event.userCode())) {
        case UserNotification::ProgramStarted: {
            const char* name = static_cast<const char*>(event.data);
            snprintf(msg, sizeof(msg), "Program \"%s\" started", name);
            break;
        }
        case UserNotification::ProgramCompleted: {
            const char* name = static_cast<const char*>(event.data);
            snprintf(msg, sizeof(msg), "Program \"%s\" completed", name);
            break;
        }
        case UserNotification::RunZoneStart: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "Zone \"%s\" begun", name->c_str());
            break;
        }
        case UserNotification::RunZoneCompleted: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "Zone \"%s\" completed", name->c_str());
            break;
        }
        case UserNotification::ZoneStarted: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "Zone \"%s\" started", name->c_str());
            break;
        }
        case UserNotification::ZoneStopped: {
            const std::string* name = static_cast<const std::string*>(event.data);
            snprintf(msg, sizeof(msg), "Zone \"%s\" stopped", name->c_str());
            break;
        }
        default:
            return;
    }
    printf("[LogController] %s\n", msg);
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

    std::map<std::string, std::pair<std::string, std::string>> zoneEvents;    // name → (time, status)
    std::map<std::string, std::pair<std::string, std::string>> programEvents;
    
    char line[128];
    while (reader->readLine(line, sizeof(line))) {
        printf("[LogController] Read line: %s\n", line);
    
        const char* firstBracket = strchr(line, '[');
        const char* firstClose   = strchr(firstBracket, ']');
        if (!firstBracket || !firstClose) continue;
    
        const char* secondBracket = strchr(firstClose + 1, '[');
        const char* secondClose   = strchr(secondBracket, ']');
        if (!secondBracket || !secondClose) continue;
    
        std::string timestamp(firstBracket + 1, firstClose);
        std::string view(secondClose + 2);  // skip space after [INFO]
    
        const char* keywords[] = {"started", "stopped", "begun", "completed"};
        std::string matchedStatus;
    
        for (const char* kw : keywords) {
            if (view.find(kw) != std::string::npos) {
                matchedStatus = kw;
                break;
            }
        }
        if (matchedStatus.empty()) continue;
    
        if (view.find("Program \"") == 0) {
            size_t q1 = view.find('"');
            size_t q2 = view.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                std::string name = view.substr(q1 + 1, q2 - q1 - 1);
                programEvents[name] = {timestamp, matchedStatus};
            }
        }
    
        if (view.find("Zone \"") == 0) {
            size_t q1 = view.find('"');
            size_t q2 = view.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos) {
                std::string name = view.substr(q1 + 1, q2 - q1 - 1);
                zoneEvents[name] = {timestamp, matchedStatus};
            }
        }
    }
    

    reader->close();

    json obj;

    for (const auto& [k, v] : programEvents)
        obj["programs"][k] = {{"time", v.first}, {"status", v.second}};

    for (const auto& [k, v] : zoneEvents)
        obj["zones"][k] = {{"time", v.first}, {"status", v.second}};

    res.json(obj);
}
