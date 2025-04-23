#pragma once

#include <string>
#include <vector>
#include <cstdint>  // For uint8_t

class PasswordHasher {
public:

    static constexpr int DEFAULT_ITERATIONS = 100000;
    static constexpr size_t HASH_LENGTH = 32;

    PasswordHasher(int iterations = DEFAULT_ITERATIONS);

    std::vector<uint8_t> hashPassword(const std::string& password, const std::vector<uint8_t>& salt);
    bool verifyPassword(const std::string& password, const std::vector<uint8_t>& salt, const std::vector<uint8_t>& expectedHash);

    std::string hashPasswordBase64(const std::string& password, const std::vector<uint8_t>& salt);
    bool verifyPasswordBase64(const std::string& password, const std::vector<uint8_t>& salt, const std::string& expectedBase64Hash);

    std::string hashAndEncode(const std::string& password);
    bool verifyEncoded(const std::string& password, const std::string& combinedSaltAndHash);

    static std::string toBase64(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> fromBase64(const std::string& base64);

    static std::vector<uint8_t> generateSalt(size_t length);

private:
    int iterations;
};
