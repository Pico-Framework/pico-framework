#ifndef FRAMEWORK_APP_H
#define FRAMEWORK_APP_H

class FrameworkApp {
public:
    virtual ~FrameworkApp() = default;
    virtual void initRoutes() = 0;
    virtual void run() = 0;
};

#endif // FRAMEWORK_APP_H