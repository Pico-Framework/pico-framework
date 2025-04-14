/**
 * @file LittleFsStorageManager.h
 * @author Ian Archbell
 * @brief Flash-backed LittleFS implementation of StorageManager interface
 * @version 0.1
 * @date 2025-04-12
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #pragma once

 #include "StorageManager.h"
 #include "lfs.h"
 
 /**
  * @brief Implements StorageManager using LittleFS and onboard flash memory.
  */
 class LittleFsStorageManager : public StorageManager
 {
 public:
     LittleFsStorageManager();
 
     bool mount() override;
     bool unmount() override;
     bool isMounted() const override;
 
     bool exists(const std::string &path) override;
     bool remove(const std::string &path) override;
     bool rename(const std::string &from, const std::string &to) override;
 
     bool readFile(const std::string &path, std::vector<uint8_t> &out) override;
     bool writeFile(const std::string &path, const std::vector<uint8_t> &data) override;
     bool appendToFile(const std::string &path, const uint8_t *data, size_t size) override;
 
     bool streamFile(const std::string &path, std::function<void(const uint8_t *, size_t)> chunkCallback) override;
 
     size_t getFileSize(const std::string &path) override;
 
     bool listDirectory(const std::string &path, std::vector<FileInfo> &out) override;
     bool createDirectory(const std::string &path) override;
     bool removeDirectory(const std::string &path) override;
 
 private:
 
     static int lfs_read_cb(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
         void* buffer, lfs_size_t size);
     static int lfs_prog_cb(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
         const void* buffer, lfs_size_t size);
     static int lfs_erase_cb(const struct lfs_config* c, lfs_block_t block);
 
 
     void configure();
     bool mounted = false;
     lfs_t lfs;
     lfs_config config;
 
     uintptr_t flashBase = 0;
     size_t flashSize = 0;
 
     static constexpr uint32_t FLASH_BASE = 0x101E0000;
     static constexpr size_t FLASH_SIZE = 128 * 1024; ///< 128 KB
     static constexpr size_t BLOCK_SIZE = 4096;
     static constexpr size_t BLOCK_COUNT = FLASH_SIZE / BLOCK_SIZE;
 
     static constexpr size_t READ_SIZE = 256;
     static constexpr size_t PROG_SIZE = 256;
     static constexpr size_t CACHE_SIZE = 256;
     static constexpr size_t LOOKAHEAD_SIZE = 256;
 };
 