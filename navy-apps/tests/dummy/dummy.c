#include <stdint.h>
#include <string.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() {
// 	int i  = 1;
	while(1){
		printf("123\n");
// 		i++;
		_syscall_(SYS_yield, 0, 0, 0);
	}
  return _syscall_(SYS_yield, 0, 0, 0);
}
