/**
 * @file test_StorageManager.cpp
 * @brief Unified tests for StorageManager interface (LittleFs and FatFs)
 */

#include <CppUTest/TestHarness.h>
#include <vector>
#include <string>
#include "storage/StorageManager.h"

#if PICO_HTTP_ENABLE_LITTLEFS
#include "storage/LittleFsStorageManager.h"
static LittleFsStorageManager storageManager;
#else
#include "storage/FatFsStorageManager.h"
static FatFsStorageManager storageManager;
#endif

// Global pointer to base class
static StorageManager *storage = &storageManager;

static const std::string testFile = "test_file.txt";
static const std::string renamedFile = "renamed.txt";
static const std::string testContent = "Embedded test content";

TEST_GROUP(StorageManagerInterface){

    void setup(){

        CHECK_TRUE(storage->mount());
        storage->remove(testFile);
        storage->remove(renamedFile);
}

void teardown()
{
    storage->remove(testFile);
    storage->remove(renamedFile);
}
}
;


TEST(StorageManagerInterface, WriteAndReadBackFile)
{
    std::vector<uint8_t> data(testContent.begin(), testContent.end());
    CHECK_TRUE(storage->writeFile(testFile, data));

    std::vector<uint8_t> read;
    CHECK_TRUE(storage->readFile(testFile, read));
    STRCMP_EQUAL(testContent.c_str(), std::string(read.begin(), read.end()).c_str());
}

TEST(StorageManagerInterface, AppendToFile)
{
    CHECK_TRUE(storage->writeFile(testFile, reinterpret_cast<const uint8_t *>("abc"), 3));
    CHECK_TRUE(storage->writeFile(testFile, reinterpret_cast<const uint8_t *>("def"), 3));

    std::string out;
    CHECK_TRUE(storage->readFileString(testFile, 0, UINT32_MAX, out));
    STRCMP_EQUAL("abcdef", out.c_str());
}

TEST(StorageManagerInterface, ExistsAndRemove)
{
    CHECK_FALSE(storage->exists(testFile));
    CHECK_TRUE(storage->writeFile(testFile, reinterpret_cast<const uint8_t *>("x"), 1));
    CHECK_TRUE(storage->exists(testFile));
    CHECK_TRUE(storage->remove(testFile));
    CHECK_FALSE(storage->exists(testFile));
}

TEST(StorageManagerInterface, RenameFile)
{
    CHECK_TRUE(storage->writeFile(testFile, reinterpret_cast<const uint8_t *>("z"), 1));
    CHECK_TRUE(storage->rename(testFile, renamedFile));
    CHECK_TRUE(storage->exists(renamedFile));
    CHECK_FALSE(storage->exists(testFile));
}

TEST(StorageManagerInterface, FileSize)
{
    std::vector<uint8_t> data = {'a', 'b', 'c'};
    CHECK_TRUE(storage->writeFile(testFile, data));
    LONGS_EQUAL(3, storage->getFileSize(testFile));
}

TEST(StorageManagerInterface, ListDirectoryIncludesFile)
{
    std::vector<uint8_t> data = {'1'};
    CHECK_TRUE(storage->writeFile(testFile, data));

    std::vector<FileInfo> listing;
    CHECK_TRUE(storage->listDirectory("/", listing));

    bool found = false;
    for (auto &file : listing)
    {
        if (file.name == testFile || file.name == "test_file.txt")
        {
            found = true;
            break;
        }
    }
    CHECK_TRUE(found);
}

TEST(StorageManagerInterface, StreamFile)
{
    std::vector<uint8_t> data(512, 'x');
    CHECK_TRUE(storage->writeFile(testFile, data));

    size_t total = 0;
    CHECK_TRUE(storage->streamFile(testFile, [&](const uint8_t *chunk, size_t len)
                                   { total += len; }));

    LONGS_EQUAL(data.size(), total);
}

TEST(StorageManagerInterface, FormatClearsStorage)
{
    std::vector<uint8_t> data = {'y'};
    CHECK_TRUE(storage->writeFile(testFile, data));
    CHECK_TRUE(storage->exists(testFile));
    CHECK_TRUE(storage->formatStorage());
    CHECK_FALSE(storage->exists(testFile));
}
