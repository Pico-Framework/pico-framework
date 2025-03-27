#pragma once

#include <string>
#include <nlohmann/json.hpp>

class Request;

class JsonRequestHelper {
public:
    static nlohmann::json getJsonValue(const Request& req, const std::string& path);
    static bool hasField(const Request& req, const std::string& key);
    static std::string getString(const Request& req, const std::string& key);
    static int getInt(const Request& req, const std::string& key, int def = 0);
    static double getDouble(const Request& req, const std::string& key, double def = 0.0);
    static bool getBool(const Request& req, const std::string& key, bool def = false);
    static nlohmann::json getFullJson(const Request& req);
    static nlohmann::json getArray(const Request& req, const std::string& key);
    static nlohmann::json getObject(const Request& req, const std::string& key);
};
