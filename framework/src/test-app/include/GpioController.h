/**
 * @fileGpioController.cpp
 * 
 * @brief GPIO controller for handling GPIO state retrieval and setting.
 * This controller provides HTTP endpoints to get and set GPIO states.
 * @version 0.1
 * @date 2025-04-14
 * @license MIT License
 * @copyright (c) 2025, Ian Archbell
 * 
 */

#pragma once

#include "framework/FrameworkController.h"

#include <vector>
#include <string>

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "PicoModel.h"

class GpioController : public FrameworkController {
public:
    GpioController(Router& r, PicoModel &pico);
    void initRoutes() override;

private:
    void getState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
    void setState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
    void handleGetMultipleGpios(HttpRequest& req, HttpResponse& res);
    PicoModel &pico; // Reference to the PicoModel for GPIO state management
};
