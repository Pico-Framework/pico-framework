#ifndef MULTIPART_PARSER_HPP
#define MULTIPART_PARSER_HPP

#include <string>
#include <unordered_map>
#include "HttpRequest.h"

class MultipartParser {
public:

    MultipartParser(int clientSocket, const Request& request);
    bool handleMultipart();
    
private:
    int clientSocket;
    std::string boundary;
    std::string filename;
    std::string leftoverData;
    std::string buffer;

    Request request;

    enum State { SEARCHING_FOR_BOUNDARY, FOUND_BOUNDARY, FOUND_DATA_START };
    State currentState = SEARCHING_FOR_BOUNDARY;

    bool extractFilename(const std::string& contentDisposition);
    bool processFileData(const std::string& fileData);
    bool handleChunk(std::string& chunkData);
    int appendFileToSD(const char* filename, const char* data, size_t size);
    int file_exists(const char* filename);
    int findDataStart(std::string& currentData);
    bool handleFinalBoundary(std::string& fileData);
    void sendHttpResponse(int statusCode, const std::string& message);
};

#endif // MULTIPART_PARSER_HPP
