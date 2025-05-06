#include "PingPongController.h"
#include "framework/AppContext.h"
#include "storage/FileResponse.h"
#include "lwip/netif.h"

PingPongController::PingPongController(const std::string &peerHostname, const std::string &startPath)
    : peerHost("http://" + peerHostname), nextPath(startPath), logger("log.txt")
{
    logger.enableFileLogging("log.txt");
}

void PingPongController::initRoutes()
{
    addRoute(HttpMethod::GET, "/ping", [this](auto &req, auto &res) { handlePing(req, res); });
    addRoute(HttpMethod::GET, "/pong", [this](auto &req, auto &res) { handlePong(req, res); });
    addRoute(HttpMethod::POST, "/config", [this](auto &req, auto &res) { handleConfig(req, res); });
    addRoute(HttpMethod::GET, "/log", [this](auto &req, auto &res) { handleLog(req, res); });

    scheduleNext(); // kick off the first ping or pong
}

void PingPongController::handlePing(HttpRequest &req, HttpResponse &res)
{
    printf("Received ping\n");
    logger.info("Received ping");
    nextPath = "/pong";
    scheduleNext();
    res.text("pong");
}

void PingPongController::handlePong(HttpRequest &req, HttpResponse &res)
{
    printf("Received pong\n");
    logger.info("Received pong");
    nextPath = "/ping";
    scheduleNext();
    res.text("ping");
}

void PingPongController::handleConfig(HttpRequest &req, HttpResponse &res)
{
    auto json = req.json();
    if (json.contains("interval")) {
        intervalMs = json["interval"];
        logger.info("Interval updated via config");
        res.json({{"status", "ok"}, {"interval", intervalMs}});
    } else {
        res.status(400).json({{"error", "Missing interval"}});
    }
}

void PingPongController::handleLog(HttpRequest &req, HttpResponse &res)
{
    auto *storage = AppContext::get<StorageManager>();
    if (storage && storage->exists("log.txt")) {
        res.send(FileResponse("log.txt", "text/plain"));
    } else {
        res.status(404).text("Log file not found");
    }
}

void PingPongController::sendMessage()
{
    HttpRequest req(HttpMethod::GET, peerHost + nextPath);
    auto &client = AppContext::get<HttpClient>();

    client.send(req, [this](HttpResponse &res) {
        if (res.statusCode() == 200) {
            std::out << "Sent " << nextPath <<  " to " peerHost << std::endl;
            logger.info(("Sent " + nextPath + " to " + peerHost).c_str());
        } else {
            logger.warn(("Failed to send " + nextPath + ": " + std::to_string(res.statusCode())).c_str());
        }
    });
}

void PingPongController::scheduleNext()
{
    timer.scheduleOnce(intervalMs, [this]() {
        sendMessage();
    });
}
