/**
 * @file JsonResponse.h
 * @author Ian Archbell
 * @brief Utility functions to send standard JSON responses using nlohmann::json
 * @version 0.1
 * @date 2025-04-04
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #pragma once

#include <string>
#include <nlohmann/json.hpp>

class HttpResponse;

namespace JsonResponse {

// Send helpers   
void sendSuccess(HttpResponse& res, const nlohmann::json& data = {}, const std::string& message = "");
void sendCreated(HttpResponse& res, const nlohmann::json& data = {}, const std::string& message = "");
void sendMessage(HttpResponse& res, const std::string& message);
void sendNoContent(HttpResponse& res);
void sendJson(HttpResponse& res, const nlohmann::json& raw, int statusCode = 200);

// Error helpers
void sendError(HttpResponse& res, int statusCode, const std::string& code, const std::string& message);
inline void sendNotFound(HttpResponse& res, const std::string& message) {
    sendError(res, 404, "not_found", message);
}
inline void sendBadRequest(HttpResponse& res, const std::string& message) {
    sendError(res, 400, "bad_request", message);
}
inline void sendUnauthorized(HttpResponse& res, const std::string& message) {
    sendError(res, 401, "unauthorized", message);
}

} // namespace JsonResponse
