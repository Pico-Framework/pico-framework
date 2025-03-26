/**
 * @file JwtAuthenticator.h
 * @author yIan Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef JWT_AUTHENTICATOR_H
#define JWT_AUTHENTICATOR_H

#include <string>

class Authenticator {
    public:
        // Singleton access
        static Authenticator& getInstance(){
            static Authenticator instance;
            return instance;
        };

        // JWT handling
        std::string generateJWT(const std::string& userId, const std::string& userName) const;
        bool validateJWT(const std::string& token, bool validateExpiry = false) const;
        bool decodeJWT(const std::string& token, std::string& header, std::string& payload, std::string& signature) const;
        bool isJWTExpired(const std::string& token) const;
        bool isJWTPayloadExpired(const std::string& payload) const;
        bool verifyJWTSignature(const std::string& encoded_header, const std::string& encoded_payload, const std::string& signature) const;

    private:
        // Private constructor for Singleton
        Authenticator();
        std::string secretKey;

        // Internal helper methods
        std::string base64urlEncode(const std::string& input) const;
        bool base64urlDecode(const std::string& input, std::string& output) const;
        bool isBase64urlEncoded(const std::string& str) const;
        std::string bytesToBase64url(const unsigned char* data, size_t length) const;
        std::string hmacSHA256(const std::string& message) const;
};

#endif // JWT_AUTHENTICATOR_HPP