#include <common.h>
#include "syscall.h"

void *do_sys_brk(intptr_t addr){
	return 0;
}

size_t do_sys_write(int fd, const void *buf, size_t count){
	switch (fd) {
		case 1: 
		case 2: 
			for (int i = 0; i < count; ++i){
				Log("putchar %c at address %x", *((char *)buf + i), (char *)buf + i);
				putch(*((char *)buf + i));
			}
			break;
		default: panic("do_sys_write unfinished fd"); 
						 return -1;
	}
	return count;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
	a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
	Log("[strace]: system call number: %x, args : %x, %x, %x", a[0], a[1], a[2], a[3]);
  switch (a[0]) {
		case SYS_exit: halt(0); break;
		case SYS_yield: yield(); c->GPRx = 0; break;
		case SYS_write: c->GPRx = do_sys_write(a[1],(void *) a[2], a[3]); break;
		case SYS_brk:		c->GPRx = (uintptr_t) do_sys_brk(a[0]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
	return;
}
