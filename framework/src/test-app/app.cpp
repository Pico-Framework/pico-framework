#include "App.h"
#include <hardware/adc.h>
#include <pico/cyw43_arch.h>
#include "AppContext.h"
#include "StorageManager.h"

App::App(int port) : FrameworkApp(port, "AppTask", 2048, 1)
{
    initRoutes();
}

// Function to read the onboard temperature sensor
float read_onboard_temperature(const char unit)
{
    adc_init();
    adc_set_temp_sensor_enabled(true);

    // Conversion factor for 12-bit ADC
    const float conversionFactor = 3.3f / (1 << 12);
    // Read ADC value
    adc_select_input(4);
    float adc = (float)adc_read() * conversionFactor;
    // Convert ADC value to temperature in Celsius
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;
    // Convert to Fahrenheit if requested
    if (unit == 'C')
    {
        return tempC;
    }
    else if (unit == 'F')
    {
        return tempC * 9 / 5 + 32;
    }
    return -1.0f;
}

void App::deleteFile(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    printf("[App] Handling file delete request...\n");
    StorageManager *fs = AppContext::getInstance().getService<StorageManager>();
    if (!fs->isMounted())
    {
        printf("[LittleFS Test] Mounting filesystem...\n");
        assert(fs->mount() && "Mount failed");
    }
    printf("[LittleFS Test] Mount successful\n");
    const std::string path = "/uploads/" + params[0];
    // check file exists
    if (fs->exists(path))
    {
        printf("[LittleFS Test] File already exists, deleting...\n");
        assert(fs->remove(path) && "Delete failed");
        res.status(200).send("File deleted: " + params[0]);
        return;
    }
    res.status(404).send("File does not exist: " + params[0]);
}

void App::getTemperature(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    float temperature = read_onboard_temperature('C');
    printf("[App] Temperature: %.2f\n", temperature);
    res.json({{"temperature", temperature}});
}

void App::getLedState(HttpRequest& req, HttpResponse& res, const std::vector<std::string>& /*params*/) {
    bool isOn = cyw43_arch_gpio_get(0); // Read the state of the LED pin
    res.json({{"state", isOn ? 1 : 0}});
}

void App::ledOn(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    printf("[App] LED on\n");
    cyw43_arch_gpio_put(0, 1); // Turn the LED on
    res.status(200).send("LED turned off");
}

void App::ledOff(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    printf("[App] LED off\n");
    cyw43_arch_gpio_put(0, 0); // Turn the LED off
    res.status(200).send("LED turned off");
}

bool setPin(int pin, int value)
{
    gpio_init(pin);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_set_dir(pin, GPIO_OUT);
    if (value == 0)
    {
        gpio_put(pin, false);
        return true;
    }
    else if (value == 1)
    {
        
        gpio_put(pin, true);
        return true;
    }
    return false;
}
bool getPin(int pin)
{
    return gpio_get(pin);
}

void App::getState(HttpRequest &req, HttpResponse &res, const std::vector<std::string> params)
{
    int pin = std::stoi(params[0]);
    bool state = getPin(pin);
    printf("[App] Pin %d get state: %d\n", pin, state);
    res.json({{"pin", pin},
              {"state", state ? 1 : 0}});
}

void App::setState(HttpRequest &req, HttpResponse &res, const std::vector<std::string> params)
{
    int pin = std::stoi(params[0]);
    int value = std::stoi(params[1]);

    if (value != 0 && value != 1)
    {
        res.status(400).json({{"error", "Invalid value"}});
        return;
    }

    setPin(pin, value);
    printf("[App] Pin %d state set to %d\n", pin, value);
    res.json({{"pin", pin},
              {"state", value}});
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

    router.addRoute("GET", "/api/v1/gpio/{pin}", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->getState(req, res, params); });

    router.addRoute("POST", "/api/v1/gpio/{pin}/{value}", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->setState(req, res, params); });

    router.addRoute("DELETE", "/uploads/{file}", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->deleteFile(req, res, params); });

    router.addRoute("GET", "/api/v1/led", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->getLedState(req, res, params); });

    router.addRoute("POST", "/api/v1/led/0", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->ledOff(req, res, params); });

    router.addRoute("POST", "/api/v1/led/1", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->ledOn(req, res, params); });

    router.addRoute("GET", "/api/v1/temperature", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->getTemperature(req, res, params); });

    router.addRoute("GET", "/ping", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    {
        printf("[App] Ping request received\n");
        res.status(200).send("pong");
        printf("[App] Sent pong\n"); });

    // Catch-all route for static files
    router.addRoute("GET", "/(.*)", [this](HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
                    { this->router.serveStatic(req, res, params); });
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