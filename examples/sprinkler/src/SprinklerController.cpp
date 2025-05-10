/**
 * @file SprinklerController.cpp
 * @brief Implementation of HTTP routes for sprinkler program control.
 */

#include "SprinklerController.h"
#include "ZoneModel.h"


void SprinklerController::initRoutes()
{


}

void SprinklerController::onEvent(const Event &event)
{
    // Handle events specific to the SprinklerController
    if (event.notification.kind == NotificationKind::User)
    {
        // Handle user notifications
        switch (static_cast<UserNotification>(event.notification.user_code))
        {
        case UserNotification::RunZoneStart: {
            const RunZone &runZone = *static_cast<const RunZone *>(event.data);

            zoneModel->startZone(runZone.zone, runZone.duration);
            break;
        }

        case UserNotification::RunZoneStarted:
            // confirmation of start
            break;
        
        case UserNotification::RunZoneCompleted:
            // zone completed
            break;
        
        case UserNotification::RunZoneStop:
            // stop a zone
            break;
        
        case UserNotification::ZoneStopped:
            // confirmation of stop
            break;

        default:
            break;
        }
    }
}
