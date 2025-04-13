#ifndef APP_H
#define APP_H

#include "FrameworkApp.h"

class App : public FrameworkApp {
public:
    App(int port);  // Constructor to initialize the application with a port number

    void initRoutes() override;
    void onStart() override;
    void poll() override;               
    void onEvent(const Event& e) override;

private:
    
};

#endif