#include <stdint.h>
#include <stdlib.h>
#include <assert.h>


#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
	uintptr_t ptr =(uintptr_t) args;
	int argc = *args;
	char **argv = *(char ***)(ptr+(uintptr_t) sizeof(int));
	char **envp = *(char ***)(ptr+ (uintptr_t)sizeof(int) +(uintptr_t) sizeof(char **));
	environ = envp;

	printf("&argc : %p\n", args);
	printf("argc = %d\n", argc);
	printf("&argv = %p\n", (char **)(ptr+(uintptr_t) sizeof(int)));
	printf("argv = %p\n", argv);
	printf("argv[0] = %p\n", argv[0]);
	printf("*argv[0] = %s\n", argv[0]);

  exit(main(argc, argv, envp));
  assert(0);
}
