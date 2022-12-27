#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main(int argc, char *argv[]) {
	printf("argc = %d\n", argc);
	printf("&argv = %p\n", &argv);
	printf("argv = %p\n", argv);
	printf("argv[0] = %p\n", argv[0]);
	printf("*argv[0] = %s\n", argv[0]);
	for(int i = 0; i < argc; i++){
		printf("argv %d = %s", i, argv[i]);
	}

//   while (1) {
//     j ++;
//     if (j == 10000) {
//       printf("Hello World from Navy-apps for the %dth time!\n", i ++);
//       j = 0;
//     }
//   }
//   return _syscall_(SYS_yield, 0, 0, 0);
	return 0;
}
