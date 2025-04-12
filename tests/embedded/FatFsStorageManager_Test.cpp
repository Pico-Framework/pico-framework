#define CPPUTEST_MEM_LEAK_DETECTION_DISABLED 1
#include "CppUTest/TestHarness.h"
#include <vector>\

#include "StorageManager.h"
#include "JsonService.h"

TEST_GROUP(FatFsStorageManager)
{
    StorageManager storage;
    const std::string testFile = "/testfile.txt";
    const std::string renamedFile = "/renamed.txt";

    void setup() {
        printf("Setting up TEST_GROUP\n");
        storage.mount();
        storage.remove(testFile); // cleanup in case of leftover
        storage.remove(renamedFile);
    }

    void teardown() {
        storage.remove(testFile);
        storage.remove(renamedFile);
        storage.unmount();
    }
};

TEST(FatFsStorageManager, CanWriteAndReadBackFile)
{
    printf("Running CanWriteAndReadBackFile\n");
    std::vector<uint8_t> data = {'H', 'e', 'l', 'l', 'o'};
    
    bool result = storage.writeFile(testFile, data);
    printf("writeFile returned: %s\n", result ? "true" : "false");
    CHECK_TRUE(result);

    std::vector<uint8_t> read;
    CHECK_TRUE(storage.readFile(testFile, read));
    CHECK_EQUAL(data.size(), read.size());
    MEMCMP_EQUAL(data.data(), read.data(), data.size());
    printf("CanWriteAndReadBackFile completed successfully\n");
}

TEST(FatFsStorageManager, CanAppendToFile)
{
    printf("Running CanAppendToFile\n");
    std::vector<uint8_t> data1 = {'A', 'B', 'C'};
    std::vector<uint8_t> data2 = {'1', '2', '3'};

    CHECK_TRUE(storage.writeFile(testFile, data1));
    CHECK_TRUE(storage.appendToFile(testFile, data2.data(), data2.size()));

    std::vector<uint8_t> expected = data1;
    expected.insert(expected.end(), data2.begin(), data2.end());

    std::vector<uint8_t> result;
    CHECK_TRUE(storage.readFile(testFile, result));
    CHECK_EQUAL(expected.size(), result.size());
    MEMCMP_EQUAL(expected.data(), result.data(), expected.size());
    printf("CanAppendToFile completed successfully\n");
}

TEST(FatFsStorageManager, CanCheckFileExistence)
{
    printf("Running CanCheckFileExistence\n");
    CHECK_FALSE(storage.exists(testFile));
    std::vector<uint8_t> data = {'X'};
    storage.writeFile(testFile, data);
    CHECK_TRUE(storage.exists(testFile));
    printf("CanCheckFileExistence completed successfully\n");
}

TEST(FatFsStorageManager, CanGetFileSize)
{
    printf("Running CanGetFileSize\n");
    std::vector<uint8_t> data = {'A', 'B', 'C', 'D'};
    storage.writeFile(testFile, data);
    size_t size = storage.getFileSize(testFile);
    CHECK_EQUAL(data.size(), size);
    printf("CanGetFileSize completed successfully\n");
}

TEST(FatFsStorageManager, CanRenameAndRemoveFile)
{
    std::vector<uint8_t> data = {'Z'};
    storage.writeFile(testFile, data);
    CHECK_TRUE(storage.rename(testFile, renamedFile));
    CHECK_TRUE(storage.exists(renamedFile));
    CHECK_FALSE(storage.exists(testFile));
    CHECK_TRUE(storage.remove(renamedFile));
    CHECK_FALSE(storage.exists(renamedFile));
}

TEST(FatFsStorageManager, CanListDirectory)
{
    printf("Running CanListDirectory\n");
    std::vector<uint8_t> data = {'1'};
    storage.writeFile(testFile, data);

    std::vector<FileInfo> listing;
    CHECK_TRUE(storage.listDirectory("/", listing));

    bool found = false;
    for (auto& file : listing) {
        if (file.name == "testfile.txt") {
            found = true;
            break;
        }
    }
    CHECK_TRUE(found);
    printf("CanListDirectory completed successfully\n");
}

TEST(FatFsStorageManager, CanStreamFile)
{
    printf("Running CanStreamFile\n");
    std::vector<uint8_t> data(1024, 'x');
    storage.writeFile(testFile, data);

    size_t total = 0;
    CHECK_TRUE(storage.streamFile(testFile, [&](const uint8_t* chunk, size_t len) {
        total += len;
    }));

    CHECK_EQUAL(data.size(), total);
    printf("CanStreamFile completed successfully\n");
}

#include "CppUTest/CommandLineTestRunner.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"

#include "CppUTest/TestOutput.h"
#include <cstdio>


