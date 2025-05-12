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
};
