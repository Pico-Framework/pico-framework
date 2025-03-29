#include "JsonService.h"
#include <cstdio>  // for printf
#include <cstdint>
#include <vector>

nlohmann::json mergeDefaults(const nlohmann::json& target, const nlohmann::json& defaults) {
    nlohmann::json result = target;

    for (auto it = defaults.begin(); it != defaults.end(); ++it) {
        const std::string& key = it.key();

        if (!result.contains(key)) {
            result[key] = it.value();
        } else if (result[key].is_object() && it.value().is_object()) {
            result[key] = mergeDefaults(result[key], it.value());
        }
    }

    return result;
}

JsonService::JsonService(StorageManager* storage)
    : storage(storage) {}

bool JsonService::load(const std::string& path) {
    std::vector<uint8_t> buffer;
    if (!storage || !storage->readFile(path, buffer)) {
        printf("JsonService: Failed to read %s\n", path.c_str());
        return false;
    }

    std::string content(buffer.begin(), buffer.end());
    nlohmann::json parsed = nlohmann::json::parse(content, nullptr, false);  // No exceptions

    if (parsed.is_discarded()) {
        printf("JsonService: Failed to parse JSON from %s\n", path.c_str());
        return false;
    }

    data_ = parsed;
    return true;
}

bool JsonService::save(const std::string& path) const {
    if (!storage) return false;

    std::string content = data_.dump(2);  // Pretty print
    std::vector<uint8_t> buffer(content.begin(), content.end());

    return storage->writeFile(path, buffer);
}

nlohmann::json& JsonService::data() {
    return data_;
}

const nlohmann::json& JsonService::data() const {
    return data_;
}

nlohmann::json& JsonService::root() {
    return data_;
}

const nlohmann::json& JsonService::root() const {
    return data_;
}

nlohmann::json& JsonService::operator*() {
    return data_;
}

const nlohmann::json& JsonService::operator*() const {
    return data_;
}