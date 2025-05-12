#include "storage/FatFsFileReader.h"
#include <cstring>
#include <ff_stdio.h>

FatFsFileReader::FatFsFileReader() = default;

FatFsFileReader::~FatFsFileReader() {
    close();
}

bool FatFsFileReader::open(const std::string& path) {
    file = ff_fopen(path.c_str(), "r");
    isOpen = (file != nullptr);
    return isOpen;
}


void FatFsFileReader::close() {
    if (isOpen && file) {
        ff_fclose(file);
        file = nullptr;
        isOpen = false;
    }
}


bool FatFsFileReader::readLine(char* outLine, size_t maxLen) {
    if (!isOpen || !file || maxLen == 0) return false;


    size_t count = 0;
    int ch;

    while (count < maxLen - 1) {
        ch = ff_fgetc(file);
        if (ch == EOF) break;
        if (ch == '\n') break;
        if (ch != '\r') {
            outLine[count++] = static_cast<char>(ch);
        }
    }

    if (count == 0 && ch == EOF) return false;

    outLine[count] = '\0';
    return true;
}


