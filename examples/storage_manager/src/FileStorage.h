#pragma once

#include <string>
#include <nlohmann/json.hpp>

class FileStorage {
public:
    nlohmann::json listFiles(const std::string& path);
    bool deleteFile(const std::string& filename);
    bool format();
};
