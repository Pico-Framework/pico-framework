/**
 * @file LittleFsStorageManager.cpp
 * @author Ian Archbell
 * @brief Flash-backed LittleFS implementation of StorageManager interface
 * @version 0.1
 * @date 2025-04-14
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "LittleFsStorageManager.h"
#include "AppContext.h"
#include "pico/stdlib.h"
#include <cstring>
#include <cstdio>

extern "C"
{
    extern uint8_t __flash_lfs_start;
    extern uint8_t __flash_lfs_end;
}
LittleFsStorageManager::LittleFsStorageManager()
{
    configure();
}

/**
 * @copydoc StorageManager::mount
 */
bool LittleFsStorageManager::mount()
{
    if (mounted)
        return true;
    int err = lfs_mount(&lfs, &config);
    mounted = (err == 0);
    printf("LittleFS mount %s\n", mounted ? "successful" : "failed");
    return mounted;
}

/**
 * @copydoc StorageManager::unmount
 */
bool LittleFsStorageManager::unmount()
{
    if (!mounted)
        return true;
    int err = lfs_unmount(&lfs);
    mounted = false;
    return (err == 0);
}

/**
 * @copydoc StorageManager::isMounted
 */
bool LittleFsStorageManager::isMounted() const
{
    return mounted;
}

/**
 * @copydoc StorageManager::exists
 */
bool LittleFsStorageManager::exists(const std::string &path)
{
    if (!autoMountIfNeeded())
        return false;
    struct lfs_info info;
    return lfs_stat(&lfs, path.c_str(), &info) == 0;
}

/**
 * @copydoc StorageManager::remove
 */
bool LittleFsStorageManager::remove(const std::string &path)
{
    if (!autoMountIfNeeded())
        return false;
    return lfs_remove(&lfs, path.c_str()) == 0;
}

/**
 * @copydoc StorageManager::rename
 */
bool LittleFsStorageManager::rename(const std::string &from, const std::string &to)
{
    if (!autoMountIfNeeded())
        return false;
    return lfs_rename(&lfs, from.c_str(), to.c_str()) == 0;
}

/**
 * @copydoc StorageManager::readFile
 */
bool LittleFsStorageManager::readFile(const std::string &path, std::vector<uint8_t> &out)
{
    if (!autoMountIfNeeded())
        return false;
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_RDONLY) < 0)
        return false;

    lfs_soff_t size = lfs_file_size(&lfs, &file);
    if (size < 0)
    {
        lfs_file_close(&lfs, &file);
        return false;
    }

    out.resize(size);
    lfs_file_read(&lfs, &file, out.data(), size);
    lfs_file_close(&lfs, &file);
    return true;
}

/**
 * @copydoc StorageManager::writeFile
 */
bool LittleFsStorageManager::writeFile(const std::string &path, const std::vector<uint8_t> &data)
{
    if (!autoMountIfNeeded())
        return false;
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) < 0)
        return false;
    lfs_file_write(&lfs, &file, data.data(), data.size());
    lfs_file_close(&lfs, &file);
    return true;
}

/**
 * @copydoc StorageManager::appendToFile
 */
bool LittleFsStorageManager::appendToFile(const std::string &path, const uint8_t *data, size_t size)
{
    if (!autoMountIfNeeded())
        return false;
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND) < 0)
        return false;
    lfs_file_write(&lfs, &file, data, size);
    lfs_file_close(&lfs, &file);
    return true;
}

/**
 * @copydoc StorageManager::streamFile
 */
bool LittleFsStorageManager::streamFile(const std::string &path, std::function<void(const uint8_t *, size_t)> chunkCallback)
{
    if (!autoMountIfNeeded())
        return false;
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_RDONLY) < 0)
        return false;

    uint8_t buffer[256];
    lfs_ssize_t bytesRead;
    while ((bytesRead = lfs_file_read(&lfs, &file, buffer, sizeof(buffer))) > 0)
    {
        chunkCallback(buffer, bytesRead);
    }

    lfs_file_close(&lfs, &file);
    return true;
}

/**
 * @copydoc StorageManager::getFileSize
 */
size_t LittleFsStorageManager::getFileSize(const std::string &path)
{
    if (!autoMountIfNeeded())
        return 0;
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_RDONLY) < 0)
        return 0;
    lfs_soff_t size = lfs_file_size(&lfs, &file);
    lfs_file_close(&lfs, &file);
    return size > 0 ? static_cast<size_t>(size) : 0;
}

/**
 * @copydoc StorageManager::listDirectory
 */
bool LittleFsStorageManager::listDirectory(const std::string &path, std::vector<FileInfo> &out)
{
    if (!autoMountIfNeeded())
        return false;
    lfs_dir_t dir;
    struct lfs_info info;

    if (lfs_dir_open(&lfs, &dir, path.c_str()) < 0)
        return false;

    while (lfs_dir_read(&lfs, &dir, &info) > 0)
    {
        if (std::string(info.name) == "." || std::string(info.name) == "..")
            continue;
        out.push_back({info.name, info.type == LFS_TYPE_DIR});
    }

    lfs_dir_close(&lfs, &dir);
    return true;
}

/**
 * @copydoc StorageManager::createDirectory
 */
bool LittleFsStorageManager::createDirectory(const std::string &path)
{
    if (!autoMountIfNeeded())
        return false;
    return lfs_mkdir(&lfs, path.c_str()) == 0;
}

/**
 * @copydoc StorageManager::removeDirectory
 */
bool LittleFsStorageManager::removeDirectory(const std::string &path)
{
    if (!autoMountIfNeeded())
        return false;
    return lfs_remove(&lfs, path.c_str()) == 0;
}

// Internal callbacks and setup

int LittleFsStorageManager::lfs_read_cb(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                                        void *buffer, lfs_size_t size)
{
    memcpy(buffer, reinterpret_cast<const uint8_t *>(c->context) + block * BLOCK_SIZE + off, size);
    return 0;
}

int LittleFsStorageManager::lfs_prog_cb(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                                        const void *buffer, lfs_size_t size)
{
    memcpy(reinterpret_cast<uint8_t *>(c->context) + block * BLOCK_SIZE + off, buffer, size);
    return 0;
}

int LittleFsStorageManager::lfs_erase_cb(const struct lfs_config *c, lfs_block_t block)
{
    memset(reinterpret_cast<uint8_t *>(c->context) + block * BLOCK_SIZE, 0xFF, BLOCK_SIZE);
    return 0;
}

void LittleFsStorageManager::configure()
{
    flashBase = reinterpret_cast<uintptr_t>(&__flash_lfs_start);
    flashSize = reinterpret_cast<uintptr_t>(&__flash_lfs_end) - flashBase;
    memset(&config, 0, sizeof(config));

    config.context = (void *)flashBase;
    config.read = lfs_read_cb;
    config.prog = lfs_prog_cb;
    config.erase = lfs_erase_cb;
    config.sync = nullptr;

    config.read_size = READ_SIZE;
    config.prog_size = PROG_SIZE;
    config.block_size = BLOCK_SIZE;
    config.block_count = BLOCK_COUNT;
    config.cache_size = CACHE_SIZE;
    config.lookahead_size = LOOKAHEAD_SIZE;
    config.block_cycles = -1;
}

/**
 * @brief Internal helper to auto-mount if not already mounted.
 * @return true if mounted or successfully mounted; false on failure
 */
bool LittleFsStorageManager::autoMountIfNeeded()
{
    if (!mounted)
    {
        printf("[LittleFS] Auto-mounting...\n");
        return mount();
    }
    return true;
}
