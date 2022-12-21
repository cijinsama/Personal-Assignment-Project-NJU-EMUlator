#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
	int *a = malloc(127);
	int *b = malloc(0);
	printf("%x\n", a);
	printf("%x\n", b);
	printf("%x\n", b - a);
	free(a);
	free(b);
	a = malloc(129);
	b = malloc(0);
	printf("%x\n", a);
	printf("%x\n", b);
	printf("%x\n", b - a);
	free(a);
	free(b);

//   while (1) {
//     j ++;
//     if (j == 10000) {
//       printf("Hello World from Navy-apps for the %dth time!\n", i ++);
//       j = 0;
//     }
//   }
  return 0;
}
