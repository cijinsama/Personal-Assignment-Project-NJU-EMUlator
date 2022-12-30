#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main(int argc, char *argv[]) {
	for(int i = 0; i < argc; i++){
		printf("argv %d = %s", i, argv[i]);
	}
	int *a = malloc(sizeof(int));
	*a = 1;
	printf("%d\n",*a);
	int j = 0;
  while (1) {
    j ++;
		printf("Hello World from Navy-apps for the %dth time!\n", j ++);
		_syscall_(SYS_yield, 0, 0, 0);
  }
  return 0;
}
