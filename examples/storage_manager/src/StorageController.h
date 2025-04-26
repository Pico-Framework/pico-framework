#pragma once

#include "framework/FrameworkController.h"
#include "FileStorage.h" // <-- Include this!

class StorageController : public FrameworkController {
public:
    StorageController(Router& router);
    void initRoutes() override;

private:
    FileStorage storage;  // <-- THIS LINE MUST BE THERE
};
