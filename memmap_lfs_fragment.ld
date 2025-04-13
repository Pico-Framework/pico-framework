MEMORY
{
    LFS_FLASH (rx) : ORIGIN = 0x101E0000, LENGTH = 128K
}

__flash_lfs_start = ORIGIN(LFS_FLASH);
__flash_lfs_end   = ORIGIN(LFS_FLASH) + LENGTH(LFS_FLASH);
