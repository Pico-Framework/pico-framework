#include "framework/FileView.h"
#include "framework/AppContext.h"
#include "storage/StorageManager.h"
#include "http/HttpResponse.h"  // for setHeader

// res.send(FileView("/data/log.csv", "text/csv", true));

FileView::FileView(const std::string& path, const std::string& contentType, bool asDownload)
    : path_(path), contentType_(contentType), download_(asDownload) {}

std::string FileView::render(const std::map<std::string, std::string>&) const {
    auto* storage = AppContext::get<StorageManager>();
    std::vector<uint8_t> data;
    if (storage && storage->readFile(path_, data)) {
        return std::string(data.begin(), data.end());
    }
    return "<h1>404 Not Found</h1>";
}

std::string FileView::getContentType() const {
    return contentType_;
}

void FileView::applyHeaders(HttpResponse& response) const {
    if (download_) {
        std::string filename = path_.substr(path_.find_last_of('/') + 1);
        response.setHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    }
}
