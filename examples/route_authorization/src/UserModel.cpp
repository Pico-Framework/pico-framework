#include "UserModel.h"

UserModel::UserModel()
    : FrameworkModel("users.json") {
    load(); // ensures `collection` is populated
}

bool UserModel::createUser(const std::string& username, const std::string& passwordHash) {
    if (find(username).has_value())
        return false;

    nlohmann::json user = {
        { "username", username },
        { "password", passwordHash }
    };
    return create(user) && save();
}

bool UserModel::verifyUser(const std::string& username, const std::string& passwordHash) {
    auto user = find(username);
    return user && (*user)["password"] == passwordHash;
}
