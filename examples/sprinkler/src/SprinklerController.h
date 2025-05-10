/**
 * @file SprinklerController.h
 * @author Ian Archbell
 * @brief HTTP controller for managing sprinkler programs.
 * @version 0.1
 * @date 2025-05-09
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#pragma once

#include "framework/FrameworkController.h"
#include "ZoneModel.h"
#include "UserNotification.h"


/**
 * @brief Controller for managing sprinkler programs via REST endpoints.
 */
class SprinklerController : public FrameworkController
{
public:
    explicit SprinklerController(Router &router, ZoneModel &zm, uint16_t stackSize = 1024, UBaseType_t priority = tskIDLE_PRIORITY + 1)
        : FrameworkController("SprinklerCtrl", router, stackSize, priority), zoneModel(&zm){};

    void initRoutes() override;

    void setZoneModel(ZoneModel *zm)
    {
        this->zoneModel = zm;
    }

    void onEvent(const Event &event) override;

private:
    ZoneModel* zoneModel = nullptr;
    
};
