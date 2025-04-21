/**
 * @file FatFsStorageManager.cpp
 * @author Ian Archbell
 * @brief Implementation of the FatFs-based StorageManager.
 *
 * Part of the PicoFramework application framework.
 * Provides thread-safe file operations for embedded systems using the FatFs API.
 * Wraps `ff_stdio.h` and manages mounting/unmounting, file streaming, and directory traversal.
 * Designed for use with Raspberry Pi Pico and other embedded SD-based storage.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(FatFsStorageManager)

#include "FatFsStorageManager.h"
#include <ff_utils.h>
#include <ff_stdio.h>

/// @copydoc FatFsStorageManager::FatFsStorageManager()
FatFsStorageManager::FatFsStorageManager() {}

/// @copydoc FatFsStorageManager::mount()
bool FatFsStorageManager::mount()
{
    if (mounted)
        return true;

    TRACE("Mounting SD card at: %s\n", mountPoint.c_str());
    mounted = ::mount(mountPoint.c_str());

    if (mounted && !probeMountPoint())
    {
        TRACE("SD mount appears successful, but directory listing failed — treating as mount failure\n");
        mounted = false;
    }

    if (!mounted)
    {
        TRACE("ERROR: SD mount FAILED for %s\n", mountPoint.c_str());
    }
    else
    {
        TRACE("SD card mounted successfully\n");
    }

    return mounted;
}

/// @copydoc FatFsStorageManager::unmount()
bool FatFsStorageManager::unmount()
{
    ::unmount(mountPoint.c_str());
    mounted = false;
    return true;
}

/// @copydoc FatFsStorageManager::exists()
bool FatFsStorageManager::exists(const std::string &path)
{
   TRACE("[FatFs] Checking if path exists: %s\n", path.c_str());
    if (!ensureMounted())
    {
        printf("SD card not mounted — cannot check path exists: %s\n", path.c_str());
        return false;
    }
    TRACE("[FatFs] Checking if exists: %s\n", resolvePath(path).c_str());
    FF_Stat_t xStat;

    // Use ff_stat to check if the file or directory exists
    TRACE("[FatFs] Calling ff_stat on path: %s\n",resolvePath(path).c_str());
    int err = ff_stat(resolvePath(path).c_str(), &xStat);
    TRACE("[FatFs] ff_stat returned: %d\n", err);
    if (err == FF_ERR_NONE){
        TRACE("[FatFs] Path exists: %s, size: %ld bytes\n", resolvePath(path).c_str(), xStat.st_size);
        return true;
    }
    TRACE("Path does not exist: %s\n", path.c_str());
    return false;
}

/// @copydoc FatFsStorageManager::listDirectory()
bool FatFsStorageManager::listDirectory(const std::string &path, std::vector<FileInfo> &out)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot list directory: %s\n", path.c_str());
        return false;
    }
    FF_FindData_t xFindStruct;
    memset(&xFindStruct, 0, sizeof(xFindStruct));

    std::string searchPath = resolvePath(path.empty() ? "/" : path);
    int result = ff_findfirst(searchPath.c_str(), &xFindStruct);

    if (result != FF_ERR_NONE)
    {
        return false;
    }

    do
    {
        if (strlen(xFindStruct.pcFileName) > 0)
        {
            FileInfo info;
            info.name = xFindStruct.pcFileName;
            info.isDirectory = xFindStruct.ucAttributes & FF_FAT_ATTR_DIR;
            info.isReadOnly = xFindStruct.ucAttributes & FF_FAT_ATTR_READONLY;
            info.size = static_cast<size_t>(xFindStruct.ulFileSize);
            out.push_back(info);
        }
    } while (ff_findnext(&xFindStruct) == FF_ERR_NONE);

    return true;
}

bool FatFsStorageManager::createDirectory(const std::string &path)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot create directory: %s\n", path.c_str());
        return false;
    }
    printf("[FatFs] Creating directory: %s\n", path.c_str());
    return ff_mkdir(resolvePath(path).c_str()) == FF_ERR_NONE;
}

bool FatFsStorageManager::removeDirectory(const std::string &path)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot remove directory: %s\n", path.c_str());
        return false;
    }
    return ff_rmdir(resolvePath(path).c_str()) == FF_ERR_NONE;
}   

/// @copydoc FatFsStorageManager::readFile()
bool FatFsStorageManager::readFile(const std::string &path, std::vector<uint8_t> &buffer)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot read file: %s\n", path.c_str());
        return false;
    }
    FF_FILE *file = ff_fopen(resolvePath(path).c_str(), "r");
    if (!file)
        return false;

    ff_fseek(file, 0, SEEK_END);
    long fileSize = ff_ftell(file);
    ff_fseek(file, 0, SEEK_SET);

    if (fileSize < 0)
    {
        ff_fclose(file);
        return false;
    }

    buffer.resize(fileSize);
    ff_fread(buffer.data(), 1, fileSize, file);
    ff_fclose(file);
    return true;
}

/// @copydoc FatFsStorageManager::writeFile()
bool FatFsStorageManager::writeFile(const std::string &path, const std::vector<uint8_t> &data)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot write to file: %s\n", path.c_str());
        return false;
    }
    FF_FILE *file = ff_fopen(resolvePath(path).c_str(), "w");
    if (!file)
        return false;

    ff_fwrite(data.data(), 1, data.size(), file);
    ff_fclose(file);
    return true;
}

/// @copydoc FatFsStorageManager::remove()
bool FatFsStorageManager::remove(const std::string &path)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot remove file: %s\n", path.c_str());
        return false;
    }
    return ff_remove(resolvePath(path).c_str()) == FF_ERR_NONE;
}

/// @copydoc FatFsStorageManager::rename()
bool FatFsStorageManager::rename(const std::string &from, const std::string &to)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot rename file %s to file: %s\n", from.c_str(), to.c_str());
        return false;
    }
    return ff_rename(resolvePath(from).c_str(), resolvePath(to).c_str(), false) == 0;
}

/// @copydoc FatFsStorageManager::streamFile()
bool FatFsStorageManager::streamFile(const std::string &path, std::function<void(const uint8_t *, size_t)> chunkCallback)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot stream file: %s\n", path.c_str());
        return false;
    }
    FF_FILE *file = ff_fopen(resolvePath(path).c_str(), "r");
    if (!file)
        return false;

    uint8_t buffer[512];
    size_t bytes;
    while ((bytes = ff_fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        chunkCallback(buffer, bytes);
    }

    ff_fclose(file);
    return true;
}

/// @copydoc FatFsStorageManager::getFileSize()
size_t FatFsStorageManager::getFileSize(const std::string &path)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot get file size: %s\n", path.c_str());
        return false;
    }
    FF_FILE *file = ff_fopen(resolvePath(path).c_str(), "r");
    if (!file)
        return 0;

    ff_fseek(file, 0, SEEK_END);
    long size = ff_ftell(file);
    ff_fclose(file);

    return (size >= 0) ? static_cast<size_t>(size) : 0;
}

/// @copydoc FatFsStorageManager::appendToFile()
bool FatFsStorageManager::appendToFile(const std::string &path, const uint8_t *data, size_t size)
{
    if (!ensureMounted())
    {
        TRACE("SD card not mounted — cannot append to file: %s\n", path.c_str());
        return false;
    }
    FF_FILE *file = ff_fopen(resolvePath(path).c_str(), "a");
    if (!file)
        return false;

    size_t written = ff_fwrite(data, 1, size, file);
    ff_fclose(file);
    return written == size;
}

/// @brief Helper function to normalize full path based on mountPoint and relative path
std::string FatFsStorageManager::resolvePath(const std::string &path) const
{
    std::string fullPath = "/" + mountPoint;
    if (!path.empty())
    {
        if (path[0] != '/')
            fullPath += "/";
        fullPath += path;
    }

    // Normalize duplicate slashes
    std::string normalized;
    bool lastWasSlash = false;
    for (char c : fullPath)
    {
        if (c == '/')
        {
            if (!lastWasSlash)
            {
                normalized += c;
                lastWasSlash = true;
            }
        }
        else
        {
            normalized += c;
            lastWasSlash = false;
        }
    }

    return normalized;
}

bool FatFsStorageManager::ensureMounted()
{
    if (!mounted)
    {
        return mount();
    }
    return true;
}

bool FatFsStorageManager::isMounted() const
{
    return mounted;
}

void FatFsStorageManager::refreshMountState()
{
    if (mounted && !probeMountPoint())
    {
        TRACE("SD no longer accessible — marking as unmounted\n");
        mounted = false;
    }
}

bool FatFsStorageManager::probeMountPoint()
{
    FF_FindData_t xFindStruct = {};
    std::string root = "/" + mountPoint;
    int result = ff_findfirst(root.c_str(), &xFindStruct);
    return result == FF_ERR_NONE;
}

bool FatFsStorageManager::formatStorage()
{
    if (!mounted) {
        printf("[FatFs] Cannot format: card not mounted\n");
        return false;
    }

    if (!format(mountPoint.c_str())) {
        printf("[FatFs] Format failed for device: %s\n", mountPoint.c_str());
        return false;
    }

    printf("[FatFs] Format successful for device: %s\n", mountPoint.c_str());

    // Optionally re-mount to refresh filesystem state
    unmount();
    mount();

    return mounted;
}

bool FatFsStorageManager::readFileString(const std::string &path, uint32_t startPosition, uint32_t length, std::string &buffer){
    if (!mounted)
    {
        TRACE("SD card not mounted — cannot read file string: %s\n", path.c_str());
        return false;
    }

    FF_FILE *file = ff_fopen(resolvePath(path).c_str(), "r");
    if (!file)
    {
        TRACE("Failed to open file: %s\n", path.c_str());
        return false;
    }

    ff_fseek(file, startPosition, SEEK_SET);
    buffer.resize(length);
    size_t bytesRead = ff_fread(buffer.data(), 1, length, file);
    ff_fclose(file);

    if (bytesRead < length)
    {
        buffer.resize(bytesRead); // Trim to actual size read
        TRACE("Read only %zu bytes from file: %s\n", bytesRead, path.c_str());
    }

    return true;
}

