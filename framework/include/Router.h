/**
 * @file Router.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpFileserver.h"   // for HttpFileServer
// #include "scheduler.hpp"    // for Scheduler
// #include "controller.hpp"   // for Controller

using RouteHandler = std::function<void(Request&, Response&, const std::vector<std::string>&)>;
using Middleware = std::function<bool(Request&, Response&, const std::vector<std::string>&)>;


    struct Route {
        std::string method;
        std::string path;        // This will be our regex pattern if dynamic
        RouteHandler handler;
        bool is_dynamic;
        bool requires_auth;      // Flag to indicate if authorization is required

        Route(const std::string& method,
            const std::string& path,
            RouteHandler handler,
            bool is_dynamic,
            bool requires_auth);  // Constructor for Route

    };

class Router {
    
public:

    void use(Middleware middleware);

    Router();  // constructor

    // For adding a route
    //void addRoute(const std::string& method, const std::string& path, RouteHandler handler, bool rerequires_auth = false);
    void addRoute(const std::string& method,
        const std::string& path,
        RouteHandler handler,
        std::vector<Middleware> middleware = {});  // Add route with middleware

    // For handling requests
    // Update handleRequest to accept the Request object
    bool handleRequest(int client_socket,
        const char* method,
        const char* uri,
        Request& req);  // Accept Request& instead of raw headers

    HttpFileServer* getFileServer() { return &fileServer; }   
    
    bool handle_auth_req(Request &req, Response &res, const std::vector<std::string> &params);

    bool extractAuthorizationToken(const std::string &auth_header);
    std::string getAuthorizationToken(const Request &req);
    bool isAuthorizedForRoute(const Route& route, Request& req, Response& res);
    void printRoutes();
    HttpFileServer& getFileHandler() { return fileServer; };

private:
    HttpFileServer fileServer; // a non-static instance
    std::unordered_map<std::string, std::vector<Route>> routes;
    std::string cached_token;  // Cache the token for future use
    std::vector<Middleware> globalMiddleware;
};

#endif // ROUTER_HPP
