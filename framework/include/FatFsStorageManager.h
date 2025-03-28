#ifndef FATFSSTORAGEMANAGER_H
#define FATFSSTORAGEMANAGER_H
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include "StorageManager.h"
#include "FreeRTOS.h"
#include "semphr.h"

class FatFsStorageManager : public StorageManager {
    public:

        FatFsStorageManager();

        bool mount() override;
        bool unmount() override;
    
        bool exists(const std::string& path) override;
        bool remove(const std::string& path) override;
        bool rename(const std::string& from, const std::string& to) override;
    
        bool readFile(const std::string& path, std::vector<uint8_t>& buffer) override;
        bool writeFile(const std::string& path, const std::vector<uint8_t>& data) override;
    
        bool streamFile(const std::string& path, std::function<void(const uint8_t*, size_t)> chunkCallback) override;
    
        bool listDirectory(const std::string& path, std::vector<FileInfo>& out) override;

        size_t getFileSize(const std::string& path);

        bool appendToFile(const std::string& path, const uint8_t* data, size_t size) override;

        private:
            bool mounted = false;  // Track if the filesystem is mounted
            std::string mountPoint = "sd0";  // Default mount point for the filesystem
            bool ensureMounted(){
                if (!mounted) {
                   mounted = mount();
                } 
                return mounted;  // Return the mount state
            }

            bool lock();            
            void unlock();

            SemaphoreHandle_t mutex;
    };

#endif // FATFSSTORAGEMANAGER_H