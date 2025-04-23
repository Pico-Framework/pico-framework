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

    bool mount() override;
    bool unmount() override;

    bool exists(const std::string &path) override;
    bool remove(const std::string &path) override;
    bool rename(const std::string &from, const std::string &to) override;

    bool readFile(const std::string &path, std::vector<uint8_t> &buffer) override;

    /**
     * @brief Read a file string into a memory buffer.
     * @param path Path to the file.
     * @param startPosition Start position in the file.
     * @param length Length of data to read.
     * @param buffer Output string to fill with data.
     * @return true if successful.
     */
    bool readFileString(const std::string &path, uint32_t startPosition, uint32_t length, std::string &buffer);

    bool writeFile(const std::string &path, const std::vector<uint8_t> &data) override;

    bool streamFile(const std::string &path, std::function<void(const uint8_t *, size_t)> chunkCallback) override;

    bool listDirectory(const std::string &path, std::vector<FileInfo> &out) override;

    bool createDirectory(const std::string &path) override;

    bool removeDirectory(const std::string &path) override;

    size_t getFileSize(const std::string &path) override;

    bool appendToFile(const std::string &path, const uint8_t *data, size_t size) override;

    bool isMounted() const override;

    bool formatStorage() override;

private:
    bool mounted = false;           ///< Indicates if the filesystem is currently mounted
    std::string mountPoint = "sd0"; ///< Default mount point

    SemaphoreHandle_t mutex; ///< Optional lock for thread safety

    std::string resolvePath(const std::string &path) const;

    bool ensureMounted();

    bool probeMountPoint(); // Check if the mount point is valid

    void refreshMountState();
};
