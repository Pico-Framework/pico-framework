#include <string>
#include <vector>
#include <cstring>       // For std::memcmp
#include <cstdint>       //  For uint8_t

//#include <mbedtls/md.h>
#include <mbedtls/base64.h>
#include <mbedtls/pkcs5.h>
#include "pico/rand.h"
#include "PasswordHasher.h"

PasswordHasher::PasswordHasher(int iterations)
    : iterations(iterations) {}

std::vector<uint8_t> PasswordHasher::hashPassword(const std::string &password, const std::vector<uint8_t> &salt)
{
    std::vector<uint8_t> output(HASH_LENGTH);

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_setup(&ctx, md, 1); // 1 = HMAC

    mbedtls_pkcs5_pbkdf2_hmac(
        &ctx,
        reinterpret_cast<const uint8_t *>(password.data()), password.size(),
        salt.data(), salt.size(),
        iterations,
        HASH_LENGTH,
        output.data());

    mbedtls_md_free(&ctx);
    return output;
}

bool PasswordHasher::verifyPassword(const std::string &password, const std::vector<uint8_t> &salt, const std::vector<uint8_t> &expectedHash)
{
    auto actualHash = hashPassword(password, salt);
    return std::memcmp(actualHash.data(), expectedHash.data(), HASH_LENGTH) == 0;
}

std::string PasswordHasher::hashPasswordBase64(const std::string &password, const std::vector<uint8_t> &salt)
{
    return toBase64(hashPassword(password, salt));
}

bool PasswordHasher::verifyPasswordBase64(const std::string &password, const std::vector<uint8_t> &salt, const std::string &expectedBase64Hash)
{
    auto expected = fromBase64(expectedBase64Hash);
    return verifyPassword(password, salt, expected);
}

std::string PasswordHasher::toBase64(const std::vector<uint8_t> &data)
{
    size_t outputLen = 0;
    size_t bufferSize = ((data.size() + 2) / 3) * 4;
    std::vector<uint8_t> encoded(bufferSize + 1);

    mbedtls_base64_encode(encoded.data(), encoded.size(), &outputLen, data.data(), data.size());
    return std::string(reinterpret_cast<char *>(encoded.data()), outputLen);
}

std::vector<uint8_t> PasswordHasher::fromBase64(const std::string &base64)
{
    size_t outputLen = 0;
    std::vector<uint8_t> decoded(base64.length());

    mbedtls_base64_decode(decoded.data(), decoded.size(), &outputLen,
                          reinterpret_cast<const uint8_t *>(base64.data()), base64.size());

    decoded.resize(outputLen);
    return decoded;
}

std::vector<uint8_t> PasswordHasher::generateSalt(size_t length) {
    std::vector<uint8_t> salt(length);
    for (size_t i = 0; i < length; i += 4) {
        uint32_t randVal = get_rand_32();
        size_t copyLen = std::min(size_t(4), length - i);
        std::memcpy(salt.data() + i, &randVal, copyLen);
    }
    return salt;
}

std::string PasswordHasher::hashAndEncode(const std::string &password)
{
    auto salt = generateSalt(16); // Salt length of 16 bytes
    auto hashB64 = hashPasswordBase64(password, salt);
    auto saltB64 = toBase64(salt);
    return saltB64 + "$" + hashB64;
}

bool PasswordHasher::verifyEncoded(const std::string &password, const std::string &combinedSaltAndHash)
{
    auto sep = combinedSaltAndHash.find('$');
    if (sep == std::string::npos)
    {
        return false; // malformed input
    }

    std::string saltB64 = combinedSaltAndHash.substr(0, sep);
    std::string hashB64 = combinedSaltAndHash.substr(sep + 1);

    auto salt = fromBase64(saltB64);
    return verifyPasswordBase64(password, salt, hashB64);
}
