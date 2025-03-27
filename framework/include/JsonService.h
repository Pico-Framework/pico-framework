#pragma once

#include <string>
#include <vector>
#include "nlohmann/json.hpp"
#include "StorageManager.h"

class JsonService {
public:
    explicit JsonService(StorageManager* storage);

    bool load(const std::string& path);
    bool save(const std::string& path);

    nlohmann::json& data();
    const nlohmann::json& data() const;

private:
    StorageManager* storage;
    nlohmann::json jsonData;
};
