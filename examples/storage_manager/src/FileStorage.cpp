#include "FileStorage.h"
#include "storage/StorageManager.h"
#include "framework/AppContext.h"

nlohmann::json FileStorage::listFiles(const std::string& path) {
    nlohmann::json result = nlohmann::json::array(); // <-- ARRAY not object

    auto storage = AppContext::get<StorageManager>();
    if (!storage) {
        result.push_back({{"error", "StorageManager not available"}});
        return result;
    }

    std::vector<FileInfo> files;
    if (storage->listDirectory(path, files)) {
        for (const auto& file : files) {
            result.push_back({
                {"name", file.name},
                {"size", file.size}
            });
        }
    } else {
        result.push_back({{"error", "Failed to list directory"}});
    }

    return result;
}

bool FileStorage::deleteFile(const std::string& filename) {
    auto storage = AppContext::get<StorageManager>();
    if (!storage) {
        return false;
    }
    return storage->remove(filename);
}

bool FileStorage::format() {
    auto storage = AppContext::get<StorageManager>();
    if (!storage) {
        return false;
    }
    return storage->formatStorage();
}
