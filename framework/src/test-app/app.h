/**
 * @file App.cpp
 * @brief Application class for the PicoFramework test application.
 * This class extends the FrameworkApp to provide application-specific functionality,
 * including route initialization, event handling, and test functions.
 * @version 0.1
 *  
 * @date 2025-04-14
 * 
 * @license MIT License
 * 
 * @copyright Copyright (c) 2025, Ian Archbell
 *  
 */

#ifndef APP_H
#define APP_H

#include "framework/FrameworkApp.h"

class App : public FrameworkApp
{
public:
    App(int port); // Constructor to initialize the application with a port number

    // standard functions
    void initRoutes() override;            // called by onStart() to initialize routes
    void onStart() override;               // called by the framework when the app is started
    void poll() override;                  // called by the framework to poll the app
    void onEvent(const Event &e) override; // called by the framework when an event occurs

    // test functions
    void ledOn(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params);
    void ledOff(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params);
    void getTemperature(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params);
    void deleteFile(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params);
    void getState(HttpRequest &req, HttpResponse &res, const std::vector<std::string> params);
    void setState(HttpRequest &req, HttpResponse &res, const std::vector<std::string> params);
    void getLedState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& /*params*/);

    enum class UserNotification : uint8_t { // user-defined notifications
        Heartbeat = 0
    };

private:
};

#endif