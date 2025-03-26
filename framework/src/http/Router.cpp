/**
 * @file Router.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "Router.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "JwtAuthenticator.h"
#include "Middleware.h"
#include <regex>
#include <cstring>
#include <iostream>


// ----- struct Route constructor
Route::Route(const std::string& method,
             const std::string& path,
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
Router::Router()
{
    // empty or do some initialization

}

bool Router::extractAuthorizationToken(const std::string &auth_header) {
    // Check if the header starts with "Bearer " and extract the token
    if (auth_header.substr(0, 7) == "Bearer ") {
        cached_token = auth_header.substr(7);  // Extract token part
        return true;
    }
    return false;  // Invalid Authorization header format
}

std::string Router::getAuthorizationToken(const Request &req) {
    // If cached_token is empty, extract it from the header
    if (cached_token.empty()) {
        std::string auth_header = req.getHeader("Authorization");
        if (!auth_header.empty()) {
            if (!extractAuthorizationToken(auth_header)) {
                printf("Error: Invalid Authorization format\n");
            }
        }
    }
    return cached_token;  // Return the cached token (could be empty if not set)
}

bool Router::handle_auth_req(Request &req, Response &res, const std::vector<std::string> &params)
{   
    TRACE("Handling /auth\n");
    // Check for the Authorization header
    std::string auth_header = req.getHeader("Authorization");
    if (auth_header.empty()) {
        printf("Missing Authorization header\n");
        res.status(401)
           .set("Content-Type", "application/json")
           .send("{\"error\":\"Missing Authorization header\"}");
        return false;
    }
    else{
        std ::string token = getAuthorizationToken(req);
        TRACE("Token: %s\n", token.c_str());
        res.status(200)
           .set("Content-Type", "application/json")
           .send("{\"token\":\"" +  token + "\"}"); 
            TRACE("Authorization header: %s\n", auth_header.c_str());
           return true;
    }
}

//----- addRoute
// void Router::addRoute(const std::string& method,
//                       const std::string& path,
//                       RouteHandler handler,
//                       bool requires_auth) 
// {
    
//     TRACE("Adding route to router %p: %s %s\n", this, method.c_str(), path.c_str());
//     std::string regex_pattern = "^" + path;  // Anchor the regex at the start

//     bool is_dynamic = false;
//     size_t pos = 0;
//     while ((pos = regex_pattern.find("{", pos)) != std::string::npos) {
//         size_t end_pos = regex_pattern.find("}", pos);
//         if (end_pos != std::string::npos) {
//             // Replace {variable} with capturing group
//             regex_pattern.replace(pos, end_pos - pos + 1, "(\\w+)");
//             is_dynamic = true;
//             pos = end_pos + 1;
//         } else {
//             break;
//         }
//     }

//     // Handle the catch-all scenario
//     if (regex_pattern == "/.*") {
//         regex_pattern = "^/(.*)$";  // Catch-all route that matches anything after '/'
//         is_dynamic = true;
//     }

//     regex_pattern += "$";  // Anchor the regex at the end
//     //TRACE("Adding route: %s %s\n", method.c_str(), regex_pattern.c_str());
//     routes[method].emplace_back(method, regex_pattern, handler, is_dynamic, requires_auth);
// }

void Router::addRoute(const std::string& method,
                      const std::string& path,
                      RouteHandler handler,
                      std::vector<Middleware> middleware)  
{
    TRACE("Adding route to router %p: %s %s\n", this, method.c_str(), path.c_str());
    std::string regex_pattern = "^" + path;  // Start regex pattern

    bool is_dynamic = false;
    size_t pos = 0;
    while ((pos = regex_pattern.find("{", pos)) != std::string::npos) {
        size_t end_pos = regex_pattern.find("}", pos);
        if (end_pos != std::string::npos) {
            // Replace {variable} with capturing group
            regex_pattern.replace(pos, end_pos - pos + 1, "(\\w+)");
            is_dynamic = true;
            pos = end_pos + 1;
        } else {
            break;
        }
    }

    // Handle the catch-all scenario
    if (regex_pattern == "/.*") {
        regex_pattern = "^/(.*)$";  // Catch-all route that matches anything after '/'
        is_dynamic = true;
    }

    regex_pattern += "$";  // Anchor regex

    RouteHandler finalHandler = [handler, this, middleware](Request &req, Response &res, const std::vector<std::string> &params) {
        // Run global middleware first
        for (const auto& mw : this->globalMiddleware) {  // Use `this->` to access class members
            if (!mw(req, res, params)) return;  // 🚨 Stop if middleware fails
        }
    
        // Run per-route middleware
        for (const auto& mw : middleware) {
            if (!mw(req, res, params)) return;
        }
    
        handler(req, res, params);
    };

    // Ensure routes are stored exactly as before
    routes[method].emplace_back(method, regex_pattern, finalHandler, is_dynamic, !middleware.empty());
}

void Router::use(Middleware middleware) {
    globalMiddleware.push_back(middleware);
}


// Helper function to split a path into segments
// std::vector<std::string> splitPath(const std::string& path) {
//     std::vector<std::string> segments;
//     std::stringstream ss(path);
//     std::string segment;
//     while (std::getline(ss, segment, '/')) {
//         if (!segment.empty()) {
//             segments.push_back(segment);
//         }
//     }
//     return segments;
// }

// ----- addRoute
// void Router::addRoute(const std::string& method,
//                       const std::string& path,
//                       RouteHandler handler,
//                       bool requires_auth) 
// {
//     TRACE("Adding route: %s %s\n", method.c_str(), path.c_str());
//     bool is_dynamic = path.find("{") != std::string::npos;
//     routes[method].emplace_back(method, path, handler, is_dynamic, requires_auth);
// }

// Function to authenticate user (placeholder)
bool authenticateUser(const std::string& userId, const std::string& password) {
    // Placeholder for actual authentication logic
    // For example, check against a database or a hardcoded list
    if (userId == "admin" && password == "password") {
        std::string userName = "John Doe";  // Example user name
 
        // Generate the token
        std::string jwtToken = Authenticator::getInstance().generateJWT(userId, userName);

        // Now you can send this token to the client (e.g., via HTTP response body or as a cookie)
        //std::cout << "Generated JWT: " << jwtToken << std::endl;
        return true;  // Authentication successful
    }
    return false;  // Authentication failed
}

// Function to extract the JWT token from the Authorization header

std::string getAuthorizationToken(const Request& req) {
    // Look for the Authorization header in the request
    // std::cout << "Printing the unordered_map:" << std::endl;
    // for (const auto& pair : req.headers) {
    //     std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    // }
    auto it = req.getHeaders().find("authorization");
    if (it != req.getHeaders().end()) {
        std::string authHeader = it->second;

        // Check if the Authorization header starts with "Bearer " (case-sensitive)
        if (authHeader.find("Bearer ") == 0) {
            // Extract and return the token (remove the "Bearer " prefix)
            return authHeader.substr(7);  // Token starts after the "Bearer " prefix
        }
    }
    printf("Authorization header not found\n");
    // If the Authorization header is missing or doesn't contain "Bearer ", return an empty string
    return "";
}

// Function to check if the route is authorized
bool Router::isAuthorizedForRoute(const Route& route, Request& req, Response& res) {
    // Check if the route requires authorization
    if (route.requires_auth) {
        printf("Authorization required for route: %s\n", route.path.c_str());
        // Extract token from Authorization header
        std::string token = getAuthorizationToken(req);  // You would have a function like this to get the token
        TRACE("Token: %s\n", token.c_str());
        bool validated = Authenticator::getInstance().validateJWT(token);
        TRACE("Token validated: %s\n", validated ? "true" : "false");
        if (token.empty() || !Authenticator::getInstance().validateJWT(token), true) {  // Token is missing or invalid
            res.status(401).send("Unauthorized");  // Send a 401 Unauthorized response
            printf("Authorization failed - route handled\n");
            return false;  // Authorization failed
        }
    }
    else {
        printf("No authorization required for route: %s\n", route.path.c_str());
    }
    return true;  // Authorization not required or successful
}

//----- handleRequest
bool Router::handleRequest(int client_socket, const char* method, const char* uri, Request& req)
{    
    printf("Router handling request: %s %s on socket %d\n", method, uri, client_socket);
    TRACE("Router %p in handleRequest: %s\n", this);
 
    // Build Response object
    Response res(client_socket); 

    // Find routes for the method
    auto it = routes.find(method);
    if (it == routes.end()) {
        printf("No routes found for method: %s\n", method);
        return false;
    }
    printf("Found routes for method: %s\n", method);
   
    // Handle matching routes
    for (const auto& route : it->second) {
        std::string uri_str(uri);
        std::regex route_regex(route.path);
        std::smatch match;
    
        if (std::regex_match(uri_str, match, route_regex)) {
            std::vector<std::string> params;
            for (size_t i = 1; i < match.size(); ++i) {
                params.push_back(match[i].str());
            }
            
            TRACE("Matched route: %s\n", route.path.c_str());
            route.handler(req, res, params);
            return true;
        }
    }
    
    // for (const auto& route : it->second) {
    //     std::string_view uri_view(uri);  // Use string_view to avoid unnecessary copies
       
    //     // Dynamic route (regex match)
    //     if (route.is_dynamic) {
    //         std::regex route_regex(route.path);  // Precompiled regex can be stored for each route
    //         std::smatch match;

    //         // Convert string_view to std::string for regex_match
    //         std::string uri_str(uri_view); // Convert string_view to std::string
    //         //if (std::regex_match(uri_str, match, route_regex)) {
    //             std::regex route_regex(route.path);  
    //             std::smatch match;
    //             std::string uri_str(uri_view);
                
    //             if (std::regex_match(uri_str, match, route_regex)) {
    //             std::vector<std::string> params;
    //             for (size_t i = 1; i < match.size(); ++i) {  // Start from 1 (0 is full match)
    //                 params.push_back(match[i].str());
    //             }                  
    //             TRACE("Dynamic route matched: %s\n", route.path.c_str());
    //             TRACE("Params: %s\n", params[0].c_str());
    //             //if(isAuthorizedForRoute(route, req, res)) { // Check authorization
    //                 route.handler(req, res, params);  // Handle dynamic route
    //                 TRACE("Dynamic route handled\n");
    //                 return true;
    //             //}

    //         }
    //     }
    //     else {  // Static route
    //         std::string uri_str(uri_view); // Convert string_view to std::string
    //         if (std::regex_match(uri_str, std::regex(route.path))) {
    //             TRACE("Static route matched: %s\n", route.path.c_str());
    //             //if(isAuthorizedForRoute(route, req, res)) { // Check authorization                   
    //                 route.handler(req, res, {});  // Handle static route
    //                 TRACE("Static route handled\n");
    //                 return true;
    //             //}
    //         }
    //     }
    // }
    printf("No matching route found\n");
    printRoutes();
    return false;
}

// bool Router::handleRequest(int client_socket, const char* method, const char* uri, Request& req)
// {    
//     std::cout << "Router address in handleRequest: " << this << std::endl;
//     //TRACE("Router %p handling request: %s %s %d\n", this, method, uri, client_socket);

//     if (!uri) {
//         printf("ERROR: uri is NULL!\n");
//         return false;
//     }

//     printf("Router object check: %p\n", this);
//     if (this == nullptr) {
//         printf("ERROR: Router instance is NULL!\n");
//         return false;
//     }

//     printf("Routes map size: %zu\n", routes.size());  // <-- If it crashes here, `routes` is invalid

//     //printRoutes(); // This prints all registered routes


//     auto it = routes.find(method);
//     if (it == routes.end()) {
//         printf("No routes found for method: %s\n", method);
//         return false;
//     }
    
//     std::vector<std::string> request_segments = splitPath(std::string(uri)); // Convert uri to std::string

//     Route* best_match = nullptr;
//     std::vector<std::string> best_params;
    
//     for (auto& route : it->second) {
//         std::vector<std::string> route_segments = splitPath(route.path);
        
//         std::vector<std::string> params;
//         bool match = true;
        
//         for (size_t i = 0; i < route_segments.size(); ++i) {
//             if (i >= request_segments.size()) {
//                 match = false;
//                 break;
//             }
            
//             if (route_segments[i].front() == '{' && route_segments[i].back() == '}') {
//                 // Handle catch-all (*) route
//                 if (route_segments[i].size() > 2 && route_segments[i][route_segments[i].size() - 2] == '*') {
//                     if (i == route_segments.size() - 1) { // Ensure it's the last segment
//                         std::string uri_str(uri);  // Convert to std::string
//                         std::size_t path_start = uri_str.find(request_segments[i]);

//                         if (path_start != std::string::npos) {
//                             std::string remaining_path = uri_str.substr(path_start);
//                             params.push_back(remaining_path);
//                             match = true;
//                         } else {
//                             printf("ERROR: Cannot find segment %s in URI %s\n", request_segments[i].c_str(), uri);
//                             params.push_back(uri_str);  // Fallback to full URI
//                         }
//                         break; // Stop checking further segments
//                     } else {
//                         match = false; // Catch-all must be the last part
//                         break;
//                     }
//                 } else {
//                     params.push_back(request_segments[i]);
//                 }
//             } else if (route_segments[i] != request_segments[i]) {
//                 match = false;
//                 break;
//             }
//         }
        
//         // Check for exact match on static and dynamic routes
//         if (match && request_segments.size() == route_segments.size()) {
//             printf("Matched exact route: %s\n", route.path.c_str());

//             Response res(client_socket); // Ensure Response is created
//             route.handler(req, res, params); // Correct function call
//             return true;
//         }

//         // Check for potential catch-all match
//         if (match && route_segments.back().size() > 2 && route_segments.back()[route_segments.back().size() - 2] == '*') {
//             best_match = &route;
//             best_params = params;
//         }
//     }
    
//     // If no exact match, but we found a catch-all, use it
//     if (best_match) {
//         printf("Matched catch-all route: %s\n", best_match->path.c_str());

//         Response res(client_socket); // Ensure Response is passed
//         best_match->handler(req, res, best_params); // Fix function call
//         return true;
//     }

//     printf("No matching route found\n");
//     return false;
// }


void Router::printRoutes() {
    printf("Routes:\n");
    for (const auto& method_pair : routes) {
        printf("Method: %s\n", method_pair.first.c_str());
        for (const auto& route : method_pair.second) {
            printf("  Route: %s, Dynamic: %s, Requires Auth: %s\n", 
                   route.path.c_str(), 
                   route.is_dynamic ? "true" : "false", 
                   route.requires_auth ? "true" : "false");
        }
    }
}
