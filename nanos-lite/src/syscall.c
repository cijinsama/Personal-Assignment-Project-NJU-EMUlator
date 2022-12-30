#include <common.h>
#include <fs.h>
#include "syscall.h"
#include <sys/time.h>
#include <memory.h>



void *do_sys_brk(intptr_t addr){
	mm_brk(addr);
	return 0;
}


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
	a[1] = c->GPR2;
	a[2] = c->GPR3;
	a[3] = c->GPR4;
#ifdef CONFIG_STRACE
	Log("[strace]: system call number: %x, args : %x, %x, %x", a[0], a[1], a[2], a[3]);
#endif
  switch (a[0]) {
// 		case SYS_exit: c->GPRx = do_sys_execve("/bin/nterm",(char *const *) a[2],(char *const *) a[3]); break;
		case SYS_exit: halt(0); break;
		case SYS_yield: yield(); c->GPRx = 0; break;
		case SYS_open: c->GPRx = do_sys_open((char *) a[1], a[2], a[3]); break;
		case SYS_read: c->GPRx = do_sys_read(a[1],(void *) a[2], a[3]); break;
		case SYS_write: c->GPRx = do_sys_write(a[1],(void *) a[2], a[3]); break;
		case SYS_close: c->GPRx = do_sys_close(a[1]); break;
		case SYS_lseek: c->GPRx = do_sys_lseek(a[1], a[2], a[3]); break;
		case SYS_brk:		c->GPRx = (uintptr_t) do_sys_brk(a[1]); break;
		case SYS_execve: c->GPRx = do_sys_execve((char *) a[1],(char *const *) a[2],(char *const *) a[3]); break;
		case SYS_gettimeofday: c->GPRx = do_sys_gettimeofday((struct timeval *) a[1], (struct timezone *) a[2]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
	return;
}
