/**
 * @file PingPongController.h
 * @brief Controller for ping-pong demo using hostnames over HTTP.
 * @version 0.2
 * @date 2025-05-06
 * @license MIT License
 */

#pragma once

#include "framework/FrameworkController.h"
#include "http/HttpClient.h"
#include "utility/Logger.h"
#include "events/TimerService.h"

#pragma once
#include "framework/FrameworkController.h"
#include <string>

enum class UserNotification : uintptr_t {
    SendNext = 1
};
class PingPongController : public FrameworkController
{
public:
    PingPongController(const char *name, Router &router, const std::string &peerHostname, const std::string &startPath);

    void onStart() override;
    void initRoutes() override;
    void onEvent(const Event &evt);

private:
    std::string peerHost;
    std::string nextPath;

    void handlePing(HttpRequest & req, HttpResponse & res);
    void handlePong(HttpRequest & req, HttpResponse & res);
    void handleLog(HttpRequest & req, HttpResponse & res);
    void handleConfig(HttpRequest & req, HttpResponse & res);

    void sendMessage();
    void scheduleNext();

    uint32_t intervalMs = 30000; // 30 seconds between messages
};
