#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

static int files_num;
void init_fs() {
	files_num = sizeof(file_table) / sizeof(Finfo);
  // TODO: initialize the size of /dev/fb
	for (int i = 3; i < files_num; i++){
		Log("write : %x", file_table[i].write);
		Log("%s", file_table[i].name);
		panic("看看这里输出什么");
	}
}

int do_sys_open(const char *path, int flags, int mode) {
	for (int i = 0; i < files_num; i++){
		if (strcmp(file_table[i].name, path) == 0){
// 			file_table[i].read = 
			return i;
		}
	}
  return 0;
}

size_t do_sys_read(int fd, void *buf, size_t count) {
  return 0;
}

int do_sys_close(int fd) {
  return 0;
}

size_t do_sys_lseek(int fd, size_t offset, int whence) {
  return 0;
}

size_t do_sys_write(int fd, const void *buf, size_t count){
	switch (fd) {
		case 1: 
		case 2: 
			for (int i = 0; i < count; ++i){
				putch(*((char *)buf + i));
			}
			break;
		default: panic("do_sys_write unfinished fd"); 
						 return -1;
	}
	return count;
}
