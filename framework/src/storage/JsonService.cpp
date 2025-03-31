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

 #include "JsonService.h"
 #include <cstdio>
 #include <cstdint>
 #include <vector>
 
 /// @copydoc mergeDefaults
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
 
 /// @copydoc JsonService::JsonService
 JsonService::JsonService(StorageManager* storage)
     : storage(storage) {}
 
 /// @copydoc JsonService::load
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
 
 /// @copydoc JsonService::save
 bool JsonService::save(const std::string& path) const {
     if (!storage) return false;
 
     std::string content = data_.dump(2);  // Pretty print
     std::vector<uint8_t> buffer(content.begin(), content.end());
 
     return storage->writeFile(path, buffer);
 }
 
 /// @copydoc JsonService::data
 nlohmann::json& JsonService::data() {
     return data_;
 }
 
 /// @copydoc JsonService::data const
 const nlohmann::json& JsonService::data() const {
     return data_;
 }
 
 /// @copydoc JsonService::root
 nlohmann::json& JsonService::root() {
     return data_;
 }
 
 /// @copydoc JsonService::root const
 const nlohmann::json& JsonService::root() const {
     return data_;
 }
 
 /// @copydoc JsonService::operator*
 nlohmann::json& JsonService::operator*() {
     return data_;
 }
 
 /// @copydoc JsonService::operator* const
 const nlohmann::json& JsonService::operator*() const {
     return data_;
 }
 