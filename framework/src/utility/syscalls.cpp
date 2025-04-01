
// This is just to quieten the linker
extern "C" {

    int _stat(const char *path, struct stat *st) {
        return -1;
    }
    
    int _unlink(const char *path) {
        return -1;
    }
    
    int _link(const char *old, const char *new_) {
        return -1;
    }
    
    int _gettimeofday(struct timeval *tv, void *tz) {
        // Optionally fill tv with current time if you have an RTC
        return -1;
    }
}