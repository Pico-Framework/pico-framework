#ifndef APP_H
#define APP_H

#include "framework/FrameworkApp.h"

class App : public FrameworkApp {
public:
    App(int port);  // Constructor to initialize the app with a specific port

    void onStart() override;
    void poll() override;
};

#endif
