// DashboardController.cpp
#include "DashboardController.h"

#include <pico/cyw43_arch.h>
#include <hardware/adc.h>
#include <cstdio>

#include "framework/AppContext.h"
#include "storage/StorageManager.h"

DashboardController::DashboardController(Router &r)
    : FrameworkController("DashboardController", r, 1024, 1) {}

void DashboardController::initRoutes()
{

    printf("[DashboardController] Initializing routes...\n");

    // Serve embedded upload HTML - you can also use a static file
    router.addRoute("GET", "/upload", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    {
        static const char* uploadHtml = R"rawliteral(
        <!DOCTYPE html>
        <html lang="en">
        <head>
        <meta charset="UTF-8">
        <title>Upload</title>
        </head>
        <body>
        <h1>Upload a File</h1>
        <form method="POST" action="/api/v1/upload" enctype="multipart/form-data">
        <input type="file" name="file" />
        <button type="submit">Upload</button>
        </form>
        <p>Try opening the file at <code>/uploads/filename.jpg</code> after uploading.</p>
        </body>
        </html>
        )rawliteral";

        res.setContentType("text/html");
        res.send(uploadHtml); });
    router.addRoute("GET", "/api/v1/temperature", [this](auto &req, auto &res, const auto &params)
                    { getTemperature(req, res, params); });

    router.addRoute("GET", "/api/v1/led", [this](auto &req, auto &res, const auto &params)
                    { getLedState(req, res, params); });

    router.addRoute("POST", "/api/v1/led/{value}", [this](auto &req, auto &res, const auto &params)
                    { setLedState(req, res, params); });

    router.addRoute("POST", "/api/v1/upload", [this](auto &req, auto &res, const auto &)
                    { uploadHandler(req, res, {}); });

    router.addRoute("DELETE", "/uploads/{file}", [this](auto &req, auto &res, const auto &params)
                    { deleteFile(req, res, params); });

    router.addRoute("GET", "/", [](auto &req, auto &res, const auto &)
                    { res.sendFile("/uploads/pico_gpios.html"); });

    // Catch-all route for static files
    router.addRoute("GET", "/(.*)", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->router.serveStatic(req, res, params); });
}

void DashboardController::getTemperature(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &)
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    float voltage = adc_read() * (3.3f / (1 << 12));
    float tempC = 27.0f - (voltage - 0.706f) / 0.001721f;
    res.json({{"temperature", tempC}});
}

void DashboardController::getLedState(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &)
{
    bool isOn = cyw43_arch_gpio_get(0);
    res.json({{"state", isOn ? 1 : 0}});
}

void DashboardController::setLedState(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    int value = std::stoi(params[0]);
    cyw43_arch_gpio_put(0, value ? 1 : 0);
    res.json({{"state", value}});
}

void DashboardController::uploadHandler(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &)
{
    req.handle_multipart(res);
}

void DashboardController::deleteFile(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    auto *fs = AppContext::get<StorageManager>();
    if (!fs->isMounted() && !fs->mount())
    {
        res.sendError(500, "mount_failed", "Failed to mount filesystem");
        return;
    }

    std::string path = "/uploads/" + params[0];
    if (fs->exists(path))
    {
        fs->remove(path);
        res.sendSuccess({{"file", params[0]}}, "File deleted");
    }
    else
    {
        res.sendError(404, "File not found");
    }
}
