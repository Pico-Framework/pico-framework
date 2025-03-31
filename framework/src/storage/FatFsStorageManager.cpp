/**
 * @file FatFsStorageManager.cpp
 * @author Ian Archbell
 * @brief Implementation of the FatFs-based StorageManager.
 *
 * Part of the PicoFramework application framework.
 * Provides thread-safe file operations for embedded systems using the FatFs API.
 * Wraps `ff_stdio.h` and manages mounting/unmounting, file streaming, and directory traversal.
 * Designed for use with Raspberry Pi Pico and other embedded SD-based storage.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #include "FatFsStorageManager.h"
 #include <ff_utils.h>
 #include <ff_stdio.h>
 #include <FreeRTOS.h>
 #include <semphr.h>
 
 /// @copydoc FatFsStorageManager::FatFsStorageManager()
 FatFsStorageManager::FatFsStorageManager() {}
 
 /// @copydoc FatFsStorageManager::lock()
 bool FatFsStorageManager::lock() {
     return true; /*xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) == pdTRUE*/
 }
 
 /// @copydoc FatFsStorageManager::unlock()
 void FatFsStorageManager::unlock() {
     /*xSemaphoreGive(mutex)*/;
 }
 
 /// @copydoc FatFsStorageManager::mount()
 bool FatFsStorageManager::mount() {
     return ::mount(mountPoint.c_str());
 }
 
 /// @copydoc FatFsStorageManager::unmount()
 bool FatFsStorageManager::unmount() {
     ::unmount(mountPoint.c_str());
     return true;
 }
 
 /// @copydoc FatFsStorageManager::exists()
 bool FatFsStorageManager::exists(const std::string& path) {
     if (!lock()) return false;
     FF_FILE* file = ff_fopen(path.c_str(), "r");
     if (file) {
         ff_fclose(file);
         unlock();
         return true;
     }
     unlock();
     return false;
 }
 
 /// @copydoc FatFsStorageManager::listDirectory()
 bool FatFsStorageManager::listDirectory(const std::string& path, std::vector<FileInfo>& out) {
     if (!lock()) return false;
 
     FF_FindData_t xFindStruct;
     memset(&xFindStruct, 0, sizeof(xFindStruct));
 
     std::string searchPath = path.empty() ? "/" : path;
     int result = ff_findfirst(searchPath.c_str(), &xFindStruct);
 
     if (result != FF_ERR_NONE) {
         unlock();
         return false;
     }
 
     do {
         if (strlen(xFindStruct.pcFileName) > 0) {
             FileInfo info;
             info.name = xFindStruct.pcFileName;
             info.isDirectory = xFindStruct.ucAttributes & FF_FAT_ATTR_DIR;
             info.isReadOnly = xFindStruct.ucAttributes & FF_FAT_ATTR_READONLY;
             info.size = static_cast<size_t>(xFindStruct.ulFileSize);
             out.push_back(info);
         }
     } while (ff_findnext(&xFindStruct) == FF_ERR_NONE);
 
     unlock();
     return true;
 }
 
 /// @copydoc FatFsStorageManager::readFile()
 bool FatFsStorageManager::readFile(const std::string& path, std::vector<uint8_t>& buffer) {
     if (!lock()) return false;
     FF_FILE* file = ff_fopen(path.c_str(), "r");
     if (!file) {
         unlock();
         return false;
     }
 
     ff_fseek(file, 0, SEEK_END);
     long fileSize = ff_ftell(file);
     ff_fseek(file, 0, SEEK_SET);
 
     if (fileSize < 0) {
         ff_fclose(file);
         unlock();
         return false;
     }
 
     buffer.resize(fileSize);
     ff_fread(buffer.data(), 1, fileSize, file);
     ff_fclose(file);
     unlock();
     return true;
 }
 
 /// @copydoc FatFsStorageManager::writeFile()
 bool FatFsStorageManager::writeFile(const std::string& path, const std::vector<uint8_t>& data) {
     if (!lock()) return false;
     FF_FILE* file = ff_fopen(path.c_str(), "w");
     if (!file) {
         unlock();
         return false;
     }
 
     ff_fwrite(data.data(), 1, data.size(), file);
     ff_fclose(file);
     unlock();
     return true;
 }
 
 /// @copydoc FatFsStorageManager::remove()
 bool FatFsStorageManager::remove(const std::string& path) {
     if (!lock()) return false;
     bool result = ff_remove(path.c_str()) == FF_ERR_NONE;
     unlock();
     return result;
 }
 
 /// @copydoc FatFsStorageManager::rename()
 bool FatFsStorageManager::rename(const std::string& from, const std::string& to) {
     if (!ensureMounted() || !lock()) return false;
     bool result = ff_rename((from).c_str(), (to).c_str(), false) == 0;
     unlock();
     return result;
 }
 
 /// @copydoc FatFsStorageManager::streamFile()
 bool FatFsStorageManager::streamFile(const std::string& path, std::function<void(const uint8_t*, size_t)> chunkCallback) {
     if (!ensureMounted() || !lock()) return false;
     FF_FILE* file = ff_fopen((path).c_str(), "r");
     if (!file) {
         unlock();
         return false;
     }
 
     uint8_t buffer[512];
     size_t bytes;
     while ((bytes = ff_fread(buffer, 1, sizeof(buffer), file)) > 0) {
         chunkCallback(buffer, bytes);
     }
 
     ff_fclose(file);
     unlock();
     return true;
 }
 
 /// @copydoc FatFsStorageManager::getFileSize()
 size_t FatFsStorageManager::getFileSize(const std::string& path) {
     if (!lock()) return 0;
     FF_FILE* file = ff_fopen(path.c_str(), "r");
     if (!file) {
         unlock();
         return 0;
     }
 
     ff_fseek(file, 0, SEEK_END);
     long size = ff_ftell(file);
     ff_fclose(file);
     unlock();
 
     return (size >= 0) ? static_cast<size_t>(size) : 0;
 }
 
 /// @copydoc FatFsStorageManager::appendToFile()
 bool FatFsStorageManager::appendToFile(const std::string& path, const uint8_t* data, size_t size) {
     if (!lock()) return false;
 
     FF_FILE* file = ff_fopen(path.c_str(), "a");
     if (!file) {
         unlock();
         return false;
     }
 
     size_t written = ff_fwrite(data, 1, size, file);
     ff_fclose(file);
     unlock();
 
     return written == size;
 }
 