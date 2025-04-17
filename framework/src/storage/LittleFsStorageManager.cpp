#include "LittleFsStorageManager.h"
#include "hardware/flash.h"           // for flash_range_program, flash_range_erase
#include "hardware/sync.h"            // for save_and_disable_interrupts, restore_interrupts
#include "pico/multicore.h"           // for multicore_lockout_start_blocking / end_blocking
#include "hardware/regs/addressmap.h" // for XIP_BASE if not already defined
#include <cstring>
#include <iostream>
#include <pico/flash.h> // for flash_safe_execute
#include "utility.h"    // for runtimeStats

extern "C"
{
    extern uint8_t __flash_lfs_start;
    extern uint8_t __flash_lfs_end;
}

LittleFsStorageManager::LittleFsStorageManager()
{
    configure();
}

int LittleFsStorageManager::lfs_read_cb(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                                        void *buffer, lfs_size_t size)
{
    auto *self = static_cast<LittleFsStorageManager *>(c->context);
    uintptr_t addr = self->flashBase + block * c->block_size + off;
    std::memcpy(buffer, reinterpret_cast<const void *>(addr), size);
    return 0;
}

// Public program callback that gets registered in config.erase
int LittleFsStorageManager::lfs_prog_cb(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                                        const void *buffer, lfs_size_t size)
{
    //printf("[LFS PROG] lfs_prog_cb\n");
#if (configNUM_CORES > 1)
    return lfs_prog_cb_multicore(c, block, off, buffer, size);
#else
    return lfs_prog_cb_singlecore(c, block, off, buffer, size);
#endif
}

int LittleFsStorageManager::lfs_prog_cb_singlecore(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                                                   const void *buffer, lfs_size_t size)
{
    auto *self = static_cast<LittleFsStorageManager *>(c->context);
    // printf("Self: %p\n", self);
    // printf("Number of cores: in cb_prog: %d\n", configNUM_CORES);
    uintptr_t addr = self->flashBase + block * c->block_size + off;
    // printf("Flash base: %p\n", self->flashBase);
    // printf("Addr: %p\n", addr);
    // printf("Buffer: %s\n", buffer);
    // printf("Starting to prog\n");
    // printf("[LFS PROG] block=%lu off=%lu size=%lu addr=0x%08lx\n",
    //        (unsigned long)block, (unsigned long)off, (unsigned long)size,
    //        (unsigned long)(self->flashBase + block * c->block_size + off));
    // printf("Buffer ptr: %p\n", buffer);
    // printf("First 8 bytes: %02x %02x %02x %02x %02x %02x %02x %02x\n",
    //        ((uint8_t *)buffer)[0], ((uint8_t *)buffer)[1], ((uint8_t *)buffer)[2], ((uint8_t *)buffer)[3],
    //        ((uint8_t *)buffer)[4], ((uint8_t *)buffer)[5], ((uint8_t *)buffer)[6], ((uint8_t *)buffer)[7]);
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(addr - XIP_BASE, reinterpret_cast<const uint8_t *>(buffer), size);
    restore_interrupts(ints);
    //printf("[LFS PROG] flash_range_program done\n");
    return 0;
}

struct FlashProgParams
{
    uint32_t addr;
    const uint8_t *data;
    size_t size;
};

static void __not_in_flash_func(flash_prog_callback)(void *p)
{
    auto *params = static_cast<FlashProgParams *>(p);
    flash_range_program(params->addr, params->data, params->size);
}

static int __not_in_flash_func(lfs_prog_multicore)(const struct lfs_config *c,
                                                   lfs_block_t block,
                                                   lfs_off_t off,
                                                   const void *buffer,
                                                   lfs_size_t size)
{
    auto *self = static_cast<LittleFsStorageManager *>(c->context);
    uintptr_t addr = self->getFlashBase() + block * c->block_size + off;

    FlashProgParams params = {
        .addr = addr - XIP_BASE,
        .data = static_cast<const uint8_t *>(buffer),
        .size = size};
    //printf("[LFS PROG] executing flash_safe_execute\n");
    int result = flash_safe_execute(flash_prog_callback, &params, 1000);
    return (result == PICO_OK) ? 0 : -1;
}

int LittleFsStorageManager::lfs_prog_cb_multicore(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    auto *self = static_cast<LittleFsStorageManager *>(c->context);
    uintptr_t addr = self->flashBase + block * c->block_size + off;
    // printf("Starting to prog\n");
    // printf("[LFS PROG] block=%lu off=%lu size=%lu addr=0x%08lx\n",
    //        (unsigned long)block, (unsigned long)off, (unsigned long)size,
    //        (unsigned long)(self->flashBase + block * c->block_size + off));
    return lfs_prog_multicore(c, block, off, buffer, size);
}

int LittleFsStorageManager::lfs_erase_cb(const struct lfs_config *c, lfs_block_t block)
{
#if (configNUM_CORES > 1)
    return lfs_erase_cb_multicore(c, block);
#else
    return lfs_erase_cb_singlecore(c, block);
#endif
}

int LittleFsStorageManager::lfs_erase_cb_singlecore(const struct lfs_config *c, lfs_block_t block)
{
    auto *self = static_cast<LittleFsStorageManager *>(c->context);
    // printf("[LFS ERASE] Erasing block %lu at 0x%08lx (size %lu)\n", block, self->flashBase + block * c->block_size, c->block_size);
    uintptr_t addr = self->flashBase + block * c->block_size;
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(addr - XIP_BASE, c->block_size);
    restore_interrupts(ints);
    // printf("[LFS ERASE] flash_range_erase done\n");
    return 0;
}

struct FlashEraseParams
{
    uint32_t addr;
    size_t size;
};

// RAM function that does the actual erase
static void __not_in_flash_func(flash_erase_callback)(void *p)
{
    auto *params = static_cast<FlashEraseParams *>(p);
    flash_range_erase(params->addr, params->size);
}

static int lfs_erase_cb_flashsafe(const struct lfs_config *c, lfs_block_t block)
{
    auto *self = static_cast<LittleFsStorageManager *>(c->context);
    // printf("[LFS ERASE] lfs_erase_cb_flashsafe\n");
    uintptr_t addr = self->getFlashBase() + block * c->block_size;

    FlashEraseParams eraseParams = {
        .addr = addr - XIP_BASE,
        .size = c->block_size};
    // printf("[LFS ERASE] executing flash_safe_execute\n");
    //runTimeStats();
    int result = flash_safe_execute(flash_erase_callback, &eraseParams, 1000);
    // printf("[LFS ERASE] flash_safe_execute result: %d\n", result);
    return (result == PICO_OK) ? 0 : -1;
}

// Public erase callback that gets registered in config.erase
int LittleFsStorageManager::lfs_erase_cb_multicore(const struct lfs_config *c, lfs_block_t block)
{
    auto *self = static_cast<LittleFsStorageManager *>(c->context);
    // printf("[LFS ERASE] lfs_erase_cb_multicore\n");
    uintptr_t addr = static_cast<LittleFsStorageManager *>(c->context)->flashBase + block * c->block_size;
    // printf("[LittleFs] Erase block %lu at 0x%08lx (size %lu)\n", block, addr, c->block_size);
    int err = lfs_erase_cb_flashsafe(c, block);
    // printf("[LittleFs] flash_range_erase return: %d\n", err);
    return err;
}

extern "C"
{
    extern uint8_t __flash_lfs_start;
    extern uint8_t __flash_lfs_end;
}

void LittleFsStorageManager::configure()
{
    flashBase = reinterpret_cast<uintptr_t>(&__flash_lfs_start);
    flashSize = reinterpret_cast<uintptr_t>(&__flash_lfs_end) - flashBase;

    std::memset(&config, 0, sizeof(config));

    config.context = this;
    config.read = lfs_read_cb;
    config.prog = lfs_prog_cb;
    config.erase = lfs_erase_cb;
    config.sync = [](const struct lfs_config *) -> int
    { return 0; };

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

bool LittleFsStorageManager::mount()
{
    int err = lfs_mount(&lfs, &config);
    if (err)
    {
        printf("Mount failed err: %d, erasing and formatting...\n", err);
        formatStorage();
 
        err = lfs_format(&lfs, &config);
        printf("Formatting complete, remounting...\n");

        err = lfs_mount(&lfs, &config);
    }
    mounted = (err == 0);
    printf("Mount %s\n", mounted ? "successful" : "failed");
    return mounted;
}

bool LittleFsStorageManager::unmount()
{
    if (mounted)
    {
        lfs_unmount(&lfs);
        mounted = false;
    }
    return true;
}

bool LittleFsStorageManager::isMounted() const
{
    return mounted;
}

bool LittleFsStorageManager::exists(const std::string &path)
{
    struct lfs_info info;
    //printf("[LittleFS] Checking existence of '%s'\n", path.c_str());
    int err = lfs_stat(&lfs, path.c_str(), &info);
    //printf("[LittleFS] lfs_stat returned %d\n", err);
    return (err == 0);
}

bool LittleFsStorageManager::remove(const std::string &path)
{
    return lfs_remove(&lfs, path.c_str()) == 0;
}

bool LittleFsStorageManager::rename(const std::string &from, const std::string &to)
{
    return lfs_rename(&lfs, from.c_str(), to.c_str()) == 0;
}

bool LittleFsStorageManager::readFile(const std::string &path, std::vector<uint8_t> &out)
{
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
    int bytes = lfs_file_read(&lfs, &file, out.data(), size);
    lfs_file_close(&lfs, &file);
    return (bytes >= 0) && (bytes == size);
}

bool LittleFsStorageManager::writeFile(const std::string &path, const std::vector<uint8_t> &data)
{
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) < 0)
        return false;
    int written = lfs_file_write(&lfs, &file, data.data(), data.size());
    int closed = lfs_file_close(&lfs, &file);
    return (written == static_cast<int>(data.size())) && (closed == 0);
}

bool LittleFsStorageManager::appendToFile(const std::string &path, const uint8_t *data, size_t size)
{
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND) < 0)
    {
        printf("[LittleFS] appendToFile: open failed for '%s'\n", path.c_str());
        lfs_file_close(&lfs, &file);
        return false;
    }
    int written = lfs_file_write(&lfs, &file, data, size);
    if (written < 0)
    {
        printf("[LittleFS] appendToFile: write failed for '%s'\n", path.c_str());
        lfs_file_close(&lfs, &file);
        return false;
    }
    lfs_file_close(&lfs, &file);
    return written == (int)size;
}

bool LittleFsStorageManager::streamFile(const std::string &path, std::function<void(const uint8_t *, size_t)> chunkCallback)
{
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_RDONLY) < 0)
        return false;

    uint8_t buf[64];
    int readBytes;
    bool success = true;

    while ((readBytes = lfs_file_read(&lfs, &file, buf, sizeof(buf))) > 0)
    {
        chunkCallback(buf, readBytes);
    }

    if (readBytes < 0)
    {
        printf("[LittleFS] streamFile: read failed for '%s'\n", path.c_str());
        success = false;
    }

    lfs_file_close(&lfs, &file);
    return success;
}

size_t LittleFsStorageManager::getFileSize(const std::string &path)
{
    lfs_file_t file;
    if (lfs_file_open(&lfs, &file, path.c_str(), LFS_O_RDONLY) < 0)
    {
        printf("[LittleFS] getFileSize: open failed for '%s'\n", path.c_str());
        return 0;
    }
    lfs_soff_t size = lfs_file_size(&lfs, &file);
    lfs_file_close(&lfs, &file);
    return size < 0 ? 0 : size;
}

bool LittleFsStorageManager::listDirectory(const std::string &path, std::vector<FileInfo> &out)
{
    lfs_dir_t dir;
    struct lfs_info info;

    if (lfs_dir_open(&lfs, &dir, path.c_str()) < 0)
        return false;

    while (lfs_dir_read(&lfs, &dir, &info) > 0)
    {
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
            continue;
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

bool LittleFsStorageManager::createDirectory(const std::string &path)
{
    //printf("[LittleFS] Creating directory '%s'\n", path.c_str());
    if (lfs_mkdir(&lfs, path.c_str()) < 0)
    {
        printf("[LittleFS] Failed to create directory '%s'\n", path.c_str());
        return false;
    }
    //printf("[LittleFS] Directory '%s' created successfully\n", path.c_str());
    return true;
}

bool LittleFsStorageManager::removeDirectory(const std::string &path)
{
    return lfs_remove(&lfs, path.c_str()) == 0;
}

struct FormatContext {
    LittleFsStorageManager *self;
    bool *result;
};

void LittleFsStorageManager::formatInner(bool *result) {
    int err = lfs_format(&lfs, &config);
    *result = (err == 0);
}

static void __not_in_flash_func(format_callback)(void *param) {
    auto *ctx = static_cast<FormatContext *>(param);
    ctx->self->formatInner(ctx->result);
}


bool LittleFsStorageManager::formatStorage()
{
    bool result = false;

    #if configNUM_CORES > 1
    // Use multicore lockout to ensure atomicity
    // Core-safe execution using flash_safe_execute
    FormatContext ctx = { this, &result };

    flash_safe_execute(format_callback, &ctx, 5000);
#else
    // Fallback single-core safe version
    uint32_t status = save_and_disable_interrupts();
    int err = lfs_format(&lfs, &config);
    restore_interrupts(status);
    result = (err == 0);
#endif

    if (result)
    {
        printf("[LittleFs] Format successful\n");
        mount();
    }
    else
    {
        printf("[LittleFs] Format failed\n");
    }
    return result;
}
