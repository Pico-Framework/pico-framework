#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H
#pragma once    


#include <string>
#include <vector>
#include <functional>
#include <cstdint>

struct FileInfo {
    std::string name;
    bool isDirectory;
    bool isReadOnly;
    size_t size;
};

class StorageManager {
    public:
        virtual bool mount() = 0;
        virtual bool unmount() = 0;
    
        virtual bool exists(const std::string& path) = 0;
        virtual bool remove(const std::string& path) = 0;
        virtual bool rename(const std::string& from, const std::string& to) = 0;
    
        virtual bool readFile(const std::string& path, std::vector<uint8_t>& buffer) = 0;
        virtual bool writeFile(const std::string& path, const std::vector<uint8_t>& data) = 0;
    
        virtual bool streamFile(const std::string& path, std::function<void(const uint8_t*, size_t)> chunkCallback) = 0;
    
        virtual bool listDirectory(const std::string& path, std::vector<FileInfo>& out) = 0;

        virtual size_t getFileSize(const std::string& path) = 0;

        virtual bool appendToFile(const std::string& path, const uint8_t* data, size_t size) = 0;
   
        virtual ~StorageManager() = default;
    };

#endif // STORAGE_MANAGER_H    