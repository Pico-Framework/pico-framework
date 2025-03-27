#include "FrameworkModel.h"

FrameworkModel::FrameworkModel(StorageManager* storage, const std::string& path)
    : jsonService(storage), storagePath(path) {}

bool FrameworkModel::load() {
    if (!jsonService.load(storagePath)) return false;
    collection = jsonService.data().value("items", nlohmann::json::array());
    return true;
}

bool FrameworkModel::save() {
    jsonService.data()["items"] = collection;
    return jsonService.save(storagePath);
}

std::vector<nlohmann::json> FrameworkModel::all() const {
    std::vector<nlohmann::json> items;
    for (const auto& item : collection) {
        items.push_back(item);
    }
    return items;
}

std::optional<nlohmann::json> FrameworkModel::find(const std::string& id) const {
    std::string idField = getIdField();
    for (const auto& item : collection) {
        if (item.contains(idField) && item[idField] == id) {
            return item;
        }
    }
    return std::nullopt;
}

bool FrameworkModel::create(const nlohmann::json& item) {
    std::string idField = getIdField();
    if (!item.contains(idField)) return false;
    std::string id = item[idField];
    if (find(id)) return false;  // already exists
    collection.push_back(item);
    return true;
}

bool FrameworkModel::update(const std::string& id, const nlohmann::json& updatedItem) {
    std::string idField = getIdField();
    for (auto& item : collection) {
        if (item.contains(idField) && item[idField] == id) {
            item = updatedItem;
            return true;
        }
    }
    return false;
}

bool FrameworkModel::remove(const std::string& id) {
    std::string idField = getIdField();
    for (auto it = collection.begin(); it != collection.end(); ++it) {
        if ((*it).contains(idField) && (*it)[idField] == id) {
            collection.erase(it);
            return true;
        }
    }
    return false;
}
