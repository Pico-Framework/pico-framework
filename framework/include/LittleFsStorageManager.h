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
 
 /**
  * @brief A LittleFS-based implementation of StorageManager, storing files in flash memory.
  * 
  * Automatically mounts on first access if not already mounted.
  */
 class LittleFsStorageManager : public StorageManager {
 public:
     /**
      * @brief Construct the manager and configure the filesystem.
      */
     LittleFsStorageManager();
 
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
     bool exists(const std::string& path) override;
 
     /**
      * @brief Remove a file or directory.
      * @param path Path to the file or directory.
      * @return true if removed.
      */
     bool remove(const std::string& path) override;
 
     /**
      * @brief Rename a file or directory.
      * @param from Source path.
      * @param to Destination path.
      * @return true if renamed.
      */
     bool rename(const std::string& from, const std::string& to) override;
 
     /**
      * @brief Read a file into a byte vector.
      * @param path Path to the file.
      * @param out Output vector with file contents.
      * @return true if successful.
      */
     bool readFile(const std::string& path, std::vector<uint8_t>& out) override;
 
     /**
      * @brief Write a byte vector to a file (overwrite).
      * @param path Path to the file.
      * @param data Data to write.
      * @return true if successful.
      */
     bool writeFile(const std::string& path, const std::vector<uint8_t>& data) override;
 
     /**
      * @brief Append data to a file.
      * @param path Path to the file.
      * @param data Pointer to data.
      * @param size Size of data in bytes.
      * @return true if appended.
      */
     bool appendToFile(const std::string& path, const uint8_t* data, size_t size) override;
 
     /**
      * @brief Stream a file in chunks using a callback.
      * @param path Path to the file.
      * @param chunkCallback Callback to receive chunks.
      * @return true if streamed successfully.
      */
     bool streamFile(const std::string& path, std::function<void(const uint8_t*, size_t)> chunkCallback) override;
 
     /**
      * @brief Get the size of a file.
      * @param path Path to the file.
      * @return Size in bytes, or 0 on error.
      */
     size_t getFileSize(const std::string& path) override;
 
     /**
      * @brief List files in a directory.
      * @param path Path to directory.
      * @param out Vector to receive file entries.
      * @return true if listed successfully.
      */
     bool listDirectory(const std::string& path, std::vector<FileInfo>& out) override;
 
     /**
      * @brief Create a new directory.
      * @param path Path of directory.
      * @return true if created.
      */
     bool createDirectory(const std::string& path) override;
 
     /**
      * @brief Remove a directory.
      * @param path Path of directory.
      * @return true if removed.
      */
     bool removeDirectory(const std::string& path) override;
 
 private:
     static constexpr size_t READ_SIZE = 256;
     static constexpr size_t PROG_SIZE = 256;
     static constexpr size_t BLOCK_SIZE = 4096;
     static constexpr size_t BLOCK_COUNT = 32;
     static constexpr size_t CACHE_SIZE = 256;
     static constexpr size_t LOOKAHEAD_SIZE = 16;
 
     lfs_t lfs;
     struct lfs_config config;
 
     uint32_t flashBase = 0;
     uint32_t flashSize = 0;
     bool mounted = false;
 
     void configure();
     static int lfs_read_cb(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size);
     static int lfs_prog_cb(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size);
     static int lfs_erase_cb(const struct lfs_config* c, lfs_block_t block);
 
     /**
      * @brief Mount automatically if not mounted yet.
      * @return true if mounted successfully or already mounted.
      */
     bool autoMountIfNeeded();
 };
 