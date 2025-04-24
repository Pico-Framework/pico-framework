#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <functional>

class HttpRequest;
class HttpResponse;

struct RouteMatch {
    std::vector<std::string> ordered;
    std::unordered_map<std::string, std::string> named;

    std::optional<std::string> getParam(const std::string& name) const {
        auto it = named.find(name);
        if (it != named.end()) return it->second;
        return std::nullopt;
    }
};

using RouteHandler = std::function<void(HttpRequest&, HttpResponse&, const RouteMatch&)>;
using Middleware   = std::function<bool(HttpRequest&, HttpResponse&, const RouteMatch&)>;

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