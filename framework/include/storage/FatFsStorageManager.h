/**
 * @file FatFsStorageManager.h
 * @author Ian Archbell
 * @brief FatFs-based implementation of the StorageManager interface for SD card access.
 *
 * Part of the PicoFramework application framework.
 * Wraps FreeRTOS and ff_stdio calls to provide thread-safe file and directory access.
 * Designed for embedded use on platforms such as Raspberry Pi Pico with SD card support.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include "StorageManager.h"
#include "FreeRTOS.h"
#include "semphr.h"

/**
 * @class FatFsStorageManager
 * @brief Concrete implementation of StorageManager using FatFs.
 */
class FatFsStorageManager : public StorageManager
{
public:
    /**
     * @brief Construct a new FatFsStorageManager object.
     */
    FatFsStorageManager();

    /**
     * @brief Mount the filesystem at the specified mount point.
     * @return true if successful, false otherwise.
     */
    bool mount() override;

    /**
     * @brief Unmount the filesystem.
     * @return true if successful, false otherwise.
     */
    bool unmount() override;

    /**
     * @brief Check if the path exists.
     * @return true if so, false otherwise.
     */
    bool exists(const std::string &path) override;

    /**
     * @brief Remove a file or directory at the specified path.
     * @return true if successful, false otherwise.
     * @note For directories, this will only succeed if the directory is empty.
     */
    bool remove(const std::string &path) override;

    /**
     * @brief Rename a file or directory.
     * @param from Current path of the file or directory.
     * @param to New path for the file or directory.
     * @return true if successful, false otherwise.
     * @note This will fail if the destination already exists.
     */
    bool rename(const std::string &from, const std::string &to) override;

    /**
     * @brief Read a file into a memory buffer.
     * @param path Path to the file.
     * @param buffer Output buffer to fill with data.
     * @return true if successful, false otherwise.
     */
    bool readFile(const std::string &path, std::vector<uint8_t> &buffer) override;

    /**
     * @brief Read a file string into a std::string.
     * @param path Path to the file.
     * @param startPosition Start position in the file.
     * @param length Length of data to read.
     * @param buffer Output string to fill with data.
     * @return true if successful.
     */
    bool readFileString(const std::string &path, uint32_t startPosition, uint32_t length, std::string &buffer);

    /**
     * @brief Write a memory buffer to a file.
     * @param path Path to the file.
     * @param data Data to write.
     * @return true if successful, false otherwise.
     */
    bool writeFile(const std::string &path, const std::vector<uint8_t> &data) override;

    /**
     * @brief Write raw data to a file.
     * @param path Path to the file.
     * @param data Pointer to the data buffer.
     * @param size Size of the data buffer.
     * @return true if successful, false otherwise.
     */
    bool writeFile(const std::string& path, const unsigned char* data, size_t size) override;

    /**
     * @brief Stream a file in chunks via callback.
     * @param path Path to the file.
     * @param chunkCallback Callback function to handle each chunk of data.
     * @return true if successful, false otherwise.
     * @note The callback will be called multiple times with chunks of data from the file.
     */
    bool streamFile(const std::string &path, std::function<void(const uint8_t *, size_t)> chunkCallback) override;

    /**
     * @brief List the contents of a directory.
     * @param path Path to the directory.
     * @param out Output vector to fill with FileInfo objects.
     * @return true if successful, false otherwise.
     */
    bool listDirectory(const std::string &path, std::vector<FileInfo> &out) override;

    /**
     * @brief Create a directory at the specified path.
     * @param path Path to create the directory.
     * @return true if successful, false otherwise.
     */
    bool createDirectory(const std::string &path) override;

    /**
     * @brief Remove a directory at the specified path.
     * @param path Path to the directory to remove.
     * @return true if successful, false otherwise.
     * @note This will only succeed if the directory is empty.
     */
    bool removeDirectory(const std::string &path) override;

    /**
     * @brief Get the size of a file.
     * @param path Path to the file.
     * @return Size of the file in bytes, or 0 if the file does not exist.
     */
    size_t getFileSize(const std::string &path) override;

    /**
     * @brief Append data to a file.
     * @param path Path to the file.
     * @param data Pointer to the data buffer.
     * @param size Size of the data buffer.
     * @return true if successful, false otherwise.
     * @note If the file does not exist, it will be created.
     * If the file exists, data will be appended to the end.
     * 
     */
    bool appendToFile(const std::string &path, const uint8_t *data, size_t size) override;

    /**
     * @brief Check if the filesystem is currently mounted.
     * @return true if mounted, false otherwise.
     * @note This can be used to check if the filesystem is ready for operations.
     */
    bool isMounted() const override;

    /**
     * @brief Format the storage device.
     * @return true if successful, false otherwise.
     * @note This will erase all data on the device and reinitialize the filesystem.
     */
    bool formatStorage() override;

    /**
     * @brief open a file for streaming read access.
     * @param path The file path
     * @return A new reader object, or nullptr on failure
     */
    std::unique_ptr<StorageFileReader> openReader(const std::string& path) override;


private:
    bool mounted = false;           ///< Indicates if the filesystem is currently mounted
    std::string mountPoint = "sd0"; ///< Default mount point

    SemaphoreHandle_t mutex; ///< Optional lock for thread safety

    std::string resolvePath(const std::string &path) const;

    bool ensureMounted();

    bool probeMountPoint(); // Check if the mount point is valid

    void refreshMountState();
};
