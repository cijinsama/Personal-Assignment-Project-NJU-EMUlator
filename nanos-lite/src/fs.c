#include <common.h>
#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
	size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENT, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t valid_read(void *buf, size_t offset, size_t len) {
	ramdisk_read(buf, offset, len);
  return len;
}

size_t valid_write(const void *buf, size_t offset, size_t len) {
	ramdisk_write(buf, offset, len);
  return len;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
	[FD_EVENT] =	{"/dev/events", 0, 0, events_read, invalid_write, 0},
#include "files.h"
};

static int files_num;
void init_fs() {
	files_num = sizeof(file_table) / sizeof(Finfo);
  // TODO: initialize the size of /dev/fb
	for (int i = 3; i < files_num; i++){
		file_table[i].read = invalid_read;
		file_table[i].write = invalid_write;
		file_table[i].open_offset = 0;
	}
}

int do_sys_open(const char *path, int flags, int mode) {
	for (int i = 0; i < files_num; i++){
		if (strcmp(file_table[i].name, path) == 0){
// 			Log("open file %d", i);
			file_table[i].read = valid_read;
			file_table[i].write = valid_write;
			file_table[i].open_offset = 0;
			return i;
		}
	}
	Log("Can't find file : %s", path);
  return 0;
}

size_t do_sys_read(int fd, void *buf, size_t count) {
	file_table[fd].read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, count);
	file_table[fd].open_offset += count;
  return count;
}

int do_sys_close(int fd){
// 	Log("close file %d", fd);
	file_table[fd].read = invalid_read;
	file_table[fd].write = invalid_write;
  return 0;
}

size_t do_sys_lseek(int fd, size_t offset, int whence) {
// 	Log("get whence : %x, offset %x", whence, offset);
	switch (whence) {
		case SEEK_SET:
			file_table[fd].open_offset = offset;
			break;
		case SEEK_CUR:
			file_table[fd].open_offset += offset;
			break;
		case SEEK_END:
			file_table[fd].open_offset = file_table[fd].size + offset;
			break;
	}
  return file_table[fd].open_offset;
}

size_t do_sys_write(int fd, const void *buf, size_t count){
// 	Log("get write file : %d: %xB at %x", fd,count,file_table[fd].open_offset);
	size_t ret = count;
	ret = file_table[fd].write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, count);
	file_table[fd].open_offset += count;
	return ret;
}

int fs_open(const char *pathname, int flags, int mode){
	return do_sys_open(pathname, flags, mode);
}

size_t fs_read(int fd, void *buf, size_t len){
	return do_sys_read(fd, buf, len);
}

size_t fs_write(int fd, const void *buf, size_t len){
	return do_sys_write(fd, buf, len);
}

size_t fs_lseek(int fd, size_t offset, int whence){
	return do_sys_lseek(fd, offset, whence);
}
int fs_close(int fd){
	return do_sys_close(fd);
}

