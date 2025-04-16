#include "App.h"

App::App(int port) : FrameworkApp(port, "AppTask", 2048, 1)
{
    std::cout << "App constructed" << std::endl;
    initRoutes();
}

void App::initRoutes()
{

    // Serve embedded upload HTML
    router.addRoute("GET", "/upload", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
    {
        static const char* uploadHtml = R"rawliteral(
          <!DOCTYPE html>
          <html lang="en">
          <head>
            <meta charset="UTF-8">
            <title>Upload</title>
          </head>
          <body>
            <h1>Upload a File</h1>
            <form method="POST" action="/api/v1/upload" enctype="multipart/form-data">
              <input type="file" name="file" />
              <button type="submit">Upload</button>
            </form>
            <p>Try opening the file at <code>/uploads/filename.jpg</code> after uploading.</p>
          </body>
          </html>
          )rawliteral";
      
        res.setContentType("text/html");
        res.send(uploadHtml); });

    router.addRoute("GET", "/ping", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    {
        printf("[App] Ping request received\n");
        res.status(200).send("pong");
        printf("[App] Sending pong\n"); });
}

void App::onStart()
{
    std::cout << "[App] Waiting for network..." << std::endl;
    waitFor(FrameworkNotification::NetworkReady);

    std::cout << "[App] Network ready. Starting services..." << std::endl;

    server.start();
}

void App::poll()
{
    runEvery(15000, [&]()
             { printf("[App] Running main loop...\n"); }, "logLoop"); // <-- Unique ID for this timer
}

void App::onEvent(const Event &e)
{
    switch (e.type)
    {
    case EventType::NetworkReady:
        std::cout << "[App] Network ready. Starting services..." << std::endl;
        break;
    default:
        break;
    }
}