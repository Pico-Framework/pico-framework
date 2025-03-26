// app.hpp (Refactored)
#ifndef APP_HPP
#define APP_HPP

#include "HttpServer.h"
#include "Router.h"

#define APPLICATION_STACK_SIZE 1024
class App {
public:
    App(int port);
    void run();
    void stop();

private:
    void initRoutes();
    HttpServer server;
    Router router;
};

#endif // APP_HPP

