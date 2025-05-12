#include "LogController.h"
#include "framework/AppContext.h"
#include "framework/FrameworkController.h"
#include "events/EventManager.h"
#include "events/Notification.h"
#include "events/Event.h"
#include "RunZone.h" // if used for zone event data

LogController::LogController(Router &r)
    : FrameworkController("LogController", r) {}

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

void LogController::onEvent(const Event &event)
{
    char msg[128] = {};
    const char *ts = ""; // Timestamping is done by Logger internally
    if (event.notification.kind == NotificationKind::User)
    {
        switch (static_cast<UserNotification>(event.notification.user_code))
        {

            case UserNotification::ProgramStarted:
            {
                const char *name = static_cast<const char *>(event.data);
                snprintf(msg, sizeof(msg), "Program \"%s\" started", name);
                break;
            }
            case UserNotification::ProgramCompleted:
            {
                const char *name = static_cast<const char *>(event.data);
                snprintf(msg, sizeof(msg), "Program \"%s\" completed", name);
                break;
            }
            case UserNotification::RunZoneStart:
            {
                const RunZone *rz = static_cast<const RunZone *>(event.data);
                snprintf(msg, sizeof(msg), "Zone \"%s\" started", rz->zone.c_str());
                break;
            }
            case UserNotification::RunZoneCompleted:
            {
                const RunZone *rz = static_cast<const RunZone *>(event.data);
                snprintf(msg, sizeof(msg), "Zone \"%s\" completed", rz->zone.c_str());
                break;
            }
            default:
                return;
            }

            Logger::info(msg);
        }
}
