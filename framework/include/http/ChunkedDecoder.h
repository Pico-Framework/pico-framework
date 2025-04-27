#pragma once
#include <string>
#include <functional>
#include "framework_config.h"
class ChunkedDecoder {
public:

    void feed(const std::string& data, size_t maxLength = MAX_HTTP_BODY_LENGTH);
    
    /**
     * @brief Feed data to the decoder, writing it to a file using the provided write function.
     * 
     * This function will write the data to a file using the provided write function.
     * If the data exceeds maxLength, it will be truncated and truncated flag will be set.
     * 
     * @param data The data to feed to the decoder.
     * @param writeFn The function to write data to a file.
     * @param maxLength The maximum length of data to process (default is MAX_HTTP_BODY_LENGTH).
     * @return true if all data was written successfully, false if it was truncated.
     * @note The write function should return true if the data was written successfully, false otherwise.
     * If the write function returns false, the decoder will stop processing further data.
     * If the data exceeds maxLength, it will be truncated and the truncated flag will be set.
     * 
     */
    bool feedToFile(const std::string& data,
        std::function<bool(const char* data, size_t len)> writeFn,
        size_t maxLength = MAX_HTTP_BODY_LENGTH);

    /**
     * @brief Get the current buffer content.       
     * This returns the raw buffer content that has been fed to the decoder.
     * @return The current buffer content as a string.
     */
    std::string getBuffer() const { return buffer; }

    /**
     * @brief Set if the content-length is longer than the maximum value allowed
     * 
     * This checks if the decoder has processed all chunks and is ready to return the decoded data.
     * @return true if decoding is complete, false otherwise.
     */
    bool wasTruncated() const { return truncated; }
    std::string getDecoded();
    bool isComplete() const;

private:
    std::string buffer;
    std::string decoded;
    bool complete = false;
    bool truncated = false;
    size_t totalDecoded = 0;

    void parseChunks(size_t maxLength);
};
