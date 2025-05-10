/**
 * @file FileView.h
 * @brief View that loads static HTML/text content from storage
 */

#pragma once
#include "framework/FrameworkView.h"
#include "framework/AppContext.h"
#include "storage/StorageManager.h"
#include <string>

class FileView : public FrameworkView {
public:
    explicit FileView(const std::string& path, const std::string& contentType = "text/html")
        : filePath(path), mimeType(contentType) {}

    std::string render(const std::map<std::string, std::string>& context = {}) const override {
        auto* sm = AppContext::get<StorageManager>();
        if (!sm) return "<h1>Storage unavailable</h1>";

        std::string result;
        if (!sm->readFileString(filePath, 0, UINT32_MAX, result)) {
            return "<h1>File not found: " + filePath + "</h1>";
        }
        return result;
    }

    std::string getContentType() const override {
        return mimeType;
    }

private:
    std::string filePath;
    std::string mimeType;
};
