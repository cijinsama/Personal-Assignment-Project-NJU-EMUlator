#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
	Log("[STRACE]: system call number: %x", a[0]);
  switch (a[0]) {
		case SYS_yield: yield(); c->GPRx = 0; return;
		case SYS_exit: halt(0); return;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
