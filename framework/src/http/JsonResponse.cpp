#include "JsonResponse.h"

namespace JsonResponse {

void sendSuccess(HttpResponse& res, const nlohmann::json& data, const std::string& message) {
    nlohmann::json j = {
        {"success", true},
        {"data", data}
    };
    if (!message.empty()) {
        j["message"] = message;
    }
    res.status(200).json(j);
}

void sendMessage(HttpResponse& res, const std::string& message) {
    nlohmann::json j = {
        {"success", true},
        {"message", message}
    };
    res.status(200).json(j);
}

void sendCreated(HttpResponse& res, const nlohmann::json& data, const std::string& message) {
    nlohmann::json j = {
        {"success", true},
        {"data", data}
    };
    if (!message.empty()) {
        j["message"] = message;
    }
    res.status(201).json(j);
}

void sendNoContent(HttpResponse& res) {
    res.status(204).send("");  // No body
}

void sendError(HttpResponse& res, int statusCode, const std::string& code, const std::string& message) {
    nlohmann::json j = {
        {"success", false},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    };
    res.status(statusCode).json(j);
}

} // namespace JsonResponse
