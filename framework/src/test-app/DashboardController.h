// DashboardController.h
#pragma once
#include "FrameworkController.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <vector>
#include <string>

class DashboardController : public FrameworkController {
public:
    DashboardController(Router& r);
    void initRoutes() override;

private:
    void getTemperature(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
    void getLedState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
    void setLedState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
    void uploadHandler(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
    void deleteFile(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
};
