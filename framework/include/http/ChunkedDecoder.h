#pragma once
#include <string>
#include "framework_config.h"
class ChunkedDecoder {
public:
    void feed(const std::string& data, size_t maxLength = MAX_HTTP_BODY_LENGTH);

    std::string getBuffer() const { return buffer; }
    bool wasTruncated() const { return truncated; }
    std::string getDecoded();
    bool isComplete() const;

private:
    std::string buffer;
    std::string decoded;
    bool complete = false;
    bool truncated = false;

    void parseChunks(size_t maxLength);
};
