
/**
* @file app.cpp
* @author Ian Archbell
* @brief Minimal PicoFramework application showcasing routing and request/response usage.
* @version 0.1
* @date 2025-05-19
* @license MIT
* @copyright Copyright (c) 2025, Ian Archbell
*
* This is the simplest route example, with the minimum of comments so you can see the structure clearly.
* See the hello_framework example for a heavily commented example that focuses solely on routing.
*/

#include "app.h"
#include <iostream>
#include "events/Notification.h"
#include "events/EventManager.h"
#include "http/Router.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "framework/AppContext.h"
#include "storage/StorageManager.h"
#include "nlohmann/json.hpp"

char BROWSER_ROOT_CA[] = "-----BEGIN CERTIFICATE-----\n\
MIIC+jCCAn+gAwIBAgICEAAwCgYIKoZIzj0EAwIwgbcxCzAJBgNVBAYTAkdCMRAw\n\
DgYDVQQIDAdFbmdsYW5kMRIwEAYDVQQHDAlDYW1icmlkZ2UxHTAbBgNVBAoMFFJh\n\
c3BiZXJyeSBQSSBMaW1pdGVkMRwwGgYDVQQLDBNSYXNwYmVycnkgUEkgRUNDIENB\n\
MR0wGwYDVQQDDBRSYXNwYmVycnkgUEkgUm9vdCBDQTEmMCQGCSqGSIb3DQEJARYX\n\
c3VwcG9ydEByYXNwYmVycnlwaS5jb20wIBcNMjExMjA5MTEzMjU1WhgPMjA3MTEx\n\
MjcxMTMyNTVaMIGrMQswCQYDVQQGEwJHQjEQMA4GA1UECAwHRW5nbGFuZDEdMBsG\n\
A1UECgwUUmFzcGJlcnJ5IFBJIExpbWl0ZWQxHDAaBgNVBAsME1Jhc3BiZXJyeSBQ\n\
SSBFQ0MgQ0ExJTAjBgNVBAMMHFJhc3BiZXJyeSBQSSBJbnRlcm1lZGlhdGUgQ0Ex\n\
JjAkBgkqhkiG9w0BCQEWF3N1cHBvcnRAcmFzcGJlcnJ5cGkuY29tMHYwEAYHKoZI\n\
zj0CAQYFK4EEACIDYgAEcN9K6Cpv+od3w6yKOnec4EbyHCBzF+X2ldjorc0b2Pq0\n\
N+ZvyFHkhFZSgk2qvemsVEWIoPz+K4JSCpgPstz1fEV6WzgjYKfYI71ghELl5TeC\n\
byoPY+ee3VZwF1PTy0cco2YwZDAdBgNVHQ4EFgQUJ6YzIqFh4rhQEbmCnEbWmHEo\n\
XAUwHwYDVR0jBBgwFoAUIIAVCSiDPXut23NK39LGIyAA7NAwEgYDVR0TAQH/BAgw\n\
BgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwCgYIKoZIzj0EAwIDaQAwZgIxAJYM+wIM\n\
PC3wSPqJ1byJKA6D+ZyjKR1aORbiDQVEpDNWRKiQ5QapLg8wbcED0MrRKQIxAKUT\n\
v8TJkb/8jC/oBVTmczKlPMkciN+uiaZSXahgYKyYhvKTatCTZb+geSIhc0w/2w==\n\
-----END CERTIFICATE-----\n\0";

App::App(int port) : FrameworkApp(port, "AppTask", 2048, 1)
{
    std::cout << "App constructed" << std::endl;
}

/**
 * @brief Initializes the HTTP routes for the application.
 * This method sets up the routing table for the HTTP server.   
 */
void App::initRoutes()
{
    router.addRoute("GET", "/", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        req.printHeaders();
        res.send("Hello from Ian Archbell!");
    });

    router.addRoute("GET", "/ls(.*)", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        std::string path = "/"; // default root
        if (!match.ordered.empty() && !match.ordered[0].empty())
        {
            path = match.ordered[0];
        }

        auto files = listFiles(path);

        if (!files.empty() && files[0].contains("error")) {
            res.sendError(404, files[0]["error"]);
            printf("[StorageController] Failed to list directory: %s\n", path.c_str());
            return;
        }

        res.json(files);

        printf("[StorageController] Listed files in path '%s'.\n", path.c_str());
        printf("[StorageController] Files: %s\n", files.dump().c_str());
    } );

    router.addRoute("GET", "/tls", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        HttpResponse response = HttpRequest().setRootCACertificate(BROWSER_ROOT_CA)
            .get("https://fw-download-alias1.raspberrypi.com/net_install/boot.sig");
        if (!response.ok()) {
            // Unsuccesful response
            printf("Unsuccesful response\n");
            res.status(response.getStatusCode()).json({ { "Response", response.getBody() } });
        }
        else{
            printf("TLS request sent successfully\n");
            res.status(200).json({{ "Response", response.getBody()}});
            printf("RPi Response: %s\n", response.getBody().c_str());
        }
    });

    router.addRoute("GET", "/tlstofile/{path}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        StorageManager* storage = AppContext::get<StorageManager>();
        HttpResponse response = HttpRequest().setRootCACertificate(BROWSER_ROOT_CA)
            .get("https://fw-download-alias1.raspberrypi.com/net_install/boot.sig").toFile(match.getParam("path").value_or("respons.txt"), storage);
        if (!response.ok()) {
            // Unsuccesful response
            printf("Unsuccesful response\n");
        }
        else{
            printf("TLS request sent successfully\n");
            res.status(200).sendFile("response.txt");
            printf("RPi Response: %s\n", response.getBody().c_str());
        }
    });

    // Add more routes as needed
}

/**
 * @brief This method is called by the framework when the application starts.
 * At this point the FreeRTOS task is running and the application is ready to process events.
 * It is a good place to initialize resources, start services, etc.
 */
void App::onStart()
{
    std::cout << "[App] Waiting for network..." << std::endl;

    AppContext::getInstance().get<EventManager>()->subscribe(eventMask(SystemNotification::NetworkReady), this);

    waitFor(SystemNotification::NetworkReady);

    std::cout << "[App] Network ready. Building routing table..." << std::endl;

    server.start();
}

/**
 * @brief This method is called when an event is received.
 */
void App::onEvent(Event &e)
{
    if (e.notification.kind == NotificationKind::System)
    {
        switch (e.notification.system)
        {

        case SystemNotification::NetworkReady:
            // Start the HTTP server here in non-blocking mode
            break;

        case SystemNotification::TimeValid:
            std::cout << "[App] Time is valid. Your scheduler, if using one, can be initialized here." << std::endl;
            // If you rely on calendar time, you can start your scheduler, server ro anything else that relies on time
            break;

        default:
            // event we are not handling
            break;
        }
    }
}

/**
 * @brief Called periodically by the framework. No polling tasks are defined in this example.
 */
void App::poll()
{
    // For example, you can check the status of sensors, etc.
    runEvery(15000, []()
    {
        // This will execute once every 15000ms
        std::cout << "[App] Polling..." << std::endl;
    }, "PollingTask");
}

nlohmann::json App::listFiles(const std::string& path) {
    nlohmann::json result = nlohmann::json::array(); // <-- ARRAY not object

    auto storage = AppContext::get<StorageManager>();
    if (!storage) {
        result.push_back({{"error", "StorageManager not available"}});
        return result;
    }

    std::vector<FileInfo> files;
    if (storage->listDirectory(path, files)) {
        for (const auto& file : files) {
            result.push_back({
                {"name", file.name},
                {"size", file.size}
            });
        }
    } else {
        result.push_back({{"error", "Failed to list directory"}});
    }

    return result;
}
