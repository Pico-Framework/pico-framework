#include "StorageController.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "FileStorage_html.h"

StorageController::StorageController(Router &router)
    : FrameworkController("StorageController", router) {}

void StorageController::initRoutes()
{

    router.addRoute("GET", "/", [](HttpRequest &req, HttpResponse &res, const auto &)
                    { res.send(FileStorage_html); });

    router.addRoute("GET", "/api/v1/ls(.*)", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        std::string path = "/"; // default root
        if (!match.ordered.empty() && !match.ordered[0].empty())
        {
            path = match.ordered[0];
        }

        auto files = this->storage.listFiles(path);

        if (!files.empty() && files[0].contains("error")) {
            res.sendError(404, files[0]["error"]);
            printf("[StorageController] Failed to list directory: %s\n", path.c_str());
            return;
        }

        res.json(files);

        printf("[StorageController] Listed files in path '%s'.\n", path.c_str());
        printf("[StorageController] Files: %s\n", files.dump().c_str());
    } );

    router.addRoute("POST", "/api/v1/upload", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        if(req.handle_multipart(res)){
            printf("[StorageController] File uploaded successfully.\n");
        }
        else {
            printf("[StorageController] Failed to upload file.\n");
        } });

    router.addRoute("DELETE", "/api/v1/files(.*)", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match) {
        std::string path = "/";
        if (!match.ordered.empty() && !match.ordered[0].empty()) {
            path = match.ordered[0];
        }
    
        if (this->storage.deleteFile(path)) {
            res.json({{"message", "File deleted successfully"}});
            printf("[StorageController] File '%s' deleted successfully.\n", path.c_str());
        } else {
            res.sendError(404, "File not found");
            printf("[StorageController] Failed to delete file '%s'. File not found.\n", path.c_str());
        }
    });
        

    router.addRoute("POST", "/api/v1/format_storage", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
                    {
        if (this->storage.format()) {
            res.json({{"message", "Storage formatted successfully"}});
            printf("[StorageController] Storage formatted successfully.\n");
        } else {
            res.sendError(500, "Failed to format storage");
            printf("[StorageController] Failed to format storage.\n");
        } });

    // Catch-all route for static files
    router.addRoute("GET", "/(.*)", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    { this->router.serveStatic(req, res, match); });
}
