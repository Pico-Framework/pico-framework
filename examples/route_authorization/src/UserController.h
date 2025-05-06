#pragma once

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "framework/FrameworkController.h"
#include "http/JwtAuthenticator.h"
#include "UserView.h"

class UserController : public FrameworkController
{
public:
    UserController(Router &router);

    void initRoutes() override;

private:

    UserView loginView;

    void handleSignup(HttpRequest &req, HttpResponse &res, const RouteMatch &match);
    void handleLogin(HttpRequest &req, HttpResponse &res, const RouteMatch &match);
};
