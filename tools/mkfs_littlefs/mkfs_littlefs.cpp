
// Minimal standalone LittleFS image builder
// Works on any platform with a C++17 compiler
// No Linux headers, no FUSE

#include "lfs.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Constants for LittleFS layout
constexpr size_t BLOCK_SIZE = 4096;
constexpr size_t BLOCK_COUNT = 256;
constexpr size_t READ_SIZE = 16;
constexpr size_t PROG_SIZE = 16;
constexpr size_t LOOKAHEAD_SIZE = 16;

std::vector<uint8_t> storage(BLOCK_SIZE * BLOCK_COUNT, 0xFF);

// Callbacks for LittleFS
int lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    std::memcpy(buffer, &storage[block * c->block_size + off], size);
    return 0;
}

int lfs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    std::memcpy(&storage[block * c->block_size + off], buffer, size);
    return 0;
}

int lfs_erase(const struct lfs_config *c, lfs_block_t block) {
    std::memset(&storage[block * c->block_size], 0xFF, c->block_size);
    return 0;
}

int lfs_sync(const struct lfs_config *c) {
    return 0;
}

// Copy all files from input dir into LittleFS root
bool populate_filesystem(const std::string& input_dir, lfs_t& lfs) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(input_dir)) {
        if (!entry.is_regular_file()) continue;

        std::string relative_path = std::filesystem::relative(entry.path(), input_dir).string();
        std::replace(relative_path.begin(), relative_path.end(), '\\', '/');

        lfs_file_t file;
        if (lfs_file_open(&lfs, &file, relative_path.c_str(), LFS_O_CREAT | LFS_O_WRONLY) < 0) {
            std::cerr << "Failed to create: " << relative_path << "\n";
            return false;
        }

        std::ifstream in(entry.path(), std::ios::binary);
        std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        lfs_file_write(&lfs, &file, data.data(), data.size());
        lfs_file_close(&lfs, &file);
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: mkfs-littlefs <input_dir> <output_img>\n";
        return 1;
    }

    std::string input_dir = argv[1];
    std::string output_img = argv[2];

    lfs_config cfg = {};
    cfg.read = lfs_read;
    cfg.prog = lfs_prog;
    cfg.erase = lfs_erase;
    cfg.sync = lfs_sync;
    cfg.read_size = READ_SIZE;
    cfg.prog_size = PROG_SIZE;
    cfg.block_size = BLOCK_SIZE;
    cfg.block_count = BLOCK_COUNT;
    cfg.block_cycles = 512;
    cfg.lookahead_size = LOOKAHEAD_SIZE;
    cfg.cache_size = READ_SIZE; // Must be a multiple of read_size and prog_size


    lfs_t lfs;
    if (lfs_format(&lfs, &cfg) != 0 || lfs_mount(&lfs, &cfg) != 0) {
        std::cerr << "Failed to format or mount filesystem\n";
        return 2;
    }

    if (!populate_filesystem(input_dir, lfs)) {
        std::cerr << "Failed to populate filesystem\n";
        return 3;
    }

    std::ofstream out(output_img, std::ios::binary);
    out.write(reinterpret_cast<char*>(storage.data()), storage.size());
    lfs_unmount(&lfs);
    return 0;
}
