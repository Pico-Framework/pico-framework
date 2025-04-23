#include "http/ChunkedDecoder.h"
#include <sstream>

void ChunkedDecoder::feed(const std::string& data) {
    buffer += data;
    parseChunks();
}

std::string ChunkedDecoder::getDecoded() {
    return decoded;
}

bool ChunkedDecoder::isComplete() const {
    return complete;
}

void ChunkedDecoder::parseChunks() {
    std::istringstream stream(buffer);
    std::string line;
    decoded.clear();

    while (std::getline(stream, line)) {
        if (line.empty())
            continue;

        int chunkSize = 0;
        std::istringstream chunkHeader(line);
        chunkHeader >> std::hex >> chunkSize;

        if (chunkSize == 0) {
            complete = true;
            break;
        }

        char* chunk = new char[chunkSize];
        stream.read(chunk, chunkSize);
        decoded.append(chunk, chunkSize);
        delete[] chunk;

        std::getline(stream, line); // consume CRLF after chunk
    }
}
