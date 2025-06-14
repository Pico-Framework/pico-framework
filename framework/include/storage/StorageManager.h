/**
 * @file StorageManager.h
 * @author Ian Archbell
 * @brief Abstract interface for file and directory storage backends.
 *
 * Part of the PicoFramework application framework.
 * Defines a virtual base class for file storage operations including read/write,
 * file streaming, directory listing, and size queries.
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
#include <nlohmann/json.hpp>
#include "storage/StorageFileReader.h"

/**
 * @brief Structure representing metadata for a file or directory.
 */
struct FileInfo
{
    std::string name; ///< File or directory name
    bool isDirectory; ///< True if item is a directory
    bool isReadOnly;  ///< True if item is read-only
    size_t size;      ///< Size in bytes
};

inline void to_json(nlohmann::json &j, const FileInfo &f) {
    j = nlohmann::json{
        {"name", f.name},
        {"size", f.size},
        {"isDir", f.isDirectory}
    };
}

/**
 * @brief Abstract base class for storage access and file operations.
 */
class StorageManager
{
public:
    virtual ~StorageManager() = default;

    /** @brief Mount the underlying storage. */
    virtual bool mount() = 0;

    /** @brief Unmount the storage. */
    virtual bool unmount() = 0;

    /** @brief Check mounted */
    virtual bool isMounted() const = 0;

    /** @brief Check whether a file or directory exists at the given path. */
    virtual bool exists(const std::string &path) = 0;

    /** @brief Remove a file or directory. */
    virtual bool remove(const std::string &path) = 0;

    /** @brief Rename a file or directory. */
    virtual bool rename(const std::string &from, const std::string &to) = 0;

    /** @brief Read a file into a memory buffer. */
    virtual bool readFile(const std::string &path, std::vector<uint8_t> &buffer) = 0;

    /** @brief Read a file string into a memory buffer */
    virtual bool readFileString(const std::string &path, uint32_t startPosition, uint32_t length, std::string &buffer) = 0;

    /** @brief Write a memory buffer to a file. */
    virtual bool writeFile(const std::string &path, const std::vector<uint8_t> &data) = 0;
    /// Write from a raw buffer
    virtual bool writeFile(const std::string& path, const unsigned char* data, size_t size) = 0;


    /** @brief Stream a file in chunks via callback. */
    virtual bool streamFile(const std::string &path, std::function<void(const uint8_t *, size_t)> chunkCallback) = 0;

    /** @brief List all entries in the given directory. */
    virtual bool listDirectory(const std::string &path, std::vector<FileInfo> &out) = 0;

    /** @brief Create a directory at the given path (recursive if needed). */
    virtual bool createDirectory(const std::string &path) = 0;

    /** @brief Remove a directory (must be empty, or recursively if supported). */
    virtual bool removeDirectory(const std::string &path) = 0;

    /** @brief Get the size of a file. */
    virtual size_t getFileSize(const std::string &path) = 0;

    /** @brief Append data to a file. */
    virtual bool appendToFile(const std::string &path, const uint8_t *data, size_t size) = 0;

    /** @brief Format the storage (if applicable). */
    virtual bool formatStorage() = 0;

    /**
     * @brief Open a file for streaming read access.
     * @param path The file path
     * @return A new reader object, or nullptr on failure
     */
    virtual std::unique_ptr<StorageFileReader> openReader(const std::string& path) = 0;
};