#pragma once
#include "framework/FrameworkView.h"

class FileView : public FrameworkView {
public:
    FileView(const std::string& path, const std::string& contentType = "application/octet-stream", bool asDownload = false);

    std::string render(const std::map<std::string, std::string>& context = {}) const override;
    std::string getContentType() const override;
    void applyHeaders(HttpResponse& response) const override;

private:
    std::string path_;
    std::string contentType_;
    bool download_;
};
