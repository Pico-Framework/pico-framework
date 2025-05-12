/**
 * @file SprinklerController.cpp
 * @brief Implementation of HTTP routes for sprinkler program control.
 */

#include "SprinklerController.h"
#include "ZoneModel.h"
#include "framework/AppContext.h"


void SprinklerController::initRoutes()
{
    printf("Router address: %p\n", &router);
    // This method is called by the base class to initialize HTTP routes.
    printf("Initializing routes for SprinklerController\n");
    router.addRoute("GET", "/api/v1/zones", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &) {
        res.json(zoneModel->getAllZones());  
    });
    
    router.addRoute("GET", "/api/v1/zones/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (name.has_value()) {
            const auto& all = zoneModel->getAllZones();
            for (const auto& zone : all) {
                if (zone.name == name.value()) {
                    res.json(zone);  // relies on NLOHMANN_DEFINE_TYPE_INTRUSIVE
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
    
        const auto& json = req.json();
    
        // Validate input types and presence
        if (!json.is_object() ||
            !json.contains("name") ||
            !json.contains("gpioPin") ||
            !json.contains("active")) {
            res.status(400).text("Missing required fields");
            return;
        }
    
        Zone zone;
        zone.name = json.value("name", "");
        zone.gpioPin = static_cast<uint8_t>(json.value("gpioPin", 255));
        zone.active = json.value("active", false);
    
        if (zoneModel->updateZone(name.value(), zone)) {
            res.text("Zone updated");
        } else {
            res.status(404).text("Zone not found");
        }
    });
    
    
    router.addRoute("POST", "/api/v1/zones/{name}/start", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (!name.has_value()) {
            res.status(400).text("Missing zone name");
            return;
        }
    
        if (zoneModel->startZone(name.value())) {
            res.text("Zone started");
        } else {
            res.status(404).text("Zone not found");
        }
    });
    
    router.addRoute("POST", "/api/v1/zones/{name}/stop", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        auto name = match.getParam("name");
        if (!name.has_value()) {
            res.status(400).text("Missing zone name");
            return;
        }
    
        if (zoneModel->stopZone(name.value())) {
            res.text("Zone stopped");
        } else {
            res.status(404).text("Zone not found");
        }
    });
    
    router.printRoutes();
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

void SprinklerController::onStart()
{
    // This method is called when the controller is started.
    // You can perform any initialization or setup here.
    printf("SprinklerController started\n");
}

void SprinklerController::poll()
{
    // This method is called periodically to perform any non-blocking tasks.
    // You can implement any background logic here.
    vTaskDelay(pdMS_TO_TICKS(10));
}
