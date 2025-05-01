/**
 * @file JsonService.cpp
 * @author Ian Archbell
 * @brief Implementation of JsonService for loading, saving, and manipulating persistent JSON data.
 *
 * Part of the PicoFramework application framework.
 * Uses a StorageManager to load and store JSON documents on persistent storage.
 * Supports fallback merge with default data and aliases for ease of use.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(JsonService)

#include "storage/JsonService.h"
#include <cstdio>
#include <cstdint>
#include <vector>

/// @copydoc mergeDefaults
nlohmann::json mergeDefaults(const nlohmann::json &target, const nlohmann::json &defaults)
{
    nlohmann::json result = target;

    for (auto it = defaults.begin(); it != defaults.end(); ++it)
    {
        const std::string &key = it.key();

        if (!result.contains(key))
        {
            result[key] = it.value();
        }
        else if (result[key].is_object() && it.value().is_object())
        {
            result[key] = mergeDefaults(result[key], it.value());
        }
    }

    return result;
}

/// @copydoc JsonService::JsonService
JsonService::JsonService(StorageManager *storage)
    : storage(storage) {}

/// @copydoc JsonService::load
bool JsonService::load(const std::string &path)
{
    std::vector<uint8_t> buffer;
    if (!storage)
    return false;

    if(!storage->isMounted())
    {
       storage->mount();
    }
    
    if (!storage->readFile(path, buffer))
    {
        TRACE("Failed to read JSON file: %s\n", path.c_str());
        return false;
    }

    std::string str(buffer.begin(), buffer.end());
    printf("Loaded JSON file from %s: %zu bytes\n", path.c_str(), str.size());
    printf("Content: %s\n", str.c_str());

    // Treat empty file as valid empty object
    if (str.empty())
    {
        data_ = nlohmann::json::object();
        return true;
    }

    data_ = nlohmann::json::parse(str, nullptr, false); // No exceptions
    return !data_.is_discarded();
}

/// @copydoc JsonService::save
bool JsonService::save(const std::string &path) const
{
    if (!storage)
        return false;
    if(!storage->isMounted())
    {
        storage->mount();
    }
    std::string content = data_.dump(2); // Pretty print
    std::vector<uint8_t> buffer(content.begin(), content.end());
    printf("Buffer size: %zu bytes\n", buffer.size());
    printf("Buffer content: %s\n", content.c_str());

    bool ok = storage->writeFile(path, buffer);
    printf("Saved JSON file to %s: %s\n", path.c_str(), ok ? "ok" : "failed");
    return ok;
}

/// @copydoc JsonService::data
nlohmann::json &JsonService::data()
{
    return data_;
}

/// @copydoc JsonService::data const
const nlohmann::json &JsonService::data() const
{
    return data_;
}

/// @copydoc JsonService::root
nlohmann::json &JsonService::root()
{
    return data_;
}

/// @copydoc JsonService::root const
const nlohmann::json &JsonService::root() const
{
    return data_;
}

/// @copydoc JsonService::operator*
nlohmann::json &JsonService::operator*()
{
    return data_;
}

/// @copydoc JsonService::operator* const
const nlohmann::json &JsonService::operator*() const
{
    return data_;
}

bool JsonService::hasValidData() const {
    return !data_.is_discarded();
}
