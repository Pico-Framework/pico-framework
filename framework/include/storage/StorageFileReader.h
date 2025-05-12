#pragma once
#include <cstddef>
/**
 * @brief Abstract interface for reading a file line-by-line.
 */
class StorageFileReader {
public:
    virtual ~StorageFileReader() = default;

    /**
     * @brief Reads a single line into the provided buffer.
     * The newline character is stripped.
     * @param buffer A character buffer
     * @param maxLen Maximum number of characters to read (including null terminator)
     * @return true if a line was read, false on EOF or error
     */
    virtual bool readLine(char* buffer, size_t maxLen) = 0;

    /**
     * @brief Close the file and release resources.
     */
    virtual void close() = 0;
};
