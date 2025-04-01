/**
 * @file JsonRequestHelper.cpp
 * @author Ian Archbell
 * @brief Implementation of helper functions to extract fields from JSON request bodies.
 * 
 * Part of the PicoFramework HTTP server.
 * This module provides functions to retrieve various data types (string, int, double, bool)
 * from JSON request bodies. It also includes methods to check for the presence of fields
 * and retrieve JSON arrays or objects.
 * 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
 #include "DebugTrace.h" 
 TRACE_INIT(JsonRequestHelper)

 #include "JsonRequestHelper.h"
 #include "HttpRequest.h"
 
 using json = nlohmann::json;
 
 /// @copydoc JsonRequestHelper::getFullJson
 json JsonRequestHelper::getFullJson(const Request& req)
 {
     if (!req.isJson()) return json::object();
     return json::parse(req.getBody(), nullptr, false);
 }
 
 /// @copydoc JsonRequestHelper::getJsonValue
 json JsonRequestHelper::getJsonValue(const Request& req, const std::string& path)
 {
     json current = getFullJson(req);
     if (!current.is_object()) return nullptr;
 
     size_t pos = 0, next;
     while ((next = path.find('.', pos)) != std::string::npos)
     {
         std::string part = path.substr(pos, next - pos);
         if (!current.contains(part) || !current[part].is_object()) return nullptr;
         current = current[part];
         pos = next + 1;
     }
 
     std::string last = path.substr(pos);
     return current.contains(last) ? current[last] : nullptr;
 }
 
 /// @copydoc JsonRequestHelper::hasField
 bool JsonRequestHelper::hasField(const Request& req, const std::string& key)
 {
     return !getJsonValue(req, key).is_null();
 }
 
 /// @copydoc JsonRequestHelper::getString
 std::string JsonRequestHelper::getString(const Request& req, const std::string& key)
 {
     auto val = getJsonValue(req, key);
     if (val.is_string()) return val.get<std::string>();
     if (!val.is_null()) return val.dump();
     return "";
 }
 
 /// @copydoc JsonRequestHelper::getInt
 int JsonRequestHelper::getInt(const Request& req, const std::string& key, int def)
 {
     auto val = getJsonValue(req, key);
     return val.is_number_integer() ? val.get<int>() : def;
 }
 
 /// @copydoc JsonRequestHelper::getDouble
 double JsonRequestHelper::getDouble(const Request& req, const std::string& key, double def)
 {
     auto val = getJsonValue(req, key);
     return val.is_number() ? val.get<double>() : def;
 }
 
 /// @copydoc JsonRequestHelper::getBool
 bool JsonRequestHelper::getBool(const Request& req, const std::string& key, bool def)
 {
     auto val = getJsonValue(req, key);
     return val.is_boolean() ? val.get<bool>() : def;
 }
 
 /// @copydoc JsonRequestHelper::getArray
 json JsonRequestHelper::getArray(const Request& req, const std::string& key)
 {
     auto val = getJsonValue(req, key);
     return val.is_array() ? val : json::array();
 }
 
 /// @copydoc JsonRequestHelper::getObject
 json JsonRequestHelper::getObject(const Request& req, const std::string& key)
 {
     auto val = getJsonValue(req, key);
     return val.is_object() ? val : json::object();
 }
 