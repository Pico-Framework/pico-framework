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

#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <vector>
#include <optional>
#include "storage/JsonService.h"
#include "storage/StorageManager.h"
#include "framework/AppContext.h"

/**
 * @brief Provides a basic JSON-backed record model.
 *
 * FrameworkModel abstracts away JSON loading/saving and record manipulation.
 * Subclass it to represent specific collections, optionally overriding `getIdField()`
 * if the record ID is not `"id"`.
 */
class FrameworkModel
{
public:
    /**
     * @brief Constructor.
     *
     * @param storage Pointer to a StorageManager (e.g. SD, flash).
     * @param path Path to the backing JSON file.
     */
    FrameworkModel(const std::string &path);

    /**
     * @brief Loads the JSON collection from storage.
     *
     * @return true if successful.
     */
    bool load();

    /**
     * @brief Saves the current collection to storage.
     *
     * @return true if successful.
     */
    bool save();

    /**
     * @brief Returns all items in the collection.
     */
    std::vector<nlohmann::json> all() const;

    /**
     * @brief Finds an item by ID.
     *
     * @param id The value of the ID field to match.
     * @return The matching JSON object, or nullopt if not found.
     */
    std::optional<nlohmann::json> find(const std::string &id) const;

    /**
     * @brief Adds a new item to the collection.
     *
     * @param item JSON object to insert.
     * @return true if added successfully; false if missing ID or already exists.
     */
    bool create(const nlohmann::json &item);

    /**
     * @brief Updates an item by ID.
     *
     * @param id The ID of the item to update.
     * @param updatedItem The new item JSON to replace the old one.
     * @return true if update succeeded; false if not found.
     */
    bool update(const std::string &id, const nlohmann::json &updatedItem);

    /**
     * @brief Removes an item by ID.
     *
     * @param id The ID of the item to remove.
     * @return true if removed; false if not found.
     */
    bool remove(const std::string &id);

    /// @brief Returns the full collection as a JSON array object
    nlohmann::json toJson() const
    {
        return nlohmann::json(collection);
    }

    /// @brief Finds a single item by ID and returns it as JSON or null
    nlohmann::json findAsJson(const std::string &id) const;

    /// @brief Saves a single item by ID and JSON object
    bool save(const std::string &id, const json &data);

    bool createFromJson(const nlohmann::json &obj);

    bool updateFromJson(const std::string &id, const nlohmann::json &updates);

    nlohmann::json deleteAsJson(const std::string &id);

    bool saveAll();

    /**
     * @brief Reads a single top-level key from the model file.
     * This is separate from array-style records and useful for persistent app state.
     *
     * @tparam T Expected return type
     * @param key JSON key name
     * @param defaultValue Returned if key is not present
     * @return Value from JSON or default
     */
    template <typename T>
    T getValue(const std::string &key, const T &defaultValue = T()) 
    {
        jsonService = AppContext::get<JsonService>();
        if(!jsonService)
        {
            printf("[FrameworkModel] JsonService not available\n");
            return defaultValue;
        }
        const auto &data = jsonService->data();
        if (data.contains(key))
        {
            return data[key].get<T>();
        }
        return defaultValue;
    }

    /**
     * @brief Sets a single top-level key in the model file.
     * This does not affect the array-style record collection.
     *
     * @tparam T Type of value to store
     * @param key JSON key
     * @param value Value to assign
     */
    template <typename T>
    void setValue(const std::string &key, const T &value)
    {
        jsonService = AppContext::get<JsonService>();
        if(!jsonService)
        {
            printf("[FrameworkModel] JsonService not available\n");
            return;
        }
        jsonService->data()[key] = value;
    }


protected:
    /**
     * @brief Returns the JSON key used as the record ID.
     *
     * Defaults to `"id"`, but can be overridden by subclasses.
     */
    virtual std::string getIdField() const { return "id"; }

    nlohmann::json collection = nlohmann::json::array(); ///< In-memory array of records

private:
    JsonService* jsonService = nullptr; ///< Underlying JSON persistence layer       
    std::string storagePath; ///< File path for this model
};
