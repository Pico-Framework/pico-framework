/**
 * @file FileView.h
 * @brief View that streams a file from storage using StorageManager
 * @author Ian Archbell
 * @version 0.1
 * @date 2025-05-09
 * @copyright Copyright (c) 2025, Ian Archbell
 * 
 * Header only implmentation of FileView.
 * 
 */

 #pragma once
 #include "View.h"
 #include "AppContext.h"
 #include "StorageManager.h"
 #include "HttpResponse.h"
 #include "FileHandle.h"
 
 class FileView : public View {
 public:
     FileView(const char* path, const char* contentType = "text/html", bool asDownload = false)
         : filePath(path), mimeType(contentType), download(asDownload) {}
 
     void render(HttpRequest& req, HttpResponse& res) override {
         StorageManager* storage = AppContext::get<StorageManager>();
         if (!storage) {
             res.status(500).text("Storage unavailable");
             return;
         }
 
         FileHandle file;
         if (!storage->open(filePath, FileOpenMode::Read, file)) {
             res.status(404).text("File not found");
             return;
         }
 
         res.setContentType(mimeType);
         if (download) {
             const char* filename = strrchr(filePath, '/');
             filename = filename ? filename + 1 : filePath;
             res.setHeader("Content-Disposition", std::string("attachment; filename=\"") + filename + "\"");
         }
 
         res.sendFile(file);
     }
 
 private:
     const char* filePath;
     const char* mimeType;
     bool download;
 };
 