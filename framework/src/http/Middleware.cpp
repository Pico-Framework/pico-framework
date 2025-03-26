#include "middleware.hpp"
#include <iostream>

Middleware authMiddleware = [](Request &req, Response &res, const std::vector<std::string> &params) {
    std::string token = req.getHeader("Authorization");
    if (token.empty() || token.find("Bearer ") != 0) {
        res.status(401).send("{\"error\":\"Unauthorized\"}");
        return false;  // 🚨 Block unauthorized requests
    }

    token = token.substr(7);
    if (!Authenticator::getInstance().validateJWT(token)) {
        res.status(401).send("{\"error\":\"Invalid token\"}");
        return false;  // 🚨 Block invalid tokens
    }

    return true;  // ✅ Let request proceed if authorized
};


Middleware loggingMiddleware = [](Request &req, Response &res, const std::vector<std::string> &params) {
    std::cout << "Received request: " << req.getMethod() << " " << req.getPath() << std::endl;
    return true;
};