/**
 * @file FrameworkModel.cpp
 * @author Ian Archbell
 * @brief Base model class for persistent JSON collections.
 *
 * Part of the PicoFramework application framework.
 * This module provides the FrameworkModel class, which abstracts the
 * loading, saving, and manipulation of JSON records stored in an array format.
 * Provides CRUD operations for JSON array records stored using the JsonService abstraction.
 * Designed for embedded use with SD or flash-backed storage.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework/FrameworkModel.h"
#include "framework/AppContext.h"
#include "storage/StorageManager.h"
#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(FrameworkModel)

/// @copydoc FrameworkModel::FrameworkModel
FrameworkModel::FrameworkModel(const std::string& path)
    : storagePath(path) {}

/// @copydoc FrameworkModel::load
bool FrameworkModel::load()
{
    auto jsonService = AppContext::get<JsonService>();
    if(!jsonService)
    {
        printf("Failed to get JsonService\n");
        return false;
    }
    if(storagePath.empty())
    {
        printf("[FrameworkModel] No storage path set, cannot load data\n");
        return false;
    }
    TRACE("[FrameworkModel] Loading data from %s\n", storagePath.c_str());
    if (!jsonService->load(storagePath))
        return false;
    collection = jsonService->data().value("items", nlohmann::json::array());
    if (collection.empty())
    {
        TRACE("No items found in %s\n", storagePath.c_str());
        return false;
    }
    else{
        TRACE("collection: %s\n", collection.dump(4).c_str());
    }
    return true;
}

/// @copydoc FrameworkModel::save
bool FrameworkModel::save()
{
    auto jsonService = AppContext::get<JsonService>();
    jsonService->data()["items"] = collection;
    return jsonService->save(storagePath);
}

/// @copydoc FrameworkModel::all
std::vector<nlohmann::json> FrameworkModel::all() const
{
    if (collection.empty())
    {
        return {};
    }
    std::vector<nlohmann::json> items;
    for (const auto &item : collection)
    {
        TRACE("[FrameworkModel::all] Item: %s\n", item.dump(4).c_str());
        items.push_back(item);
    }
    return items;
}

/// @copydoc FrameworkModel::find
std::optional<nlohmann::json> FrameworkModel::find(const std::string &id) const
{
    std::string idField = getIdField();
    for (const auto &item : collection)
    {
        if (item.contains(idField) && item[idField] == id)
        {
            return item;
        }
    }
    return std::nullopt;
}

/// @copydoc FrameworkModel::create
bool FrameworkModel::create(const nlohmann::json &item)
{
    std::string idField = getIdField();
    if (!item.contains(idField))
        return false;
    std::string id = item[idField];
    if (find(id))
        return false;
    collection.push_back(item);
    return true;
}

/// @copydoc FrameworkModel::update
bool FrameworkModel::update(const std::string &id, const nlohmann::json &updatedItem)
{
    std::string idField = getIdField();
    for (auto &item : collection)
    {
        if (item.contains(idField) && item[idField] == id)
        {
            item = updatedItem;
            return true;
        }
    }
    return false;
}

/// @copydoc FrameworkModel::remove
bool FrameworkModel::remove(const std::string &id)
{
    std::string idField = getIdField();
    for (auto it = collection.begin(); it != collection.end(); ++it)
    {
        if (it->contains(idField) && (*it)[idField] == id)
        {
            collection.erase(it);
            return true;
        }
    }
    return false;
}

/// @copydoc FrameworkModel::findAsJson
nlohmann::json FrameworkModel::findAsJson(const std::string &id) const
{
    auto result = find(id);
    return result ? *result : nlohmann::json(nullptr);
}

/// @copydoc FrameworkModel::save (single record)
bool FrameworkModel::save(const std::string &id, const nlohmann::json &data)
{
    std::string idField = getIdField();

    for (auto &item : collection)
    {
        if (item.contains(idField) && item[idField] == id)
        {
            item = data;
            return save();
        }
    }

    // If not found, append
    collection.push_back(data);
    return save();
}

/// @copydoc FrameworkModel::createFromJson
bool FrameworkModel::createFromJson(const nlohmann::json &obj)
{
    std::string idField = getIdField();
    if (!obj.contains(idField))
        return false;
    return save(obj[idField], obj);
}

/// @copydoc FrameworkModel::updateFromJson
bool FrameworkModel::updateFromJson(const std::string &id, const nlohmann::json &updates)
{
    std::string idField = getIdField();
    for (auto &item : collection)
    {
        if (item.contains(idField) && item[idField] == id)
        {
            for (auto &el : updates.items())
            {
                item[el.key()] = el.value(); // Patch keys
            }
            return save(id, item);
        }
    }
    return false;
}

/// @copydoc FrameworkModel::deleteAsJson
nlohmann::json FrameworkModel::deleteAsJson(const std::string &id)
{
    std::string idField = getIdField();
    for (auto it = collection.begin(); it != collection.end(); ++it)
    {
        if (it->contains(idField) && (*it)[idField] == id)
        {
            nlohmann::json removed = *it;
            collection.erase(it);
            return saveAll() ? removed : nullptr;
        }
    }
    return nullptr;
}

/// @copydoc FrameworkModel::saveAll
bool FrameworkModel::saveAll()
{
    auto jsonService = AppContext::get<JsonService>();
    jsonService->data()["items"] = collection;
    return jsonService->save(storagePath);
}
