#include "storage_manager.hpp"
#include <ff_utils.h>
#include <ff_stdio.h>

class FatFsStorage : public StorageManager {
    public:
        bool mount() override {
            return ::mount("sd0");
        }
    
        bool unmount() override {
            ::unmount("sd0");
            return true;
        }
    
        bool fileExists(const std::string& path) override {
            FF_FILE* file = ff_fopen(path.c_str(), "r");
            if (file) {
                ff_fclose(file);
                return true;
            }
            return false;
        }
    
        bool listDirectory(const std::string& path, std::vector<std::string>& files) override {
            printf("Listing directory: %s\n", path.c_str());
        
            FF_FindData_t xFindStruct;
            memset(&xFindStruct, 0, sizeof(FF_FindData_t));
        
            // Ensure the path is correct
            std::string searchPath = path.empty() ? "/" : path;
            int result = ff_findfirst(searchPath.c_str(), &xFindStruct);
        
            if (result != FF_ERR_NONE) {
                printf("ff_findfirst failed: %d\n", result);
                return false;
            }
        
            do {
                if (strlen(xFindStruct.pcFileName) > 0) {
                    files.push_back(std::string(xFindStruct.pcFileName));
        
                    // Debug output to ensure it's working
                    printf("Found: %s [%s] [size=%lu]\n",
                           xFindStruct.pcFileName,
                           (xFindStruct.ucAttributes & FF_FAT_ATTR_DIR) ? "directory" : "file",
                           xFindStruct.ulFileSize);
                }
            } while (ff_findnext(&xFindStruct) == FF_ERR_NONE);
        
            return true;
        }
        
    
        bool readFile(const std::string& path, std::vector<uint8_t>& buffer) override {
            FF_FILE* file = ff_fopen(path.c_str(), "r");
            if (!file) return false;
    
            ff_fseek(file, 0, SEEK_END);
            long fileSize = ff_ftell(file);
            ff_fseek(file, 0, SEEK_SET);
    
            buffer.resize(fileSize);
            ff_fread(buffer.data(), 1, fileSize, file);
            ff_fclose(file);
            return true;
        }
    
        bool writeFile(const std::string& path, const std::vector<uint8_t>& data) override {
            FF_FILE* file = ff_fopen(path.c_str(), "w");
            if (!file) return false;
    
            ff_fwrite(data.data(), 1, data.size(), file);
            ff_fclose(file);
            return true;
        }
    
        bool deleteFile(const std::string& path) override {
            return ff_remove(path.c_str()) == FF_ERR_NONE;
        }
    };
    