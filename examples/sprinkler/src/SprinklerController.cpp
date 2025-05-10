/**
 * @file SprinklerController.cpp
 * @brief Implementation of HTTP routes for sprinkler program control.
 */

#include "SprinklerController.h"
#include "ZoneModel.h"
#include "framework/AppContext.h"


void SprinklerController::initRoutes()
{
    router.addRoute("GET", "/api/v1/zones", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &) {
        res.json(AppContext::get<ZoneModel>()->all());  // inherited from FrameworkModel
    });
    
    router.addRoute("GET", "/api/v1/zones/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (name.has_value()) {
            const auto& all = AppContext::get<ZoneModel>()->all();
            for (const auto& zone : all) {
                if (zone["name"] == name.value()) {
                    res.json(zone);
                    return;
                }
            }
        }
        res.status(404).text("Zone not found");
    });
    
    router.addRoute("PUT", "/api/v1/zones/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (!name.has_value()) {
            res.status(400).text("Missing zone name");
            return;
        }
    
        auto json = req.json();
        if (!json.contains("name") || !json.contains("gpioPin") || !json.contains("active")) {
            res.status(400).text("Missing required fields");
            return;
        }
    
        Zone zone;
        zone.name = json["name"];
        zone.gpioPin = json["gpioPin"];
        zone.active = json["active"];
    
        if (AppContext::get<ZoneModel>()->updateZone(name.value(), zone)) {
            res.text("Zone updated");
        } else {
            res.status(404).text("Zone not found");
        }
    });
    
    
    router.addRoute("POST", "/api/v1/zones/{name}/start", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (name.has_value() && AppContext::get<ZoneModel>()->startZone(name.value())) {
            res.text("Zone started");
        } else {
            res.status(404).text("Zone not found");
        }
    });
    
    router.addRoute("POST", "/api/v1/zones/{name}/stop", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (name.has_value() && AppContext::get<ZoneModel>()->stopZone(name.value())) {
            res.text("Zone stopped");
        } else {
            res.status(404).text("Zone not found");
        }
    });
    
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
