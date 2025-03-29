#pragma once

#include <string>
#include <vector>
#include "StorageManager.h"
#include "nlohmann/json.hpp"

class JsonService {
public:
    JsonService(StorageManager* storage);

    bool load(const std::string& path);
    bool save(const std::string& path) const;

    nlohmann::json& data();                  // direct access
    const nlohmann::json& data() const;

    nlohmann::json& root();                  // alias for data()
    const nlohmann::json& root() const;

    nlohmann::json& operator*();             // alias for data()
    const nlohmann::json& operator*() const;

private:
    StorageManager* storage;
    nlohmann::json data_;
};

// Optional: keep this helper function in a header or move to its own file
nlohmann::json mergeDefaults(const nlohmann::json& target, const nlohmann::json& defaults);