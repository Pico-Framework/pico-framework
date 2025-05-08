#include "PingPongController.h"
#include "framework/AppContext.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "events/TimerService.h"
#include "utility/Logger.h"
#include "time/PicoTime.h"

PingPongController::PingPongController(const char* name, Router& router, const std::string& peerHostname, const std::string& startPath)
    : FrameworkController(name, router, 2048, tskIDLE_PRIORITY + 1),
      peerHost("http://" + peerHostname),
      nextPath(startPath) {}

void PingPongController::onStart() {
    configASSERT(AppContext::has<StorageManager>());
    AppContext::get<StorageManager>()->mount();
    scheduleNext();
}

void PingPongController::initRoutes() {
    printf("[PingPongController] Initializing routes...\n");
    router.addRoute("GET", "/ping", [this](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
        handlePing(req, res);
    });

    router.addRoute("GET", "/pong", [this](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
        handlePong(req, res);
    });

    router.addRoute("GET", "/log", [this](HttpRequest& req, HttpResponse& res, const RouteMatch& match){
        handleLog(req, res);
    });

    router.addRoute("GET", "/config", [this](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
        handleConfig(req, res);
    });

    router.addRoute("GET", "/", [](HttpRequest &req, HttpResponse &res, const auto &) {
        res.sendFile("/pingpong.html");
    });
}

void PingPongController::handlePing(HttpRequest& req, HttpResponse& res) {
    AppContext::get<Logger>()->info("Received /ping from peer");
    nextPath = "/pong";
    scheduleNext();
    res.send("OK");
}

void PingPongController::handlePong(HttpRequest& req, HttpResponse& res) {
    AppContext::get<Logger>()->info("Received /pong from peer");
    nextPath = "/ping";
    scheduleNext();
    res.send("OK");
}

void PingPongController::handleLog(HttpRequest& req, HttpResponse& res) {
    res.sendFile("log.txt");
}

void PingPongController::handleConfig(HttpRequest& req, HttpResponse& res) {
    res.json({
        {"peerHost", peerHost},
        {"nextPath", nextPath},
        {"intervalMs", intervalMs}
    });
}

void PingPongController::sendMessage() {
    HttpResponse res = 
        HttpRequest().setMethod("GET")
        .setUri(peerHost + nextPath)
        .send();
    if (res.ok()) {
        AppContext::get<Logger>()->info(("Sent " + nextPath + " to " + peerHost).c_str());
    } else {
        AppContext::get<Logger>()->warn(("Failed to send " + nextPath + ": HTTP " + std::to_string(res.getStatusCode())).c_str());
    }
}

void PingPongController::scheduleNext() {
    Event event;
    event.notification.kind = NotificationKind::User;
    event.notification.user_code = static_cast<uintptr_t>(UserNotification::SendNext);  // Define your enum
    configASSERT(AppContext::has<TimerService>());
    printf("PicoTime::now() %d\n", PicoTime::now());
    printf("PicoTime::now() + %d: %d\n", intervalMs, PicoTime::now() + intervalMs);
    AppContext::get<TimerService>()->scheduleAt(PicoTime::now() + intervalMs, event);
}

void PingPongController::onEvent(const Event& evt) {
    if (evt.notification.kind == NotificationKind::User &&
        evt.notification.user_code == static_cast<uintptr_t>(UserNotification::SendNext)) {
        sendMessage();
    }
}

