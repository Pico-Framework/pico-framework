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
#include "http/RouteTypes.h"
#include "PicoModel.h"

class DashboardController : public FrameworkController {
public:
    DashboardController(Router& r, PicoModel &pico);
    void initRoutes() override;

private:
    void getTemperature(HttpRequest& req, HttpResponse& res, const RouteMatch & match);
    void getLedState(HttpRequest& req, HttpResponse& res, const RouteMatch & match);
    void setLedState(HttpRequest& req, HttpResponse& res, const RouteMatch & match);
    void uploadHandler(HttpRequest& req, HttpResponse& res, const RouteMatch & match);
    void deleteFile(HttpRequest& req, HttpResponse& res, const RouteMatch & match);
    
    PicoModel &pico; // Reference to the PicoModel for dashboard state management
};
