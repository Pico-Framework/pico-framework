/**
 * @file FrameworkApp.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "FrameworkApp.h"

FrameworkApp::FrameworkApp(const char* name, uint16_t stackSize, UBaseType_t priority)
    : FrameworkTask(name, stackSize, priority) {}