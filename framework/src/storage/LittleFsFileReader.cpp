#include "storage/LittleFsFileReader.h"
#include <string>

LittleFsFileReader::LittleFsFileReader(lfs_t *lfs)
    : lfs(lfs) {}

bool LittleFsFileReader::open(const std::string &path)
{
    if (!lfs)
        return false;
    isOpen = (lfs_file_open(lfs, &file, path.c_str(), LFS_O_RDONLY) == 0);
    if (!isOpen)
    {
        return false;
    }
    return isOpen;
}

LittleFsFileReader::~LittleFsFileReader()
{
    close();
}

bool LittleFsFileReader::readLine(char *outLine, size_t maxLen)
{
    if (!isOpen || maxLen == 0)

        return false;

    size_t count = 0;
    uint8_t ch;
    int result;

    while (count < maxLen - 1)
    {
        result = lfs_file_read(lfs, &file, &ch, 1);

        if (result <= 0)
            break;
        if (ch == '\n')
            break;
        if (ch != '\r')
        {
            outLine[count++] = static_cast<char>(ch);
        }
    }

    if (count == 0 && result <= 0)
        return false;

    outLine[count] = '\0';
    return true;
}

void LittleFsFileReader::close()
{
    if (isOpen) {
        lfs_file_close(lfs, &file);
        isOpen = false;
    }
}
