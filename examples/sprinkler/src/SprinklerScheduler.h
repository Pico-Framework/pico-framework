/**
 * @file SprinklerScheduler.h
 * @author Ian Archbell
 * @brief Periodically schedules zone activations based on ProgramModel.
 * @version 0.1
 * @date 2025-05-09
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#pragma once

#include "framework/FrameworkController.h"
#include "ProgramModel.h"
#include "time/PicoTime.h"
#include "events/Event.h"

/**
 * @brief Background task that scans the program list and triggers events for active programs.
 */
class SprinklerScheduler : public FrameworkController
{
public:
    /**
     * @brief Construct the scheduler with a reference to the program model.
     * @param model ProgramModel reference
     */
    SprinklerScheduler(Router &router, ProgramModel &model, uint16_t stackSize = 1024, UBaseType_t priority = tskIDLE_PRIORITY + 2)
        : FrameworkController("SprinklerScheduler", router, stackSize, priority), programModel(&model) {};

    void initRoutes() override;
    void onStart() override;
    void poll() override;

    void setProgramModel(ProgramModel *pm)
    {
        this->programModel = pm;
    }

private:
    ProgramModel* programModel = nullptr;
    uint32_t lastCheckMinute = 0;
    std::string runningProgramName;
    int pendingZones = 0;


    /**
     * @brief Check all programs and activate those scheduled to start now.
     */
    void checkPrograms();

    /**
     * @brief Trigger events for the given program and its zones.
     * @param program Program to activate
     */
    void activateProgram(const SprinklerProgram &program);

    void onEvent(const Event &e) override;

    void checkSchedule();
};
