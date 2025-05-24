MEMORY
{
  LFS   (rx) : ORIGIN = 0x103C0000, LENGTH = 0x40000   /* 256 KB */
}

__flash_lfs_start = ORIGIN(LFS);
__flash_lfs_end   = ORIGIN(LFS) + LENGTH(LFS);


