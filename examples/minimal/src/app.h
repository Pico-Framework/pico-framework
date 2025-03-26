#ifndef APP_H
#define APP_H

#include "HttpServer.h"
#include "Router.h"
#include "FrameworkApp.h"

class App : public FrameworkApp {
public:
    App(int port);
    void initRoutes() override;
    void run() override;

private:
    HttpServer server;
    Router router;
};

#endif
