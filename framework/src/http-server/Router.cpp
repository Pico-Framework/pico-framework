/**
 * @file Router.cpp
 * @author Ian Archbell
 *
 * @brief HTTP routing with middleware and optional JWT-based authorization.
 *
 * Part of the PicoFramework HTTP server.
 * This module provides the Router class, which manages HTTP routes, middleware,
 * and optional JWT authentication. It allows dynamic route registration,
 * regex-based path matching, and supports global and per-route middleware.
 * The Router can handle requests, invoke appropriate handlers, and apply
 * middleware functions in sequence. It also includes a built-in route handler
 * for testing JWT tokens at the /auth endpoint.
 *
 * @version 0.2
 * @date 2025-03-26
 *
 * Improvements:
 *  - Unified token extraction via helper function.
 *  - Removed token caching to avoid cross-request issues.
 *  - Fixed auth check bug.
 *  - Streamlined route registration and logging.
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(Router)

#include <regex>

#include "http/Router.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/JwtAuthenticator.h"
#include "http/Middleware.h"
#include "utility/utility.h"
#include "http/url_utils.h"
#include "http/JsonResponse.h"
#include "framework/AppContext.h"

SemaphoreHandle_t lock_;
StaticSemaphore_t Router::lockBuffer_;

// -----------------------------------------------------------------------------
// Helper function to extract a bearer token from an Authorization header.
static std::string extractBearerToken(const std::string &auth_header)
{
    const std::string bearerPrefix = "Bearer ";
    if (auth_header.compare(0, bearerPrefix.size(), bearerPrefix) == 0)
    {
        return auth_header.substr(bearerPrefix.size());
    }
    return "";
}

// ----- Router constructor
/// @copydoc Router::Router
Router::Router()
{
    lock_ = xSemaphoreCreateRecursiveMutex();
    configASSERT(lock_);
}

// Returns the bearer token from the request's Authorization header.
/// @copydoc Router::getAuthorizationToken
std::string Router::getAuthorizationToken(const HttpRequest &req)
{
    std::string auth_header = req.getHeader("Authorization");
    std::string token = extractBearerToken(auth_header);
    if (token.empty())
    {
        TRACE("Error: Missing or invalid Authorization header format\n");
    }
    return token;
}

// Handles /auth route: checks for the Authorization header and returns token info.
// If the header is missing, it responds with a 401 Unauthorized status.
// If the header is present, it extracts the token and returns it in the response.
// This is useful for testing JWT token handling.
// Returns true if the request was successfully handled, false otherwise.
// /auth route is typically used for testing JWT token handling and will need to be in the routes.
/// @copydoc Router::handle_auth_req
#ifdef PICO_HTTP_ENABLE_JWT
bool Router::handle_auth_req(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    TRACE("Handling /auth\n");
    std::string auth_header = req.getHeader("Authorization");
    if (auth_header.empty())
    {
        TRACE("Missing Authorization header\n");
        JsonResponse::sendError(res, 401, "UNAUTHORIZED", "Missing authorization header");
        return false;
    }
    else
    {
        std::string token = getAuthorizationToken(req);
        TRACE("Token: %s\n", token.c_str());
        JsonResponse::sendSuccess(res, {{"token", token}}, "Authorization successful");
        return true;
    }
}
#endif

// Adds a middleware to be executed for all routes.
/// @copydoc Router::use
void Router::use(Middleware middleware)
{
    globalMiddleware.push_back(middleware);
}

// Checks if the request is authorized for the given route.
/// @copydoc Router::isAuthorizedForRoute
#ifdef PICO_HTTP_ENABLE_JWT
bool Router::isAuthorizedForRoute(const Route &route, HttpRequest &req, HttpResponse &res)
{

    if (route.requiresAuth)
    {
        TRACE("Authorization required for route: %s\n", route.path.c_str());
        std::string token = getAuthorizationToken(req);
        TRACE("Token: %s\n", token.c_str());
        // Fixed the conditional check: removed the erroneous comma operator.
        if (token.empty() || !AppContext::get<JwtAuthenticator>()->validateJWT(token))
        {
            JsonResponse::sendError(res, 401, "UNAUTHORIZED", "Missing authorization header");
            TRACE("Authorization failed\n");
            return false;
        }
    }
    else
    {
        TRACE("No authorization required for route: %s\n", route.path.c_str());
    }

    return true;
}
#endif

/// @copydoc Router::addRoute
void Router::addRoute(const std::string &method,
                      const std::string &path,
                      RouteHandler handler,
                      std::vector<Middleware> middleware)
{
    printf("Adding route: %s %s\n", method.c_str(), path.c_str());

    std::string regex_pattern = "^" + path;
    bool is_dynamic = false;
    std::vector<std::string> paramNames;
    size_t pos = 0;

    while ((pos = regex_pattern.find("{", pos)) != std::string::npos)
    {
        size_t end = regex_pattern.find("}", pos);
        if (end != std::string::npos)
        {
            std::string paramName = regex_pattern.substr(pos + 1, end - pos - 1);
            paramNames.push_back(paramName);
            regex_pattern.replace(pos, end - pos + 1, "([^/]+)");
            is_dynamic = true;
            pos += std::string("([^/]+)").size();
        }
        else
        {
            break;
        }
    }

    if (regex_pattern == "/.*")
    {
        regex_pattern = "^/(.*)$";
        is_dynamic = true;
    }
    regex_pattern += "$";

    RouteHandler finalHandler = [handler, paramNames, this, middleware](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        for (const auto &mw : this->globalMiddleware)
        {
            if (!mw(req, res, match))
                return;
        }

        for (const auto &mw : middleware)
        {
            if (!mw(req, res, match))
                return;
        }

        handler(req, res, match);
    };

    withRoutes([&](auto &r)
    { 
        r[method].emplace_back(method, regex_pattern, finalHandler, is_dynamic, !middleware.empty(), paramNames);
    });
}

bool Router::handleRequest(HttpRequest &req, HttpResponse &res)
{
    TRACE("Router::handleRequest: %s %s\n", req.getMethod().c_str(), req.getPath().c_str());
    bool matched = false;
    const Route *matchedRoute = nullptr;
    std::vector<std::string> params;

    withRoutes([&](auto &r)
               {
                   auto it = r.find(req.getMethod());
                   if (it == r.end())
                       return;

                   for (const auto &route : it->second)
                   {
                       TRACE("Checking route: %s\n", route.path.c_str());

                       std::smatch match;
                       const std::string &path = req.getPath();
                       if (std::regex_match(path, match, route.compiledRegex)) // <--- USE precompiled
                       {
                           for (size_t i = 1; i < match.size(); ++i)
                           {
                               TRACE("Matched param %zu: %s\n", i, match[i].str().c_str());
                               params.push_back(urlDecode(match[i].str()));
                           }
                           matchedRoute = &route;
                           matched = true;
                           TRACE("Matched route: %s\n", route.path.c_str());
                           return;
                       }
                   }
               });
    TRACE("Matched: %s\n", matched ? "true" : "false");
    if (matched && matchedRoute)
    {
        RouteMatch match;
        match.ordered = params;
        const auto &names = matchedRoute->paramNames;

        for (size_t i = 0; i < names.size() && i < params.size(); ++i)
        {
            match.named[names[i]] = params[i];
        }

        TRACE("Matched route: %s\n", matchedRoute->path.c_str());
        matchedRoute->handler(req, res, match);
        return true;
    }

    TRACE("No matching route found\n");
    printRoutes();
    return false;
}

// Prints all registered routes (for debugging).
/// @copydoc Router::iprintRoutes
void Router::printRoutes()
{
    printf("Routes:\n");
    withRoutes([&](auto &r)
               {
        for (const auto &method_pair : routes)
        {
            printf("Method: %s\n", method_pair.first.c_str());
            for (const auto &route : method_pair.second)
            {
                printf("  Route: %s, Dynamic: %s, Requires Auth: %s\n",
                    route.path.c_str(),
                    route.isDynamic ? "true" : "false",
                    route.requiresAuth ? "true" : "false");
            }
        } });
}

// Serve static files using the HttpFileserver instance.
/// @copydoc Router::serveStatic
void Router::serveStatic(HttpRequest &req, HttpResponse &res, const RouteMatch &match)
{
    fileServer.handle_static_request(req, res, match);
}

// Handle list directory using HttpFileserver instance.
/// @copydoc Router::listDirectory
void Router::listDirectory(HttpRequest &req, HttpResponse &res, const RouteMatch &match)
{

    fileServer.handle_list_directory(req, res, match);
    ;
}

void Router::withRoutes(const std::function<void(std::unordered_map<std::string, std::vector<Route>> &)> &fn)
{
    const char* taskName = pcTaskGetName(nullptr);
    printf("Router: [%s] waiting for lock\n", taskName);

    if (xSemaphoreTakeRecursive(lock_, pdMS_TO_TICKS(5000)) != pdTRUE)
    {
        printf("Router: [%s] ERROR - failed to acquire lock within timeout\n", taskName);
        return;
    }

    printf("Router: [%s] acquired lock\n", taskName);

    fn(routes);  

    printf("Router: [%s] releasing lock\n", taskName);
    xSemaphoreGiveRecursive(lock_);
    vTaskDelay(pdMS_TO_TICKS(1));  // Give any pending route-registration task a chance to run
}