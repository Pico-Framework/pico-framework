MEMORY
{
  LFS   (rx) : ORIGIN = 0x101E0000, LENGTH = 0x20000   /* 128 KB */
}

__flash_lfs_start = ORIGIN(LFS);
__flash_lfs_end   = ORIGIN(LFS) + LENGTH(LFS);


