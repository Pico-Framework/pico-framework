#pragma once
#include <string>

class ChunkedDecoder {
public:
    void feed(const std::string& data);
    std::string getDecoded();
    bool isComplete() const;

private:
    std::string buffer;
    std::string decoded;
    bool complete = false;

    void parseChunks();
};
