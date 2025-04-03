/**
 * @file MultipartParser.h
 * @author Ian Archbell
 * @brief Parser for handling multipart/form-data file uploads.
 * Part of the PicoFramework HTTP server.
 * This module provides the MultipartParser class, which is responsible for
 * processing multipart/form-data uploads over HTTP. It handles detecting
 * boundaries, parsing headers, extracting filenames, and writing file data
 * to storage (e.g., an SD card). It also sends appropriate HTTP responses
 * based on the success or failure of the upload process.
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 */

 #ifndef MULTIPART_PARSER_HPP
 #define MULTIPART_PARSER_HPP
 #pragma once
 
 #include <string>
 #include <unordered_map>
 #include "HttpRequest.h"
 
 enum State {
    SEARCHING_FOR_BOUNDARY,
    FOUND_BOUNDARY,
    FOUND_DATA_START,
    COMPLETE
};


 /**
  * @brief Parses and processes multipart/form-data uploads over HTTP.
  * 
  * MultipartParser is responsible for:
  * - Detecting boundaries and parsing multipart headers
  * - Extracting uploaded filenames
  * - Writing uploaded file data to SD or other storage
  * - Sending appropriate HTTP responses
  */
 class MultipartParser {
 public:
     /**
      * @brief Construct a new Multipart Parser object.
      * 
      * @param clientSocket The socket for the HTTP client connection.
      * @param request The full HTTP request object.
      */
     MultipartParser(int clientSocket, const Request& request);
 
     /**
      * @brief Begin processing the multipart upload from the socket.
      * 
      * This method reads from the client socket in chunks, processes headers and boundaries,
      * and writes file contents to storage.
      * 
      * @return true if upload succeeds.
      * @return false on failure (mazlformed request, existing file, or write error).
      */
     bool handleMultipart();
 
 private:
     int clientSocket;
     std::string boundary;
     std::string filename;
     std::string leftoverData;
     std::string buffer;
     Request request;
 
     static State currentState;
 
     /**
      * @brief Extract the filename from a Content-Disposition header.
      * @param contentDisposition The raw header line.
      * @return true if a valid filename is found.
      */
     bool extractFilename(const std::string& contentDisposition);
 
     /**
      * @brief Append a chunk of file data to the output file.
      * @param fileData Data to append.
      * @return true on success.
      */
     bool processFileData(const std::string& fileData);
 
     /**
      * @brief Handle an incoming chunk of multipart data.
      * @param chunkData The raw data received.
      * @return true if processing should continue.
      */
     bool handleChunk(std::string& chunkData);
 
     /**
      * @brief Append file data to storage (e.g., SD card).
      * @param filename Name of the file to write to.
      * @param data Pointer to raw data.
      * @param size Length of the data buffer.
      * @return 0 on success, non-zero on failure.
      */
     int appendFileToSD(const char* filename, const char* data, size_t size);
 
     /**
      * @brief Check if a file already exists in storage.
      * @param filename Name of the file to check.
      * @return 1 if exists, 0 if not.
      */
     int file_exists(const char* filename);
 
     /**
      * @brief Find the start of the file content in a chunk (after headers).
      * @param currentData The buffer containing headers + data.
      * @return Byte offset of start of data, or -1 if not found.
      */
     int findDataStart(std::string& currentData);
 
     /**
      * @brief Detect and handle the final multipart boundary.
      * @param fileData The current chunk of file data.
      * @return true if final boundary was found and handled.
      */
     bool handleFinalBoundary(std::string& fileData);
 
     /**
      * @brief Send a simple HTTP response to the client.
      * @param statusCode HTTP status code.
      * @param message Message body to send.
      */
     void sendHttpResponse(int statusCode, const std::string& message);
 };

 #endif // MULTIPART_PARSER_HPP
 