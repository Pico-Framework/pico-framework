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

#include "Router.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "JwtAuthenticator.h"
#include "Middleware.h"
#include <regex>
#include "utility.h"
#include "url_utils.h"
#include "JsonResponse.h"
#include "AppContext.h"

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

// ----- Route struct constructor
/// @copydoc Route::Route
Route::Route(const std::string &method,
             const std::string &path,
             RouteHandler handler,
             bool is_dynamic,
             bool requires_auth)
    : method(method),
      path(path),
      handler(handler),
      is_dynamic(is_dynamic),
      requires_auth(requires_auth)
{
}

// ----- Router constructor
/// @copydoc Router::Router
Router::Router()
{
    // Initialization if needed.
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

// Adds a route to the router, converting dynamic segments (e.g. {id}) to regex groups.
/// @copydoc Router::addRoute
void Router::addRoute(const std::string &method,
                      const std::string &path,
                      RouteHandler handler,
                      std::vector<Middleware> middleware)
{
    TRACE("Adding route: %s %s\n", method.c_str(), path.c_str());

    // Build regex pattern from path.
    std::string regex_pattern = "^" + path;
    bool is_dynamic = false;
    size_t pos = 0;
    while ((pos = regex_pattern.find("{", pos)) != std::string::npos)
    {
        size_t end_pos = regex_pattern.find("}", pos);
        if (end_pos != std::string::npos)
        {
            // Replace dynamic segment with capturing group for URL parameters.
            regex_pattern.replace(pos, end_pos - pos + 1, "([^/]+)");
            is_dynamic = true;
            pos += std::string("([^/]+)").size();
        }
        else
        {
            break;
        }
    }

    // Handle catch-all route scenario.
    if (regex_pattern == "/.*")
    {
        regex_pattern = "^/(.*)$";
        is_dynamic = true;
    }
    regex_pattern += "$"; // Anchor the regex.

    // Final handler executes global middleware, then route-specific middleware, then the actual handler.
    RouteHandler finalHandler = [handler, this, middleware](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
    {
        // Execute global middleware.
        for (const auto &mw : this->globalMiddleware)
        {
            if (!mw(req, res, params))
                return;
        }
        // Execute per-route middleware.
        for (const auto &mw : middleware)
        {
            if (!mw(req, res, params))
                return;
        }
        // Execute the route handler.
        handler(req, res, params);
    };

    // Store the route; note: using !middleware.empty() as a simple indicator of auth requirement.
    routes[method].emplace_back(method, regex_pattern, finalHandler, is_dynamic, !middleware.empty());
}

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
  
    if (route.requires_auth)
    {
        TRACE("Authorization required for route: %s\n", route.path.c_str());
        std::string token = getAuthorizationToken(req);
        TRACE("Token: %s\n", token.c_str());
        // Fixed the conditional check: removed the erroneous comma operator.
        if (token.empty() || !AppContext::getInstance().getService<JwtAuthenticator>()->validateJWT(token))
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

// Handles an incoming request by matching the request method and URI to a registered route.
/// @copydoc Router::handleRequest
bool Router::handleRequest(int client_socket, const char *method, const char *uri, HttpRequest &req)
{
    TRACE("Handling request: %s %s on socket %d\n", method, uri, client_socket);
    HttpResponse res(client_socket);

    // Look up routes by HTTP method.
    auto it = routes.find(method);
    if (it == routes.end())
    {
        TRACE("No routes found for method: %s\n", method);
        return false;
    }

    // Check each route for a match.
    for (const auto &route : it->second)
    {
        std::string uri_str(uri);
        std::regex route_regex(route.path);
        std::smatch match;

        if (std::regex_match(uri_str, match, route_regex))
        {
            // Extract parameters from the capturing groups.
            std::vector<std::string> params;
            for (size_t i = 1; i < match.size(); ++i)
            {
                params.push_back(urlDecode(match[i].str()));
            }

            TRACE("Matched route: %s\n", route.path.c_str());
            route.handler(req, res, params);
            return true;
        }
    }

    TRACE("No matching route found\n");
    printRoutes();
    return false;
}

// Prints all registered routes (for debugging).
/// @copydoc Router::iprintRoutes
void Router::printRoutes()
{
    TRACE("Routes:\n");
    for (const auto &method_pair : routes)
    {
        TRACE("Method: %s\n", method_pair.first.c_str());
        for (const auto &route : method_pair.second)
        {
            TRACE("  Route: %s, Dynamic: %s, Requires Auth: %s\n",
                  route.path.c_str(),
                  route.is_dynamic ? "true" : "false",
                  route.requires_auth ? "true" : "false");
        }
    }
}

// Serve static files using the HttpFileserver instance.
/// @copydoc Router::serveStatic
void Router::serveStatic(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    fileServer.handle_static_request(req, res, params);
}

// Convenience method to list directory contents using the HttpFileserver instance.
/// @copydoc Router::listDirectory
void Router::listDirectory(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    fileServer.handle_list_directory(req, res, params);
}
