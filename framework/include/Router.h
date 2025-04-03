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
 
 #include "HttpRequest.h"
 #include "HttpResponse.h"
 #include "HttpFileserver.h"   // for HttpFileserver
 
 /**
  * @brief Function signature for HTTP route handlers.
  *
  * Route handlers receive the parsed request, response object, and any
  * captured parameters from dynamic paths (e.g., /device/{id}).
  */
 using RouteHandler = std::function<void(Request&, Response&, const std::vector<std::string>&)>;
 
 /**
  * @brief Function signature for middleware.
  *
  * Middleware may short-circuit request handling by returning false.
  */
 using Middleware = std::function<bool(Request&, Response&, const std::vector<std::string>&)>;
 
 /**
  * @brief A single route entry in the Router.
  */
 struct Route {
     std::string method;            ///< HTTP method (GET, POST, etc.)
     std::string path;              ///< Regex path pattern
     RouteHandler handler;          ///< Handler to invoke if matched
     bool is_dynamic;               ///< Whether the route uses parameter matching
     bool requires_auth;            ///< If true, route requires JWT auth
 
     /**
      * @brief Construct a new Route.
      * @param method HTTP method
      * @param path Regex path pattern
      * @param handler The handler function
      * @param is_dynamic True if the route includes parameters
      * @param requires_auth True if JWT auth is required
      */
     Route(const std::string& method,
           const std::string& path,
           RouteHandler handler,
           bool is_dynamic,
           bool requires_auth);
 };
 
 /**
  * @brief The central router for handling HTTP requests and middleware.
  * 
  * Supports:
  * - Dynamic route registration
  * - Regex-based matching
  * - Global and per-route middleware
  * - Optional JWT authentication
  */
 class Router {
 public:
     /**
      * @brief Construct the router instance.
      */
     Router();
 
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
     void addRoute(const std::string& method,
                   const std::string& path,
                   RouteHandler handler,
                   std::vector<Middleware> middleware = {});
 
     /**
      * @brief Handle an incoming HTTP request.
      * @param client_socket Client socket
      * @param method HTTP method
      * @param uri Request URI
      * @param req Fully parsed Request object
      * @return true if the request was handled
      * @return false if no matching route was found
      */
     bool handleRequest(int client_socket,
                        const char* method,
                        const char* uri,
                        Request& req);
 
     /**
      * @brief Built-in route handler for /auth token testing.
      * @param req Incoming request
      * @param res Response to send
      * @param params Unused in this handler
      * @return true on success
      */
     bool handle_auth_req(Request& req, Response& res, const std::vector<std::string>& params);
 
     /**
      * @brief Extracts and caches a Bearer token from an Authorization header.
      * @param auth_header Raw header string
      * @return true if token is valid
      */
     bool extractAuthorizationToken(const std::string& auth_header);
 
     /**
      * @brief Returns the cached Authorization token, or extracts it from the request.
      * @param req The incoming request
      * @return Extracted Bearer token or empty string
      */
     std::string getAuthorizationToken(const Request& req);
 
     /**
      * @brief Check if a route requires and is granted JWT authorization.
      * @param route The matched route
      * @param req HTTP request
      * @param res HTTP response
      * @return true if access is granted
      */
     bool isAuthorizedForRoute(const Route& route, Request& req, Response& res);
 
     /**
      * @brief Print all registered routes to stdout.
      */
     void printRoutes();
 
     /**
      * @brief Get the file server instance.
      * @return A reference to the internal HttpFileserver
      */
     HttpFileserver& getFileHandler() { return fileServer; }

     /**
      * @brief Serve static files from the internal HttpFileserver.
      */
     void serveStatic(Request &req, Response &res, const std::vector<std::string> &params);

    /**
      * @brief Convenience method to list directory from the internal HttpFileserver.
      */
     void listDirectory(Request &req, Response &res, const std::vector<std::string> &params);

 
 private:
     HttpFileserver fileServer; ///< Internal file server instance
     std::unordered_map<std::string, std::vector<Route>> routes;
     std::string cached_token; ///< Cached Bearer token
     std::vector<Middleware> globalMiddleware;
 };
 
 #endif // ROUTER_HPP
 