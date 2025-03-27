#pragma once

#include <string>
#include <vector>
#include <optional>
#include "JsonService.h"
#include "nlohmann/json.hpp"

class FrameworkModel {
public:
    FrameworkModel(StorageManager* storage, const std::string& path);

    bool load();
    bool save();

    std::vector<nlohmann::json> all() const;
    std::optional<nlohmann::json> find(const std::string& id) const;

    bool create(const nlohmann::json& item);
    bool update(const std::string& id, const nlohmann::json& updatedItem);
    bool remove(const std::string& id);

protected:
    virtual std::string getIdField() const { return "id"; }

    nlohmann::json collection = nlohmann::json::array();

private:
    JsonService jsonService;
    std::string storagePath;
};
