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
 #include "time/TimerService.h"
 
 class PingPongController : public FrameworkController
 {
 public:
     explicit PingPongController(const std::string &peerHostname, const std::string &startPath);
     void initRoutes() override;
 
 private:
     void handlePing(HttpRequest &req, HttpResponse &res);
     void handlePong(HttpRequest &req, HttpResponse &res);
     void handleConfig(HttpRequest &req, HttpResponse &res);
     void handleLog(HttpRequest &req, HttpResponse &res);
 
     void sendMessage();  // Sends either "ping" or "pong"
     void scheduleNext();
 
     std::string peerHost;        // e.g. "http://ping-b"
     std::string nextPath;        // "/ping" or "/pong"
     uint32_t intervalMs = 5000;  // default interval
 
     Logger logger;
     TimerService timer;
 };
 