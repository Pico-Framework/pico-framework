#ifndef APP_H
#define APP_H

#include "FrameworkApp.h"

class App : public FrameworkApp {
public:
    App(int port);  // Constructor to initialize the app with a specific port

    void initRoutes() override;
    void run() override;
};

#endif
