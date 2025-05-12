/**
 * @file Router.h
 * @author Ian Archbell
 * @brief HTTP routing with middleware and optional JWT-based authorization.
 * Part of the PicoFramework HTTP server.
 * This module provides the Router class, which manages HTTP routes, middleware,
 * and optional JWT authentication. It allows dynamic route registration,
 * regex-based path matching, and supports global and per-route middleware.
 * The Router can handle requests, invoke appropriate handlers, and apply
 * middleware functions in sequence. It also includes a built-in route handler
 * for testing JWT tokens at the /auth endpoint.
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 */

#ifndef ROUTER_HPP
#define ROUTER_HPP
#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <FreeRTOS.h>
#include <semphr.h>
#include "http/RouteTypes.h" // for RouteMatch
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpFileserver.h" // for HttpFileserver


/**
 * @brief Function signature for HTTP route handlers.
 *
 * Route handlers receive the parsed request, response object, and any
 * captured parameters from dynamic paths (e.g., /device/{id}).
 */

using RouteHandler = std::function<void(HttpRequest &, HttpResponse &, const RouteMatch &)>;

/**
 * @brief Function signature for middleware.
 *
 * Middleware may short-circuit request handling by returning false.
 */
using Middleware = std::function<bool(HttpRequest &, HttpResponse &, const RouteMatch &)>;

/**
 * @brief The central router for handling HTTP requests and middleware.
 *
 * Supports:
 * - Dynamic route registration
 * - Regex-based matching
 * - Global and per-route middleware
 * - Optional JWT authentication
 */
class Router
{
public:
    /**
     * @brief Construct the router instance.
     */
    Router();
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    /**
     * @brief Register a global middleware function.
     * @param middleware Middleware to apply to all routes.
     */
    void use(Middleware middleware);

    /**
     * @brief Register a route with optional middleware.
     * @param method HTTP method
     * @param path URL path (can include {params})
     * @param handler Handler to invoke on match
     * @param middleware Optional list of middleware specific to this route
     */
    void addRoute(const std::string &method,
                  const std::string &path,
                  RouteHandler handler,
                  std::vector<Middleware> middleware = {});


    
    // Optional 2-argument route handler overload for convenience
    inline void addRoute(Router& router,
            const std::string& method,
            const std::string& path,
            std::function<void(HttpRequest&, HttpResponse&)> simpleHandler) {
            router.addRoute(method, path,
            [=](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
            simpleHandler(req, res);
        });
    }              

    /**
     * @brief Handle an incoming HTTP request.
     * @param req Fully parsed HttpRequest object
     * @param res HttpResponse to send
     * @return true if the request was handled
     * @return false if no matching route was found
     */
    bool handleRequest(HttpRequest &req, HttpResponse &res);

/**
 * @brief Built-in route handler for /auth token testing.
 * @param req Incoming request
 * @param res HttpResponse to send
 * @param params Unused in this handler
 * @return true on success
 */
#ifdef PICO_HTTP_ENABLE_JWT
    bool handle_auth_req(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params);
#endif

    /**
     * @brief Extracts and caches a Bearer token from an Authorization header.
     * @param auth_header Raw header string
     * @return true if token is valid
     */
    bool extractAuthorizationToken(const std::string &auth_header);

    /**
     * @brief Returns the cached Authorization token, or extracts it from the request.
     * @param req The incoming request
     * @return Extracted Bearer token or empty string
     */
    std::string getAuthorizationToken(const HttpRequest &req);

    /**
     * @brief Check if a route requires and is granted JWT authorization.
     * @param route The matched route
     * @param req HTTP request
     * @param res HTTP response
     * @return true if access is granted
     */
#ifdef PICO_HTTP_ENABLE_JWT
    bool isAuthorizedForRoute(const Route &route, HttpRequest &req, HttpResponse &res);
#else
    bool isAuthorizedForRoute(const Route &route, HttpRequest &req, HttpResponse &res) { return true; }
#endif
    /**
     * @brief Print all registered routes to stdout.
     */
    void printRoutes();

    /**
     * @brief Get the file server instance.
     * @return A reference to the internal HttpFileserver
     */
    HttpFileserver &getFileHandler() { return fileServer; }

    /**
     * @brief Serve static files from the internal HttpFileserver.
     */
    void serveStatic(HttpRequest &req, HttpResponse &res, const RouteMatch &match);

    /**
     * @brief Convenience method to list directory from the internal HttpFileserver.
     */
    void listDirectory(HttpRequest &req, HttpResponse &res, const RouteMatch &match);

private:
    HttpFileserver fileServer; ///< Internal file server instance
    std::unordered_map<std::string, std::vector<Route>> routes;
    std::string cached_token; ///< Cached Bearer token
    std::vector<Middleware> globalMiddleware;
    
    static StaticSemaphore_t lockBuffer_;
    SemaphoreHandle_t lock_ = xSemaphoreCreateRecursiveMutexStatic(&lockBuffer_);

    void withRoutes(const std::function<void(std::unordered_map<std::string, std::vector<Route>> &)> &fn);
};

#endif // ROUTER_HPP
