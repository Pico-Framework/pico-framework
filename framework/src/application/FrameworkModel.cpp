/**
 * @file FrameworkModel.h
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

#include "FrameworkModel.h"

/// @copydoc FrameworkModel::FrameworkModel
FrameworkModel::FrameworkModel(StorageManager *storage, const std::string &path)
    : jsonService(storage), storagePath(path) {}

/// @copydoc FrameworkModel::load
bool FrameworkModel::load()
{
    if (!jsonService.load(storagePath))
        return false;
    collection = jsonService.data().value("items", nlohmann::json::array());
    return true;
}

/// @copydoc FrameworkModel::save
bool FrameworkModel::save()
{
    jsonService.data()["items"] = collection;
    return jsonService.save(storagePath);
}

/// @copydoc FrameworkModel::all
std::vector<nlohmann::json> FrameworkModel::all() const
{
    std::vector<nlohmann::json> items;
    for (const auto &item : collection)
    {
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
