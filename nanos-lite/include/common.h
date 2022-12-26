#ifndef __COMMON_H__
#define __COMMON_H__

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_CTE
//#define HAS_VME
//#define MULTIPROGRAM
//#define TIME_SHARING

#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <debug.h>
#include <sys/time.h>

void do_syscall(Context *c);
Context* schedule(Context *prev);
	
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
//# define CONFIG_STRACE
//#define CONFIG_RAMTRACE

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t do_sys_gettimeofday(struct timeval * tv, struct timezone * tz);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t do_sys_execve(const char * filename, char *const argv[], char *const envp[]);
#endif
