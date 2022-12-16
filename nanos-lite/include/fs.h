#ifndef __FS_H__
#define __FS_H__

#include <common.h>

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

int do_sys_open(const char *path, int flags, int mode);
size_t do_sys_read(int fd, void *buf, size_t count);
int do_sys_close(int fd);
size_t do_sys_lseek(int fd, size_t offset, int whence);
size_t do_sys_write(int fd, const void *buf, size_t count);

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

#endif
