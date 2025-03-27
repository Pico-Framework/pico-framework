#include "json_request_helper.h"
#include "request.h"  // Your Request class header

using json = nlohmann::json;

json JsonRequestHelper::getFullJson(const Request& req) {
    if (!req.isJson()) return json::object();
    try {
        return json::parse(req.getBody());
    } catch (...) {
        return json::object();
    }
}

json JsonRequestHelper::getJsonValue(const Request& req, const std::string& path) {
    json j = getFullJson(req);
    if (j.is_null()) return nullptr;

    json current = j;
    size_t pos = 0, next;
    while ((next = path.find('.', pos)) != std::string::npos) {
        std::string part = path.substr(pos, next - pos);
        if (!current.contains(part)) return nullptr;
        current = current[part];
        pos = next + 1;
    }

    std::string last = path.substr(pos);
    return current.contains(last) ? current[last] : nullptr;
}

bool JsonRequestHelper::hasField(const Request& req, const std::string& key) {
    return !getJsonValue(req, key).is_null();
}

std::string JsonRequestHelper::getString(const Request& req, const std::string& key) {
    auto val = getJsonValue(req, key);
    if (val.is_string()) return val.get<std::string>();
    if (!val.is_null()) return val.dump();
    return "";
}

int JsonRequestHelper::getInt(const Request& req, const std::string& key, int def) {
    auto val = getJsonValue(req, key);
    return val.is_number_integer() ? val.get<int>() : def;
}

double JsonRequestHelper::getDouble(const Request& req, const std::string& key, double def) {
    auto val = getJsonValue(req, key);
    return val.is_number() ? val.get<double>() : def;
}

bool JsonRequestHelper::getBool(const Request& req, const std::string& key, bool def) {
    auto val = getJsonValue(req, key);
    return val.is_boolean() ? val.get<bool>() : def;
}

static const json JsonRequestHelper::getFullJson(const Request& req) {
    return parseJsonBody(req);
}

json JsonRequestHelper::getArray(const Request& req, const std::string& key) {
    auto val = getJsonValue(req, key);
    return val.is_array() ? val : json::array();
}

json JsonRequestHelper::getObject(const Request& req, const std::string& key) {
    auto val = getJsonValue(req, key);
    return val.is_object() ? val : json::object();
}


// void SettingsController::handle(Request& req, Response& res) {
//     std::string name = JsonRequestHelper::getString(req, "device.name");
//     int retryCount = JsonRequestHelper::getInt(req, "network.retry", 5);
//     bool enabled = JsonRequestHelper::getBool(req, "features.enabled");

//     if (JsonRequestHelper::hasField(req, "mode")) {
//         std::string mode = JsonRequestHelper::getString(req, "mode");
//         // ...
//     }

//     // Full object
//     auto settings = JsonRequestHelper::getFullJson(req);
// }



// #include "json_request_helper.h"

// void DeviceController::update(Request& req, Response& res) {
//     std::string name = JsonRequestHelper::getString(req, "device.name");
//     int port = JsonRequestHelper::getInt(req, "network.port", 8080);
//     bool logging = JsonRequestHelper::getBool(req, "debug.logging");
// }


// auto users = JsonRequestHelper::getArray(req, "data.users");
// for (const auto& user : users) {
//     std::string name = user["name"];
//     int id = user["id"];
// }

// auto settings = JsonRequestHelper::getObject(req, "config.network");
// std::string ip = settings.value("ip", "192.168.0.1");
// int port = settings.value("port", 80);
