// DashboardController.cpp
#include "DashboardController.h"

#include <pico/cyw43_arch.h>
#include <hardware/adc.h>
#include <cstdio>

#include "framework/AppContext.h"
#include "storage/StorageManager.h"
#include "PicoModel.h"

DashboardController::DashboardController(Router &r, PicoModel &pico)
    : FrameworkController("DashboardController", r, 1024, 1), pico(pico) {}

void DashboardController::initRoutes()
{

    printf("[DashboardController] Initializing routes...\n");

    // Serve embedded upload HTML - you can also use a static file
    router.addRoute("GET", "/upload", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
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
    router.addRoute("GET", "/api/v1/temperature", [this](auto &req, auto &res, const auto &match)
                    { getTemperature(req, res, match); });

    router.addRoute("GET", "/api/v1/led", [this](auto &req, auto &res, const auto &match)
                    { getLedState(req, res, match); });

    router.addRoute("POST", "/api/v1/led/{value}", [this](auto &req, auto &res, const auto &match)
                    { setLedState(req, res, match); });

    router.addRoute("POST", "/api/v1/upload", [this](auto &req, auto &res, const auto &)
                    { uploadHandler(req, res, {}); });

    router.addRoute("DELETE", "/uploads/{file}", [this](auto &req, auto &res, const auto &match)
                    { deleteFile(req, res, match); });

    router.addRoute("GET", "/", [](auto &req, auto &res, const auto &)
                    { res.sendFile("/uploads/pico_gpios.html"); });

    router.addRoute("GET", "/api/v1/ls(.*)", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    { this->router.listDirectory(req, res, match); });

    // Catch-all route for static files
    router.addRoute("GET", "/(.*)", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    { this->router.serveStatic(req, res, match); });
}

void DashboardController::getTemperature(HttpRequest &req, HttpResponse &res, const RouteMatch &)
{   
    float tempC = pico.getTemperature(); // Get temperature from pico model
    res.json({{"temperature", tempC}});
}

void DashboardController::getLedState(HttpRequest &req, HttpResponse &res, const RouteMatch &)
{
    bool isOn = pico.getLedState(); // Get LED state from pico model
    res.json({{"state", isOn ? 1 : 0}});
}

void DashboardController::setLedState(HttpRequest &req, HttpResponse &res, const RouteMatch &match)
{
    int value = std::stoi(match.getParam("value").value_or("0"));
    pico.setLedState(value != 0); // Convert to boolean
    res.json({{"state", value}});
}

void DashboardController::uploadHandler(HttpRequest &req, HttpResponse &res, const RouteMatch &match)
{
    req.handle_multipart(res);
}

void DashboardController::deleteFile(HttpRequest &req, HttpResponse &res, const RouteMatch &match)
{
    auto *fs = AppContext::get<StorageManager>();
    if (!fs->isMounted() && !fs->mount())
    {
        res.sendError(500, "mount_failed", "Failed to mount filesystem");
        return;
    }

    std::string path = "/uploads/" + match.getParam("file").value_or("");
    if (fs->exists(path))
    {
        fs->remove(path);
        res.sendSuccess({{"file", match.getParam("file")}}, "File deleted");
    }
    else
    {
        res.sendError(404, "File not found");
    }
}
