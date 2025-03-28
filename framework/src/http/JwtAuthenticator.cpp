/**
 * @file JwtAuthenticator.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <iostream>
#include <mbedtls/md.h>
#include <mbedtls/base64.h>
#include <mbedtls/error.h>
#include <nlohmann/json.hpp>
#include <chrono>

#include "JwtAuthenticator.h"

using json = nlohmann::json;

#ifndef MBEDTLS_SHA256_DIGEST_LENGTH
#define MBEDTLS_SHA256_DIGEST_LENGTH 32
#endif

Authenticator::Authenticator() {
    // Set the secret key for HMAC-SHA256
    secretKey = JWT_SECRET;
}

std::string Authenticator::base64urlEncode(const std::string& input) const {
    // Calculate the required buffer size for Base64 encoding
    size_t encoded_len = ((input.length() + 2) / 3) * 4;  // Correct formula
    encoded_len++;
    char* encoded_data = new char[encoded_len];

    //printf("Input length: %i\n", input.length());
    //printf("Encoded length %i\n", encoded_len);

    // Perform Base64 encoding
    size_t len = 0;
    int result = mbedtls_base64_encode(reinterpret_cast<unsigned char*>(encoded_data), encoded_len, &len, 
                                       reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
    //printf("Output length %i\n", len);

    // Check for encoding failure
    if (result != 0) {
        std::cerr << "Base64 encoding failed with error code: " << result << std::endl;
        delete[] encoded_data;
        return "";
    }

    // Debug: Print the length of the base64 output
    //std::cout << "Base64 length: " << len << std::endl;

    // Convert the encoded data into a string
    std::string base64_str(encoded_data, len);

    // Debug: Print the base64 output
    //std::cout << "Base64 encoded: " << base64_str << std::endl;

    // Convert Base64 to Base64Url (replace '+' -> '-', '/' -> '_', remove '=')
    std::string base64url_str(base64_str);
    std::replace(base64url_str.begin(), base64url_str.end(), '+', '-');
    std::replace(base64url_str.begin(), base64url_str.end(), '/', '_');
    
    // Remove padding ('=')
    base64url_str.erase(std::remove(base64url_str.begin(), base64url_str.end(), '='), base64url_str.end());

    // Debug: Print the base64url output
    //std::cout << "Base64Url encoded: " << base64url_str << std::endl;

    delete[] encoded_data;
    return base64url_str;
}

bool Authenticator::base64urlDecode(const std::string& input, std::string& output) const {
    std::string base64_input = input;
    
    // Replace URL-safe base64 characters with standard ones
    std::replace(base64_input.begin(), base64_input.end(), '-', '+');
    std::replace(base64_input.begin(), base64_input.end(), '_', '/');

    // Add padding if necessary
    while (base64_input.size() % 4 != 0) {
        base64_input += "=";
    }

     // Debugging: Check for any characters with negative ASCII values
     for (size_t i = 0; i < base64_input.size(); ++i) {
        char c = base64_input[i];  // Use `char` to check for negative values
        if (c < 0) {
            std::cout << "Invalid character found (less than 0): " << (int)c << " at position " << i << std::endl;
            return false;  // Invalid character detected
        }
    }
    
    // Decode base64 string using mbedTLS
    size_t decoded_len = base64_input.size() * 3 / 4 + 4;
    decoded_len++;  // Add space for null terminator
    unsigned char* decoded_data = new unsigned char[decoded_len];

    //std::cout << "Base64 input before decode: " << base64_input << std::endl;
    
    int ret = mbedtls_base64_decode(decoded_data, decoded_len, &decoded_len, 
                                     (const unsigned char*)base64_input.c_str(), base64_input.size());
    //printf("Decoded length: %i\n", decoded_len);
    // Check for decoding failure
    if (ret != 0) {
        char error_msg[100];
        mbedtls_strerror(ret, error_msg, sizeof(error_msg));  // Pass the error code, buffer, and buffer size
        printf("Base64 decoding failed with error code: %d, message: %s\n", ret, error_msg);
        std::cout << "Error Message: " << error_msg << std::endl;
        delete[] decoded_data;
        return false;  // Return false on error
    }

    output.assign(reinterpret_cast<char*>(decoded_data), decoded_len);
    delete[] decoded_data;
    return true;
}

bool Authenticator::isBase64urlEncoded(const std::string& str) const{
    // Base64url encoded strings should only contain characters in the following set:
    // A-Z, a-z, 0-9, "-", "_", and should have lengths that are a multiple of 4
    static const std::string base64url_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    // Length of the string must be a multiple of 4 (standard base64 length rule)
    if (str.length() % 4 != 0) {
        return false;
    }

    // Check if all characters are valid base64url characters
    for (char c : str) {
        if (base64url_chars.find(c) == std::string::npos) {
            return false;  // Invalid character found
        }
    }

    return true;  // String is base64url encoded
}

std::string Authenticator::hmacSHA256(const std::string& message) const {
    unsigned char hmac_output[MBEDTLS_SHA256_DIGEST_LENGTH];
    mbedtls_md_context_t ctx;
    
    mbedtls_md_init(&ctx);
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_setup(&ctx, md_info, 1);
    mbedtls_md_hmac_starts(&ctx, reinterpret_cast<const unsigned char*>(secretKey.c_str()), secretKey.size());
    mbedtls_md_hmac_update(&ctx, reinterpret_cast<const unsigned char*>(message.c_str()), message.size());
    mbedtls_md_hmac_finish(&ctx, hmac_output);
    mbedtls_md_free(&ctx);

    return std::string(reinterpret_cast<char*>(hmac_output), MBEDTLS_SHA256_DIGEST_LENGTH);
}

std::string Authenticator::generateJWT(const std::string& userId, const std::string& userName) const {
    // Header
    nlohmann::json header;
    header["alg"] = "HS256";  // HMAC-SHA256
    header["typ"] = "JWT";

    // Payload
    nlohmann::json payload;
    payload["sub"] = userId;  // Subject (usually the user ID)
    payload["name"] = userName;  // Name (or other user info)
    payload["iat"] = std::time(0);  // Issued at
    payload["exp"] = std::time(0) + 3600;  // Expiry time (1 hour from now)

    // Base64 URL Encode Header and Payload
    std::string encodedHeader = base64urlEncode(header.dump());
    std::string encodedPayload = base64urlEncode(payload.dump());

    // Create the message to sign (Header + Payload)
    std::string message = encodedHeader + "." + encodedPayload;

    // Signature (HMAC-SHA256)
    std::string signature = base64urlEncode(hmacSHA256(message));

    // Combine Header, Payload, and Signature into JWT
    return encodedHeader + "." + encodedPayload + "." + signature;
}

bool Authenticator::decodeJWT(const std::string& token, std::string& header, std::string& payload, std::string& signature) const{
    size_t first_dot = token.find('.');
    size_t second_dot = token.find('.', first_dot + 1);

    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        return false;  // Invalid JWT format
    }

    // Get the JWT header, payload, and signature parts
    header = token.substr(0, first_dot);
    payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
    signature = token.substr(second_dot + 1);

    // Check if the header and payload need to be decoded
    if (isBase64urlEncoded(header)) {
        std::string decoded_header;
        if (!base64urlDecode(header, decoded_header)) {
            return false;  // Base64 decoding failed
        }
        header = decoded_header;
    }

    if (isBase64urlEncoded(payload)) {
        std::string decoded_payload;
        if (!base64urlDecode(payload, decoded_payload)) {
            return false;  // Base64 decoding failed
        }
        payload = decoded_payload;
    }

    return true;
}

std::string Authenticator::bytesToBase64url(const unsigned char* data, size_t length) const{
    // Base64 encode the byte array
    size_t output_len = length * 4 / 3 + 4;  // max size of base64 output
    char* base64_output = new char[output_len];
    size_t written_len = 0;
    mbedtls_base64_encode((unsigned char*)base64_output, output_len, &written_len, data, length);
    
    // Replace "+" with "-", "/" with "_", and remove padding
    std::string base64_result(base64_output, written_len);
    std::replace(base64_result.begin(), base64_result.end(), '+', '-');
    std::replace(base64_result.begin(), base64_result.end(), '/', '_');
    base64_result.erase(std::remove(base64_result.end() - 1, base64_result.end(), '='), base64_result.end());  // Remove padding

    delete[] base64_output;
    return base64_result;
}

bool Authenticator::verifyJWTSignature(const std::string& encoded_header, const std::string& encoded_payload, const std::string& signature) const{
    // Step 1: Concatenate the encoded header and payload with a dot.
    std::string header_payload = encoded_header + "." + encoded_payload;

    // Step 2: Compute HMAC SHA256 of the header_payload using the secret key
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_setup(&ctx, md_info, 1);

    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)secretKey.c_str(), secretKey.length());
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)header_payload.c_str(), header_payload.length());

    unsigned char hmac_output[MBEDTLS_SHA256_DIGEST_LENGTH];
    mbedtls_md_hmac_finish(&ctx, hmac_output);
    mbedtls_md_free(&ctx);

    // Step 3: Base64Url-encode the HMAC output (this is the computed signature)
    std::string computed_signature = bytesToBase64url(hmac_output, MBEDTLS_SHA256_DIGEST_LENGTH);

    // Debug: Print computed and JWT signature for comparison
    //printf("Computed Signature: %s\n", computed_signature.c_str());
    //printf("JWT Signature: %s\n", signature.c_str());

    // Step 4: Compare the computed signature with the JWT's signature
    return computed_signature == signature;
}

bool Authenticator::isJWTPayloadExpired(const std::string& payload) const{
    json json_data;

    // Parse JSON safely without exceptions
    auto parsed = json::parse(payload, nullptr, false);  // `false` disables exceptions
    if (parsed.is_discarded()) {  // Check if parsing failed
        std::cerr << "Error: Failed to parse JWT payload." << std::endl;
        return false;
    }

    json_data = parsed;

    // Check if "exp" field exists and is an integer
    if (!json_data.contains("exp") || !json_data["exp"].is_number_integer()) {
        std::cerr << "Error: Missing or invalid 'exp' field in the JWT payload." << std::endl;
        return false;
    }

    long long exp_timestamp = json_data["exp"].get<long long>();

    //printf("Expiration timestamp: %lld\n", exp_timestamp);
    // Check if the expiration timestamp is in the future
    if (exp_timestamp <= 0) {
        std::cerr << "Error: Invalid expiration timestamp." << std::endl;
        return false;
    }
    //printf("Current timestamp: %lld\n", std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

    // Get current time in UNIX timestamp
    auto current_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Compare expiration timestamp with the current time
    return current_timestamp >= exp_timestamp;  // `true` if expired, `false` otherwise
}


bool Authenticator::isJWTExpired(const std::string& token) const {

    // Step 1: Split the JWT string into three parts (header, payload, and signature)
    size_t first_dot = token.find('.');
    size_t second_dot = token.find('.', first_dot + 1);

    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        std::cerr << "Error: Invalid JWT format" << std::endl;
        return false;
    }

    // Extract the encoded header, payload, and signature
    std::string encoded_header = token.substr(0, first_dot);
    std::string encoded_payload = token.substr(first_dot + 1, second_dot - first_dot - 1);

    std::string decoded_payload;

    if (!base64urlDecode(encoded_payload, decoded_payload)) {
        std::cerr << "Error: Base64 URL decoding failed" << std::endl;
        return false;
    }
    return isJWTPayloadExpired(decoded_payload);
}

// Function to validate JWT
bool Authenticator::validateJWT(const std::string& token, bool validateExpiry) const{
    // Step 1: Split the JWT string into three parts (header, payload, and signature)
    size_t first_dot = token.find('.');
    size_t second_dot = token.find('.', first_dot + 1);

    if (first_dot == std::string::npos || second_dot == std::string::npos) {
        std::cerr << "Error: Invalid JWT format" << std::endl;
        return false;
    }

    // Extract the encoded header, payload, and signature
    std::string encoded_header = token.substr(0, first_dot);
    std::string encoded_payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
    std::string signature = token.substr(second_dot + 1);

    // Debug: Print the raw JWT components (header, payload, signature)
    //std::cout << "Encoded Header: " << encoded_header << std::endl;
    //std::cout << "Encoded Payload: " << encoded_payload << std::endl;
    //std::cout << "Signature: " << signature << std::endl;

    // Step 2: Base64Url-decode the header and payload using the existing base64url_decode function
    std::string decoded_header;
    std::string decoded_payload;
    if (!base64urlDecode(encoded_header, decoded_header) || !base64urlDecode(encoded_payload, decoded_payload)) {
        std::cerr << "Error: Base64 URL decoding failed" << std::endl;
        return false;
    }

    //Check if the JWT is expired
    if(validateExpiry){
        if (isJWTPayloadExpired(decoded_payload)) {
            std::cerr << "Error: JWT is expired" << std::endl;
            return false;
        }
    }
    // Debug: Print the decoded header and payload
    //std::cout << "Decoded Header: " << decoded_header << std::endl;
    //std::cout << "Decoded Payload: " << decoded_payload << std::endl;

    // Step 3: Combine the header and payload into a string for signature verification
    std::string header_payload = encoded_header + "." + encoded_payload;

    // Step 4: Verify the JWT signature
    return verifyJWTSignature(encoded_header, encoded_payload, signature);
}
