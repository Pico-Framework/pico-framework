#ifndef APP_H
#define APP_H

#include "FrameworkApp.h"

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

private:
};

#endif