#pragma once

#include "FrameworkController.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <vector>
#include <string>

class GpioController : public FrameworkController {
public:
    GpioController(Router& r);
    void initRoutes() override;

private:
    void getState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
    void setState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params);
};
