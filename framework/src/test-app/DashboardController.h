/**
 * @file DashboardController.cpp
 * @brief DashboardController for handling dashboard-related HTTP requests.
 * @author Ian Archbell
 * @date 2025-04-22
 * @version 0.1
 * @copyright Copyright (c) 2025, Ian Archbell 
 * 
 */


// DashboardController.h


#pragma once
#include "framework/FrameworkController.h"

#include <vector>
#include <string>

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"


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
