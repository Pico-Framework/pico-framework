#include "PingPongController.h"
#include "framework/AppContext.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "events/TimerService.h"
#include "utility/Logger.h"
#include "time/PicoTime.h"
#include "events/EventManager.h"
#include "events/Notification.h"
#include "events/Event.h"

PingPongController::PingPongController(const char *name, Router &router, const std::string &peerHostname, const std::string &startPath)
    : FrameworkController(name, router, 2048, tskIDLE_PRIORITY + 1),
      peerHost("http://" + peerHostname),
      host(name),
      nextPath(startPath) {}

void PingPongController::onStart()
{
    EventManager *eventManager = AppContext::get<EventManager>();
    eventManager->subscribe(eventMask(SystemNotification::NetworkReady), this);
    waitFor(SystemNotification::NetworkReady);

    configASSERT(AppContext::has<StorageManager>());
    AppContext::get<StorageManager>()->mount();
    configASSERT(AppContext::has<Logger>());
    AppContext::get<Logger>()->enableFileLogging("log.txt");
    AppContext::get<Logger>()->info("PingPongController started");

    if (std::string(host) == "ping-a")
    { // only ping-a starts things off
        printf("[PingPongController] Starting ping-pong demo...\n");
        AppContext::get<Logger>()->info("Delaying first ping to allow peer startup...");
        nextPath = "/ping";    // ensure we initiates
    }

    scheduleNext();
}

void PingPongController::initRoutes()
{
    printf("[PingPongController] Initializing routes...\n");

    router.use(corsMiddleware);

    router.addRoute("GET", "/ping", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    { handlePing(req, res); });

    router.addRoute("GET", "/pong", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    { handlePong(req, res); });

    router.addRoute("GET", "/log", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    { handleLog(req, res); });

    router.addRoute("GET", "/config", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    { handleConfig(req, res); });

    router.addRoute("GET", "/", [](HttpRequest &req, HttpResponse &res, const auto &)
                    { res.sendFile("/pingpong.html"); });
}

void PingPongController::handlePing(HttpRequest &req, HttpResponse &res)
{
    printf("Received /ping from peer\n");
    AppContext::get<Logger>()->info("Received /ping from peer");
    nextPath = "/pong";
    res.send("OK");
}

void PingPongController::handlePong(HttpRequest &req, HttpResponse &res)
{
    printf("Received /pong from peer\n");
    AppContext::get<Logger>()->info("Received /pong from peer");
    nextPath = "/ping";
    res.send("OK");
}

void PingPongController::handleLog(HttpRequest &req, HttpResponse &res)
{
    res.sendFile("log.txt");
}

void PingPongController::handleConfig(HttpRequest &req, HttpResponse &res)
{
    res.json({{"peerHost", peerHost},
              {"nextPath", nextPath},
              {"intervalMs", intervalMs}});
}

void PingPongController::sendMessage()
{
    printf("Sending %s to %s\n", nextPath.c_str(), peerHost.c_str());
    HttpResponse res =
        HttpRequest().setMethod("GET").setUri(peerHost + nextPath).send();
    if (res.ok())
    {
        AppContext::get<Logger>()->info(("Sent " + nextPath + " to " + peerHost).c_str());
    }
    else
    {
        AppContext::get<Logger>()->warn(("Failed to send " + nextPath + ": HTTP " + std::to_string(res.getStatusCode())).c_str());
    }
}

void PingPongController::scheduleNext()
{
    if (nextScheduled) return;  // already scheduled
    
    Event event;
    event.notification.kind = NotificationKind::User;
    event.notification.user_code = static_cast<uintptr_t>(UserNotification::SendNext); // Define your enum
    configASSERT(AppContext::has<TimerService>());
    printf("PicoTime::now() %d\n", PicoTime::now());
    printf("PicoTime::now() + %d: %d\n", intervalMs, PicoTime::now() + intervalMs);
    time_t target = PicoTime::now() + static_cast<time_t>(intervalMs / 1000);
    AppContext::get<TimerService>()->scheduleAt(target, event);
}

void PingPongController::onEvent(const Event &evt)
{
    if (evt.notification.kind == NotificationKind::User &&
        evt.notification.user_code == static_cast<uintptr_t>(UserNotification::SendNext))
    {
        nextScheduled = false;  // reset flag for next time
        sendMessage();
        scheduleNext(); 
    }
}

bool PingPongController::corsMiddleware(HttpRequest &req, HttpResponse &res, const RouteMatch &)
{
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");

    if (req.getMethod() == "OPTIONS") {
        res.setStatus(204).send("");  // Preflight handled
        return false;         // Stop middleware chain
    }

    return true;              // Continue to controller
}