/**
 * @file Middleware.h
 * @author Ian Archbell
 * @brief Middleware definitions for HTTP routing (e.g., logging, auth).
 * Part of the PicoFramework HTTP server.
 * This module defines middleware functions that can be used in the HTTP request
 * processing pipeline. Middleware can perform tasks such as authentication,
 * logging, and modifying requests or responses before passing them to the
 * next handler in the chain.
 * Middleware functions are designed to be composable, allowing multiple
 * middleware components to be applied in sequence.
 * Each middleware function can either allow the request to continue to the
 * next handler or stop further processing based on certain conditions.
 * 
 * To create your own middleware:
 * @code
 * Middleware myMiddleware = [](HttpRequest& req, HttpResponse& res, const auto& params) {
 *     Check something
 *     return true;
 * };
 * @endcode
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 */

 #ifndef MIDDLEWARE_HPP
 #define MIDDLEWARE_HPP
 #pragma once
 
 #include "HttpRequest.h"
 #include "HttpResponse.h"
 #include "JwtAuthenticator.h"
 #include <functional>
 #include <vector>
 
 /**
  * @brief Function signature for middleware components.
  * 
  * Middleware functions operate on the request and response objects.
  * They return `true` to allow processing to continue, or `false` to stop further middleware and route execution.
  * 
  * @param req The incoming HTTP request.
  * @param res The HTTP response being constructed.
  * @param params Path parameters captured by the router.
  * @return true to continue to the next middleware or handler; false to halt processing.
  */
 using Middleware = std::function<bool(HttpRequest&, HttpResponse&, const std::vector<std::string>&)>;
 
 /**
  * @brief Authentication middleware that checks for a valid JWT in the Authorization header.
  * 
  * If a valid token is not present, this middleware responds with HTTP 401 Unauthorized.
  *
  * @example
  * Example: Register route with per-route middleware
  * 
  * router.addRoute("GET", "/secure/data", { authMiddleware, loggingMiddleware },
  *     [](HttpRequest& req, HttpResponse& res, const std::vector<std::string>& params)
  *     {
  *         res.json({{"message", "You are authenticated!"}});
  *     }
  * );
  *
  * Example: Middleware halts request if not authorized
  * authMiddleware will return false and send 401, preventing the handler from running
  * 
  */
 extern Middleware authMiddleware;
 
 /**
  * @brief Logging middleware that prints the HTTP method and path to the console.
  * 
  * This middleware always allows processing to continue.
  */
 extern Middleware loggingMiddleware;
 
 #endif // MIDDLEWARE_HPP
 