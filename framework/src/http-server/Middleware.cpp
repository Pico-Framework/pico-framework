/**
 * @file Middleware.cpp
 * @author Ian Archbell
 * @brief Predefined middleware implementations for logging and JWT authentication.
 *
 * Part of the PicoFramework HTTP server.
 * This module provides two middleware functions:
 * - `authMiddleware`: Checks for a valid JWT in the Authorization header.
 * - `loggingMiddleware`: Logs the HTTP method and path of incoming requests.
 * Both middleware functions are designed to be used in the HTTP request processing pipeline.
 * If the authentication fails, the `authMiddleware` will respond with an HTTP 401 Unauthorized status.
 *
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */


#include "http/Middleware.h"

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(Middleware)

#include <iostream>
#include "http/HttpResponse.h"
#include "http/JsonResponse.h"
#include "framework/AppContext.h"

/// @copydoc authMiddleware
#ifdef PICO_HTTP_ENABLE_JWT
#include "http/JwtAuthenticator.h"
Middleware authMiddleware = [](HttpRequest &req, HttpResponse &res, const RouteMatch &)
{
    std::string token = req.getHeader("Authorization");
    if (token.empty() || token.find("Bearer ") != 0)
    {
        res.setStatus(401).send("{\"error\":\"Unauthorized\"}");
        return false;
    }

    token = token.substr(7); // Remove "Bearer " prefix
    if (!AppContext::get<JwtAuthenticator>()->validateJWT(token))
    {
        JsonResponse::sendError(res, 401, "INVALID_TOKEN", "Invalid token");
        return false;
    }

    return true; // Authorized
};

#endif

/// @copydoc loggingMiddleware
Middleware loggingMiddleware = [](HttpRequest &req, HttpResponse &res, const RouteMatch &)
{
    std::cout << "Received request: " << req.getMethod() << " " << req.getPath() << std::endl;
    return true;
};

