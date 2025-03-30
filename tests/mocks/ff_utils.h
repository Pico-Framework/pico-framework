#ifndef DUMMY_FF_UTILS_H
#define DUMMY_FF_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define FF_ERR_NONE 0

// File attribute macros
#define FF_FAT_ATTR_DIR      0x10
#define FF_FAT_ATTR_READONLY 0x01

// Dummy file structure
typedef struct FF_FILE {
    int dummy; // dummy member to represent a file handle
} FF_FILE;

// Dummy structure for file/directory information
typedef struct FF_FindData_t {
    const char* pcFileName;    // File name
    unsigned long ulFileSize;  // File size
    unsigned char ucAttributes;// File attributes
} FF_FindData_t;

// Stub for mounting a filesystem
inline int mount(const char* mountPoint) {
    // Simulate a successful mount.
    return 0;
}

// Stub for unmounting a filesystem
inline int unmount(const char* mountPoint) {
    // Simulate a successful unmount.
    return 0;
}

// Stub for opening a file.
// Returns a pointer to a dummy file structure.
inline FF_FILE* ff_fopen(const char* path, const char* mode) {
    static FF_FILE dummyFile;
    return &dummyFile;
}

// Stub for closing a file.
// Accepts an FF_FILE pointer and simulates closure.
inline int ff_fclose(FF_FILE* file) {
    // Dummy implementation: no action.
    return 0;
}

// Stub for finding the first file in a directory.
// Sets dummy values in the provided FF_FindData_t structure.
inline int ff_findfirst(const char* searchPath, FF_FindData_t* findData) {
    if (findData) {
        // For testing, simulate an empty result by providing an empty file name.
        findData->pcFileName = "";
        findData->ulFileSize = 0;
        findData->ucAttributes = 0;
    }
    return FF_ERR_NONE;
}

// Stub for finding the next file in a directory.
// Returns a non-zero value to indicate that there are no more files.
inline int ff_findnext(FF_FindData_t* findData) {
    return 1; // Non-zero to indicate no more files.
}

// Stub for seeking within a file.
// Does nothing and returns success.
inline int ff_fseek(FF_FILE* file, long offset, int whence) {
    return 0;
}

// Stub for telling the current file position.
// Returns 0 as a dummy value.
inline long ff_ftell(FF_FILE* file) {
    return 0;
}

// Stub for reading from a file.
// Returns 0 to simulate that no data was read.
inline size_t ff_fread(void* buffer, size_t size, size_t count, FF_FILE* file) {
    return 0;
}

// Stub for writing to a file.
// Returns count to simulate that all data was written successfully.
inline size_t ff_fwrite(const void* buffer, size_t size, size_t count, FF_FILE* file) {
    return count;
}

// Stub for removing a file.
// Simulates success by returning FF_ERR_NONE.
inline int ff_remove(const char* path) {
    return FF_ERR_NONE;
}

// Stub for renaming a file.
// Simulates success by returning 0.
inline int ff_rename(const char* from, const char* to, bool force) {
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif // DUMMY_FF_UTILS_H
