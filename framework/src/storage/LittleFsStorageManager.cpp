#include "LittleFsStorageManager.h"
#include "hardware/flash.h"
#include <cstring>
#include <iostream>

LittleFsStorageManager::LittleFsStorageManager() {
    configure();
}

void LittleFsStorageManager::configure() {
    config.context = this;
    config.read = [](const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
                     void* buffer, lfs_size_t size) -> int {
        uint32_t addr = FLASH_BASE + block * c->block_size + off;
        std::memcpy(buffer, (const void*)addr, size);
        return 0;
    };

    config.prog = [](const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
                     const void* buffer, lfs_size_t size) -> int {
        uint32_t addr = FLASH_BASE + block * c->block_size + off;
        flash_range_program(addr - XIP_BASE, reinterpret_cast<const uint8_t*>(buffer), size);
        return 0;
    };

    config.erase = [](const struct lfs_config* c, lfs_block_t block) -> int {
        uint32_t addr = FLASH_BASE + block * c->block_size;
        flash_range_erase(addr - XIP_BASE, c->block_size);
        return 0;
    };

    config.sync = [](const struct lfs_config*) -> int { return 0; };

    config.read_size = READ_SIZE;
    config.prog_size = PROG_SIZE;
    config.block_size = BLOCK_SIZE;
    config.block_count = BLOCK_COUNT;
    config.cache_size = CACHE_SIZE;
    config.lookahead_size = LOOKAHEAD_SIZE;
    config.block_cycles = 500;
}

bool LittleFsStorageManager::mount() {
    int err = lfs_mount(&lfs, &config);
    if (err) {
        lfs_format(&lfs, &config);
        err = lfs_mount(&lfs, &config);
    }
    mounted = (err == 0);
    return mounted;
}

bool LittleFsStorageManager::unmount() {
    if (mounted) {
        lfs_unmount(&lfs);
        mounted = false;
    }
    return true;
}

bool LittleFsStorageManager::isMounted() const {
    return mounted;
}

bool LittleFsStorageManager::exists(const std::string& path) {
    struct lfs_info info;
    return lfs_stat(&lfs, path.c_str(), &info) == 0;
}

bool LittleFsStorageManager::remove(const std::string& path) {
    return lfs_remove(&lfs, path.c_str()) == 0;
}

bool LittleFsStorageManager::rename(const std::string& from, const std::string& to) {
    return lfs_rename(&lfs, from.c_str(), to.c_str()) == 0;
}

bool LittleFsStorageManager::readFile(const std::string& path, std::vector<uint8_t>& out) {
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_RDONLY) < 0) return false;
    lfs_soff_t size = lfs_file_size(&lfs, &file);
    if (size < 0) {
        lfs_file_close(&lfs, &file);
        return false;
    }
    out.resize(size);
    int bytes = lfs_file_read(&lfs, &file, out.data(), size);
    lfs_file_close(&lfs, &file);
    return bytes == size;
}

bool LittleFsStorageManager::writeFile(const std::string& path, const std::vector<uint8_t>& data) {
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) < 0) return false;
    int written = lfs_file_write(&lfs, &file, data.data(), data.size());
    lfs_file_close(&lfs, &file);
    return written == (int)data.size();
}

bool LittleFsStorageManager::appendToFile(const std::string& path, const uint8_t* data, size_t size) {
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND) < 0) return false;
    int written = lfs_file_write(&lfs, &file, data, size);
    lfs_file_close(&lfs, &file);
    return written == (int)size;
}

bool LittleFsStorageManager::streamFile(const std::string& path, std::function<void(const uint8_t*, size_t)> chunkCallback) {
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_RDONLY) < 0) return false;
    uint8_t buf[64];
    int readBytes;
    while ((readBytes = lfs_file_read(&lfs, &file, buf, sizeof(buf))) > 0) {
        chunkCallback(buf, readBytes);
    }
    lfs_file_close(&lfs, &file);
    return true;
}

size_t LittleFsStorageManager::getFileSize(const std::string& path) {
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_RDONLY) < 0) return 0;
    lfs_soff_t size = lfs_file_size(&lfs, &file);
    lfs_file_close(&lfs, &file);
    return size < 0 ? 0 : size;
}

bool LittleFsStorageManager::listDirectory(const std::string& path, std::vector<FileInfo>& out) {
    lfs_dir_t dir;
    struct lfs_info info;

    if (lfs_dir_open(&lfs, &dir, path.c_str()) < 0) return false;

    while (lfs_dir_read(&lfs, &dir, &info) > 0) {
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) continue;
        FileInfo entry;
        entry.name = info.name;
        entry.size = info.size;
        entry.isDirectory = (info.type == LFS_TYPE_DIR);
        entry.isReadOnly = false;
        out.push_back(entry);
    }

    lfs_dir_close(&lfs, &dir);
    return true;
}

bool LittleFsStorageManager::createDirectory(const std::string& path) {
    return lfs_mkdir(&lfs, path.c_str()) == 0;
}

bool LittleFsStorageManager::removeDirectory(const std::string& path) {
    return lfs_remove(&lfs, path.c_str()) == 0;
}
