/**
 * @file JsonService.h
 * @author Ian Archbell
 * @brief Lightweight JSON wrapper for persistent config management using StorageManager.
 *
 * Part of the PicoFramework application framework.
 * Provides convenience access to a JSON document that can be loaded from and saved to storage.
 * Also supports aliases and direct access to internal `nlohmann::json` object.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #pragma once

 #include <string>
 #include <vector>
 #include "StorageManager.h"
 #include "nlohmann/json.hpp"
 
 /**
  * @class JsonService
  * @brief Manages loading and saving of a single JSON document using StorageManager.
  */
 class JsonService {
 public:
     /**
      * @brief Construct a new JsonService.
      * @param storage Pointer to a StorageManager for persistent access.
      */
     JsonService(StorageManager* storage);
 
     /**
      * @brief Load a JSON file from storage.
      * @param path File path to load from.
      * @return true if load was successful and JSON parsed correctly.
      */
     bool load(const std::string& path);
 
     /**
      * @brief Save the current JSON data to storage.
      * @param path File path to save to.
      * @return true if save was successful.
      */
     bool save(const std::string& path) const;
 
     /**
      * @brief Access the internal JSON object.
      */
     nlohmann::json& data();
 
     /**
      * @brief Const access to the internal JSON object.
      */
     const nlohmann::json& data() const;
 
     /**
      * @brief Alias for data().
      */
     nlohmann::json& root();
 
     /**
      * @brief Const alias for data().
      */
     const nlohmann::json& root() const;
 
     /**
      * @brief Operator alias for data().
      */
     nlohmann::json& operator*();
 
     /**
      * @brief Const operator alias for data().
      */
     const nlohmann::json& operator*() const;
 
 private:
     StorageManager* storage;
     nlohmann::json data_;
 };
 
 /**
  * @brief Merge a default JSON structure into a target, preserving existing keys.
  *
  * For nested objects, recursion is applied. Used to safely inject default values into config data.
  *
  * @param target Target JSON object to merge into.
  * @param defaults Default values to apply where keys are missing.
  * @return A new merged JSON object.
  */
 nlohmann::json mergeDefaults(const nlohmann::json& target, const nlohmann::json& defaults);
 