#include "LogController.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "framework/AppContext.h"
#include "framework/FrameworkController.h"
#include "events/EventManager.h"
#include "events/Notification.h"
#include "events/Event.h"
#include "RunZone.h" // if used for zone event data

LogController::LogController(Router &r)
    : FrameworkController("LogController", r) {}

    void LogController::initRoutes() {
        router.addRoute("GET", "/api/v1/logs/summary", [this](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
            handleSummary(req, res);
        });
    }   

void LogController::onStart()
{
    EventManager *eventManager = AppContext::get<EventManager>();
    eventManager->subscribe(
        eventMask(
            SystemNotification::NetworkReady,
            SystemNotification::TimeValid,
            SystemNotification::TimeSync,
            SystemNotification::TimeInvalid),
        this);
}

void LogController::onEvent(const Event& event) {
    if (!event.isUser()) return;

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
            const RunZone* rz = static_cast<const RunZone*>(event.data);
            snprintf(msg, sizeof(msg), "Zone \"%s\" started", rz->zone.c_str());
            break;
        }
        case UserNotification::RunZoneCompleted: {
            const RunZone* rz = static_cast<const RunZone*>(event.data);
            snprintf(msg, sizeof(msg), "Zone \"%s\" completed", rz->zone.c_str());
            break;
        }
        default:
            return;
    }

    Logger::info(msg);
}

void LogController::handleSummary(HttpRequest& req, HttpResponse& res) {
    auto* storage = AppContext::get<StorageManager>();
    auto reader = storage->openReader("log.txt");
    if (!reader) {
        res.status(500).json({{"error", "Unable to open log file"}});
        return;
    }

    std::map<std::string, std::string> programTimes;
    std::map<std::string, std::string> zoneTimes;

    char line[128];
    while (reader->readLine(line, sizeof(line))) {
        const char* timeStart = strchr(line, '[');
        const char* timeEnd = strchr(line, ']');
        if (!timeStart || !timeEnd || timeEnd <= timeStart + 1) continue;

        std::string timestamp(timeStart + 1, timeEnd);
        std::string view(line + (timeEnd - line) + 2);

        if (view.find("Program \"") == 0 && view.find("completed") != std::string::npos) {
            size_t q1 = view.find('"');
            size_t q2 = view.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos)
                programTimes[view.substr(q1 + 1, q2 - q1 - 1)] = timestamp;
        }

        if (view.find("Zone \"") == 0 && view.find("completed") != std::string::npos) {
            size_t q1 = view.find('"');
            size_t q2 = view.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos)
                zoneTimes[view.substr(q1 + 1, q2 - q1 - 1)] = timestamp;
        }
    }

    reader->close();

    json obj;
    for (const auto& [k, v] : programTimes) obj["programs"][k] = v;
    for (const auto& [k, v] : zoneTimes) obj["zones"][k] = v;

    res.json(obj);
}