#include "LittleFsStorageManager.h"
#include "hardware/flash.h"      // for flash_range_program, flash_range_erase
#include "hardware/sync.h"       // for save_and_disable_interrupts, restore_interrupts
#include "pico/multicore.h"      // for multicore_lockout_start_blocking / end_blocking
#include "hardware/regs/addressmap.h" // for XIP_BASE if not already defined
#include <cstring>
#include <iostream>

extern "C" {
    extern uint8_t __flash_lfs_start;
    extern uint8_t __flash_lfs_end;
}

LittleFsStorageManager::LittleFsStorageManager() {
    configure();
}

int LittleFsStorageManager::lfs_read_cb(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
                                        void* buffer, lfs_size_t size) {
    auto* self = static_cast<LittleFsStorageManager*>(c->context);
    uintptr_t addr = self->flashBase + block * c->block_size + off;
    std::memcpy(buffer, reinterpret_cast<const void*>(addr), size);
    return 0;
}

// int LittleFsStorageManager::lfs_prog_cb(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
//                                         const void* buffer, lfs_size_t size) {
//     auto* self = static_cast<LittleFsStorageManager*>(c->context);
//     uintptr_t addr = self->flashBase + block * c->block_size + off;
//     printf("[LFS PROG] block=%lu off=%lu size=%lu addr=0x%08lx\n",
//         (unsigned long)block, (unsigned long)off, (unsigned long)size,
//         (unsigned long)(self->flashBase + block * c->block_size + off));
//     flash_range_program(addr - XIP_BASE, reinterpret_cast<const uint8_t*>(buffer), size);
//     return 0;
// }

int LittleFsStorageManager::lfs_prog_cb(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
                                        const void* buffer, lfs_size_t size) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(c->context) + block * c->block_size + off;
    uintptr_t offset = addr - XIP_BASE;

#if defined(PICO_MULTICORE)
    multicore_lockout_start_blocking();
#endif

    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(offset, reinterpret_cast<const uint8_t*>(buffer), size);
    restore_interrupts(ints);

#if defined(PICO_MULTICORE)
    multicore_lockout_end_blocking();
#endif

    return 0;
}


// int LittleFsStorageManager::lfs_erase_cb(const struct lfs_config* c, lfs_block_t block) {
//     auto* self = static_cast<LittleFsStorageManager*>(c->context);
//     uintptr_t addr = self->flashBase + block * c->block_size;
//     flash_range_erase(addr - XIP_BASE, c->block_size);
//     return 0;
// }

int LittleFsStorageManager::lfs_erase_cb(const struct lfs_config* c, lfs_block_t block) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(c->context) + block * c->block_size;
    uintptr_t offset = addr - XIP_BASE;

#if defined(PICO_MULTICORE)
    multicore_lockout_start_blocking();
#endif

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(offset, c->block_size);
    restore_interrupts(ints);

#if defined(PICO_MULTICORE)
    multicore_lockout_end_blocking();
#endif

    return 0;
}


extern "C" {
    extern uint8_t __flash_lfs_start;
    extern uint8_t __flash_lfs_end;
}

void LittleFsStorageManager::configure() {
    flashBase = reinterpret_cast<uintptr_t>(&__flash_lfs_start);
    flashSize = reinterpret_cast<uintptr_t>(&__flash_lfs_end) - flashBase;

    std::memset(&config, 0, sizeof(config));

    config.context = this;
    config.read = lfs_read_cb;
    config.prog = lfs_prog_cb;
    config.erase = lfs_erase_cb;
    config.sync = [](const struct lfs_config*) -> int { return 0; };

    config.read_size = 256;
    config.prog_size = 256;
    config.block_size = 4096;
    config.block_count = flashSize / config.block_size;
    config.cache_size = 256;
    config.lookahead_size = 256;
    config.block_cycles = 500;
    config.compact_thresh = (lfs_size_t)-1;

    printf("[LittleFS] Flash base: 0x%08x, size: %zu bytes (%zu blocks)\n",
           static_cast<unsigned>(flashBase), flashSize, config.block_count);
}

bool LittleFsStorageManager::mount() {
    int err = lfs_mount(&lfs, &config);
    if (err) {
        printf("Mount failed err: %d, erasing and formatting...\n", err);
        flash_range_erase(flashBase - XIP_BASE, flashSize);  // 🔥 THIS IS THE FIX
        printf("[LittleFS] Erased %zu bytes at 0x%08lx before format\n", flashSize, flashBase);

        err = lfs_format(&lfs, &config);
        printf("Formatting complete, remounting...\n");

        err = lfs_mount(&lfs, &config);
    }
    mounted = (err == 0);
    printf("Mount %s\n", mounted ? "successful" : "failed");
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
    int closed = lfs_file_close(&lfs, &file);
    return (written == static_cast<int>(data.size())) && (closed == 0);
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
