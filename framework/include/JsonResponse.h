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

 #include "HttpResponse.h"
 #include <nlohmann/json.hpp>
 
 namespace JsonResponse {
 
     void sendSuccess(HttpResponse& res, const nlohmann::json& data, const std::string& message = {});
     void sendMessage(HttpResponse& res, const std::string& message);
     void sendCreated(HttpResponse& res, const nlohmann::json& data, const std::string& message = {});
     void sendNoContent(HttpResponse& res);
 
     void sendError(HttpResponse& res, int statusCode, const std::string& code, const std::string& message);
 
 } // namespace JsonResponse
 