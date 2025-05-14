#pragma once

#include "framework/FrameworkController.h"
#include "utility/Logger.h"
#include "UserNotification.h"

/**
 * @brief Logs high-level program and zone events to persistent storage.
 */
class LogController : public FrameworkController {
public:
    LogController(Router& router);

protected:
    void onStart() override;
    void onEvent(const Event& event) override;
    void initRoutes() override;

    /**
     * @brief Handle the /api/v1/logs/summary route.
     *
     * @param req The incoming HTTP request.
     * @param res The HTTP response to send.
     * 
     * Has to be static to be used as a route handler unless using lambda
     * 
     */
    static void handleSummary(HttpRequest& req, HttpResponse& res);
    static void handleSummaryJson(HttpRequest& req, HttpResponse& res);

};
