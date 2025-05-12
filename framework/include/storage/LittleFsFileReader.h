#pragma once

#include "lfs.h"
#include "storage/StorageFileReader.h"
#include <memory>

/**
 * @brief Buffered line reader for LittleFS lfs_file_t files
 */
class LittleFsFileReader : public StorageFileReader{
public:
    explicit LittleFsFileReader(lfs_t* lfs);

    ~LittleFsFileReader() override;

    /**
     * @brief Opens a file for reading
     * @param path Path to the file
     * @return true if opened successfully
     */
    bool open(const std::string& path);

    /**
     * @brief Reads a single line into the provided buffer.
     *
     * Reads characters until newline (`\n`) or buffer is full.
     * Carriage returns (`\r`) are stripped. Returns false on EOF or error.
     *
     * @param outLine Buffer to store the line
     * @param maxLen Size of the buffer
     * @return true if a line was read, false on EOF or error
     */
    bool readLine(char* outLine, size_t maxLen);

    /**
     * @brief Close the file and release resources.
     */
    void close() override;


private:
    lfs_t* lfs = nullptr;
    lfs_file_t file{};
    bool isOpen = false;
};
