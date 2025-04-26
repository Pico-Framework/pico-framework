#ifndef APP_H
#define APP_H

#include "http/HttpRequest.h"

#include "framework/FrameworkApp.h"

class App : public FrameworkApp {
public:
    App(int port);  // Constructor to initialize the app with a specific port

    void initRoutes() override;
    void onStart() override;

    bool getLocationFromIp(std::string &tzName, double &lat, double &lon);
    void handleWeatherRequest(HttpRequest &req, HttpResponse &res);
    bool fetchWeatherSnapshot(nlohmann::json &weatherOut, double lat, double lon);

 private:   
    float latitude = 37.7749f; // Default example (San Francisco)
    float longitude = -122.4194f;
    std::string location = "Unknown";
    

};

#endif
