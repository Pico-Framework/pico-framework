#include "GpioController.h"
#include "hardware/gpio.h"
#include <cstdio>

GpioController::GpioController(Router& r)
    : FrameworkController("GpioController", r, 1024, 1) {}

void GpioController::initRoutes() {

    printf("[GpioController] Initializing GPIO routes...\n");

    router.addRoute("GET", "/api/v1/gpio/{pin}", [this](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
        getState(req, res, match.ordered);
    });
    
    router.addRoute("POST", "/api/v1/gpio/{pin}/{value}", [this](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
        setState(req, res, match.ordered);
    });
}

void GpioController::getState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params) {
    int pin = std::stoi(params[0]);
    gpio_init(pin);
    bool state = gpio_get(pin);
    res.json({{"pin", pin}, {"state", state ? 1 : 0}});
}

void GpioController::setState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params) {
    int pin = std::stoi(params[0]);
    int value = std::stoi(params[1]);
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, value);
    res.json({{"pin", pin}, {"state", value}});
}

