#include <stdint.h>
#include <string.h>
#include <stdio.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() {
	while(1){
		printf("This is dummy running\n");
	}
  return 0;
}
