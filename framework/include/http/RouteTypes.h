#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <functional>

class HttpRequest;
class HttpResponse;

/**
 * @brief Represents a match of a route against an incoming HTTP request.
 * 
 * This structure holds the ordered parameters and named parameters extracted from the route.
 * It provides a method to retrieve a parameter by name.
 */
struct RouteMatch {
    std::vector<std::string> ordered;
    std::unordered_map<std::string, std::string> named;

    std::optional<std::string> getParam(const std::string& name) const {
        auto it = named.find(name);
        if (it != named.end()) return it->second;
        return std::nullopt;
    }
};

/**
 * @brief Type definitions for route handlers and middleware.
 */
using RouteHandler = std::function<void(HttpRequest&, HttpResponse&, const RouteMatch&)>;
using Middleware   = std::function<bool(HttpRequest&, HttpResponse&, const RouteMatch&)>;

/**
 * @brief Represents a single HTTP route.
 * 
 * This structure encapsulates the method, path, handler function, and metadata about the route.
 * It supports both static and dynamic routes, as well as authentication requirements.
 */
struct Route {
    std::string method;
    std::string path;
    RouteHandler handler;
    bool isDynamic;
    bool requiresAuth;
    std::vector<std::string> paramNames;

    Route(const std::string& method, const std::string& path,
          RouteHandler handler, bool isDynamic, bool requiresAuth,
          std::vector<std::string> paramNames = {})
        : method(method), path(path), handler(handler),
          isDynamic(isDynamic), requiresAuth(requiresAuth),
          paramNames(std::move(paramNames)) {}
};