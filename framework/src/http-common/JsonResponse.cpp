/**
 * @file HttpParser.cpp
 * @author Ian Archbell 
 * @brief HTTP parser for status codes, headers, and body handling. 
 * * Part of the PicoFramework HTTP server.
 * This module provides methods to parse HTTP status lines, headers, and
 * handle HTTP body content, including chunked transfer encoding and
 * content-length handling.
 * It is designed for use in embedded systems with FreeRTOS and lwIP.
 * @version 0.1
 * @date 2025-03-26 
 *  
 * @license MIT License
 *  
 * @copyright Copyright (c) 2025, Ian Archbell 
 */

#include "http/JsonResponse.h"
#include "http/HttpResponse.h"

namespace JsonResponse {

void sendSuccess(HttpResponse& res, const nlohmann::json& data, const std::string& message) {
    nlohmann::json j = { {"success", true} };
    if (!data.is_null() && !data.empty()) j["data"] = data;
    if (!message.empty()) j["message"] = message;
    res.status(200).json(j);
}

void sendCreated(HttpResponse& res, const nlohmann::json& data, const std::string& message) {
    nlohmann::json j = { {"success", true} };
    if (!data.is_null() && !data.empty()) j["data"] = data;
    if (!message.empty()) j["message"] = message;
    res.status(201).json(j);
}

void sendMessage(HttpResponse& res, const std::string& message) {
    res.status(200).json({
        {"success", true},
        {"message", message}
    });
}

void sendNoContent(HttpResponse& res) {
    res.status(204).send("");
}

void sendJson(HttpResponse& res, const nlohmann::json& raw, int statusCode) {
    res.status(statusCode).json(raw);
}

void sendError(HttpResponse& res, int statusCode, const std::string& code, const std::string& message) {
    res.status(statusCode).json({
        {"success", false},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    });
}

} // namespace JsonResponse

HttpResponse& HttpResponse::sendSuccess(const nlohmann::json& data, const std::string& message) {
    JsonResponse::sendSuccess(*this, data, message);
    return *this;
}

HttpResponse& HttpResponse::sendCreated(const nlohmann::json& data, const std::string& message) {
    JsonResponse::sendCreated(*this, data, message);
    return *this;
}

HttpResponse& HttpResponse::sendMessage(const std::string& message) {
    JsonResponse::sendMessage(*this, message);
    return *this;
}

HttpResponse& HttpResponse::sendNoContent() {
    JsonResponse::sendNoContent(*this);
    return *this;
}

HttpResponse& HttpResponse::sendError(int statusCode, const std::string& code, const std::string& message) {
    JsonResponse::sendError(*this, statusCode, code, message);
    return *this;
}