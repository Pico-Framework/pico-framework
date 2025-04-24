#pragma once
#include <string>
#include <functional>
#include "framework_config.h"
class ChunkedDecoder {
public:
    void feed(const std::string& data, size_t maxLength = MAX_HTTP_BODY_LENGTH);
    bool feedToFile(const std::string& data,
        std::function<bool(const char* data, size_t len)> writeFn,
        size_t maxLength = MAX_HTTP_BODY_LENGTH);

    std::string getBuffer() const { return buffer; }
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
