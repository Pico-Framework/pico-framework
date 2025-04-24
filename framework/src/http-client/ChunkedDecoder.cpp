#include "http/ChunkedDecoder.h"
#include <sstream>

void ChunkedDecoder::feed(const std::string& data, size_t maxLength) {
    buffer += data;
    parseChunks(maxLength);
}

bool ChunkedDecoder::feedToFile(const std::string& data,
                                std::function<bool(const char* data, size_t len)> writeFn,
                                size_t maxLength)
{
    buffer += data;
    std::istringstream stream(buffer);
    std::string line;
    truncated = false;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        int chunkSize = 0;
        std::istringstream chunkHeader(line);
        chunkHeader >> std::hex >> chunkSize;

        if (chunkSize == 0) {
            complete = true;
            buffer.clear();
            return true;
        }

        if ((int)stream.rdbuf()->in_avail() < chunkSize + 2) {
            return true; // wait for more input
        }

        char* chunkData = new char[chunkSize];
        stream.read(chunkData, chunkSize);

        size_t available = maxLength - totalDecoded;
        size_t toWrite = std::min((size_t)chunkSize, available);

        if (!writeFn(chunkData, toWrite)) {
            delete[] chunkData;
            return false;
        }

        delete[] chunkData;
        totalDecoded += toWrite;

        if (chunkSize > (int)available) {
            truncated = true;
            complete = true;
            return true;
        }

        std::getline(stream, line); // Consume CRLF
    }

    buffer.clear();
    return true;
}


std::string ChunkedDecoder::getDecoded() {
    return decoded;
}

bool ChunkedDecoder::isComplete() const {
    return complete;
}

void ChunkedDecoder::parseChunks(size_t maxLength) {
    std::istringstream stream(buffer);
    std::string line;
    std::string tempDecoded;
    truncated = false;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        int chunkSize = 0;
        std::istringstream chunkHeader(line);
        chunkHeader >> std::hex >> chunkSize;

        if (chunkSize == 0) {
            complete = true;
            break;
        }

        char* chunk = new char[chunkSize];
        stream.read(chunk, chunkSize);

        size_t remaining = maxLength - tempDecoded.size();
        if (chunkSize > remaining) {
            tempDecoded.append(chunk, remaining);
            truncated = true;
            delete[] chunk;
            break;
        }

        tempDecoded.append(chunk, chunkSize);
        delete[] chunk;

        std::getline(stream, line); // Consume CRLF
    }

    decoded = std::move(tempDecoded);
}
