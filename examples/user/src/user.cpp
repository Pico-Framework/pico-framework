#include "UserModel.h"

UserModel::UserModel(StorageManager* storage)
    : FrameworkModel(storage, "/users.json") {}

std::string UserModel::getUsername(const std::string& id) const {
    auto user = find(id);
    if (user && (*user).contains("name")) {
        return (*user)["name"];
    }
    return "";
}

void UserModel::setUsername(const std::string& id, const std::string& newName) {
    auto user = find(id);
    if (user) {
        nlohmann::json updated = *user;
        updated["name"] = newName;
        update(id, updated);
    }
}
