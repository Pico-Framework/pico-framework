/**
 * @file LittleFsStorageManager.h
 * @author Ian Archbell
 * @brief Flash-backed implementation of the StorageManager interface using LittleFS.
 * @version 0.1
 * @date 2025-04-14
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#pragma once

#include "StorageManager.h"
#include "lfs.h"
#include <vector>
#include <string>
#include <FreeRTOS.h> // for FreeRTOS types and functions
#include <semphr.h>   // for SemaphoreHandle_t, StaticSemaphore_t, xSemaphoreCreateMutexStatic, xSemaphoreTake, xSemaphoreGive

/**
 * @brief A LittleFS-based implementation of StorageManager, storing files in flash memory.
 *
 * Automatically mounts on first access if not already mounted.
 */
class LittleFsStorageManager : public StorageManager
{
public:
    /**
     * @brief Construct the manager and configure the filesystem.
     */
    LittleFsStorageManager();

    /// @brief open a file for streaming read line access.
    std::unique_ptr<StorageFileReader> openReader(const std::string& path) override;

    /**
     * @brief Mount the LittleFS filesystem.
     * @return true if mounted successfully.
     */
    bool mount() override;

    /**
     * @brief Unmount the LittleFS filesystem.
     * @return true if unmounted successfully.
     */
    bool unmount() override;

    /**
     * @brief Check if the filesystem is mounted.
     * @return true if mounted.
     */
    bool isMounted() const override;

    /**
     * @brief Check if a file or directory exists.
     * @param path Path to the file or directory.
     * @return true if it exists.
     */
    bool exists(const std::string &path) override;

    /**
     * @brief Remove a file or directory.
     * @param path Path to the file or directory.
     * @return true if removed.
     */
    bool remove(const std::string &path) override;

    /**
     * @brief Rename a file or directory.
     * @param from Source path.
     * @param to Destination path.
     * @return true if renamed.
     */
    bool rename(const std::string &from, const std::string &to) override;

    /**
     * @brief Read a file into a byte vector.
     * @param path Path to the file.
     * @param out Output vector with file contents.
     * @return true if successful.
     */
    bool readFile(const std::string &path, std::vector<uint8_t> &out) override;

    /**
     * @brief Read a file string into a memory buffer.
     * @param path Path to the file.
     * @param startPosition Start position in the file.
     * @param length Length of data to read.
     * @param buffer Output string to fill with data.
     * @return true if successful.
     */
    bool readFileString(const std::string &path, uint32_t startPosition, uint32_t length, std::string &buffer);

    /**
     * @brief Write a byte vector to a file (overwrite).
     * @param path Path to the file.
     * @param data Data to write.
     * @return true if successful.
     */
    bool writeFile(const std::string &path, const std::vector<uint8_t> &data) override;
    bool writeFile(const std::string& path, const unsigned char* data, size_t size) override;

    /**
     * @brief Append data to a file.
     * @param path Path to the file.
     * @param data Pointer to data.
     * @param size Size of data in bytes.
     * @return true if appended.
     */
    bool appendToFile(const std::string &path, const uint8_t *data, size_t size) override;

    /**
     * @brief Stream a file in chunks using a callback.
     * @param path Path to the file.
     * @param chunkCallback Callback to receive chunks.
     * @return true if streamed successfully.
     */
    bool streamFile(const std::string &path, std::function<void(const uint8_t *, size_t)> chunkCallback) override;

    /**
     * @brief Get the size of a file.
     * @param path Path to the file.
     * @return Size in bytes, or 0 on error.
     */
    size_t getFileSize(const std::string &path) override;

    /**
     * @brief List files in a directory.
     * @param path Path to directory.
     * @param out Vector to receive file entries.
     * @return true if listed successfully.
     */
    bool listDirectory(const std::string &path, std::vector<FileInfo> &out) override;

    /**
     * @brief Create a new directory.
     * @param path Path of directory.
     * @return true if created.
     */
    bool createDirectory(const std::string &path) override;

    /**
     * @brief Remove a directory.
     * @param path Path of directory.
     * @return true if removed.
     */
    bool removeDirectory(const std::string &path) override;

    /**
     * @brief Format the filesystem.
     * @return true if formatted successfully.
     */
    bool formatStorage() override;

    /**
     * get Flash base address
     * @return Flash base address
     */
    uintptr_t getFlashBase() const
    {
        return flashBase;
    }

    void formatInner(bool *result);

private:
    uintptr_t flashBase = 0;
    size_t flashSize = 0;

    static constexpr uint32_t FLASH_BASE = 0x101E0000;
    static constexpr size_t FLASH_SIZE = 128 * 1024; ///< 128 KB

    static constexpr size_t READ_SIZE = 256;
    static constexpr size_t PROG_SIZE = 256;
    static constexpr size_t BLOCK_SIZE = 4096;
    static constexpr size_t BLOCK_COUNT = FLASH_SIZE / BLOCK_SIZE;
    static constexpr size_t CACHE_SIZE = 256;
    static constexpr size_t LOOKAHEAD_SIZE = 256;

    lfs_t lfs;
    struct lfs_config config;

    bool mounted = false;

    // --- LittleFS Thread Safety Lock (FreeRTOS mutex, statically allocated) ---

    static StaticSemaphore_t lfs_mutex_buf;
    static SemaphoreHandle_t lfs_mutex;

    void configure();
    static int lfs_read_cb(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
    static int lfs_prog_cb(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
    static int lfs_erase_cb(const struct lfs_config *c, lfs_block_t block);
    static int lfs_prog_cb_singlecore(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
    static int lfs_erase_cb_singlecore(const struct lfs_config *c, lfs_block_t block);
    static int lfs_prog_cb_multicore(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
    static int lfs_erase_cb_multicore(const struct lfs_config *c, lfs_block_t block);
    static int lfs_lock(const struct lfs_config *c);
    static int lfs_unlock(const struct lfs_config *c);

    /**
     * @brief Mount automatically if not mounted yet.
     * @return true if mounted successfully or already mounted.
     */
    bool autoMountIfNeeded();
};
