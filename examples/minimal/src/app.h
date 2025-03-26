#ifndef APP_H
#define APP_H

#include "HttpServer.h"
#include "Router.h"
#include "FrameworkApp.h"
#include "FrameworkManager.h"

#include <memory>

class App : public FrameworkApp {
public:
    App(int port);
    void start() override;
    void initRoutes() override;
    void run() override;
    virtual ~App() = default;

private:
    int port;
    std::unique_ptr<HttpServer> server;
    std::unique_ptr<FrameworkManager> manager;
    Router router;
};

#endif
