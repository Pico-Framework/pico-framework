#pragma once

#include "storage/StorageFileReader.h"
#include <ff_stdio.h>
#include <string>

/**
 * @brief Buffered line reader for FatFs FILE* files
 */
class FatFsFileReader : public StorageFileReader {
public:
    FatFsFileReader();
    ~FatFsFileReader() override;

    /**
     * @brief Opens a file for reading
     * @param path Path to the file
     * @return true if opened successfully
     */
    bool open(const std::string& path);

    /**
     * @brief Reads a single line into the provided buffer.
     * Carriage returns are stripped. Stops at newline or buffer end.
     * @param outLine Destination buffer
     * @param maxLen Size of the buffer
     * @return true if line read, false on EOF or error
     */
    bool readLine(char* outLine, size_t maxLen) override;

    /**
     * @brief Closes the file
     */
    void close() override;

private:
    FF_FILE* file = nullptr;  // FatFs file handle
    bool isOpen = false;
};
