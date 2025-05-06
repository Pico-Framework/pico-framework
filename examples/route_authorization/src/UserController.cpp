#include "UserController.h"
#include "framework/AppContext.h"
#include "http/JwtAuthenticator.h"
#include "http/JsonResponse.h"
#include "http/Middleware.h"
#include "UserModel.h"

UserController::UserController(Router &router)
    : FrameworkController("UserController", router) {}

void UserController::initRoutes()
{
    router.addRoute("GET", "/", [](auto& req, auto& res, auto&) {
        res.redirect("/login", 302);
    });

    router.addRoute("GET", "/login", [this](auto& req, auto& res, auto&) {
        res.send(loginView);  
    });;
    
    router.addRoute("POST", "/signup", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    { handleSignup(req, res, match); });

    router.addRoute("POST", "/auth", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    { handleLogin(req, res, match); });

    /**
     * @brief Example of a protected route using JWT authentication.
     * @param req Incoming request
     * @param res HttpResponse to send
     * @param match RouteMatch object containing matched parameters
     * @param authMiddleware Middleware function to check JWT token
     * @return Sends a JSON response with a message if authorized
     *
     */
    router.addRoute("GET", "/api/v1/protected-data",
                    [](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    {
                        res.json({{"message", "You are authorized!"}});
                    },
                    {authMiddleware});
}

void UserController::handleSignup(HttpRequest &req, HttpResponse &res, const RouteMatch &)
{
    auto json = req.json();
    printf("JSON string: %s\n", req.getBody().c_str());
    if (json.is_null())
    {
        printf("Invalid JSON: %s\n", req.getBody().c_str());
        JsonResponse::sendError(res, 400, "BAD_REQUEST", "Invalid JSON");
        return;
    }
    std::string username = json.value("username", "");
    std::string password = json.value("password", "");

    if (username.empty() || password.empty())
    {
        JsonResponse::sendError(res, 400, "BAD_REQUEST", "Missing username or password");
        return;
    }

    UserModel model;
    if (!model.createUser(username, password))
    {
        JsonResponse::sendError(res, 409, "USER_EXISTS", "User already exists");
        return;
    }

    std::string token = AppContext::get<JwtAuthenticator>()->generateJWT(username, username);
    res.json({{"token", token}});
}

void UserController::handleLogin(HttpRequest &req, HttpResponse &res, const RouteMatch &)
{
    auto json = req.json();
    std::string username = json.value("username", "");
    std::string password = json.value("password", "");

    if (username.empty() || password.empty())
    {
        JsonResponse::sendError(res, 400, "BAD_REQUEST", "Missing username or password");
        return;
    }

    UserModel model;
    if (!model.verifyUser(username, password))
    {
        JsonResponse::sendError(res, 401, "INVALID_CREDENTIALS", "Invalid username or password");
        return;
    }

    std::string token = AppContext::get<JwtAuthenticator>()->generateJWT(username, username);
    res.json({{"token", token}});
}
