#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
	int *a = malloc(i);
	int *b = malloc(i);
	int *c = malloc(i+1);
	int *d = malloc(i+1);
	printf("%x\n", a);
	printf("%x\n", b);
	printf("%x\n", b - a);
	printf("%x\n", c);
	printf("%x\n", c - b);
	printf("%x\n", d);
	printf("%x\n", d - c);
	free(a);
	free(b);
	free(c);

//   while (1) {
//     j ++;
//     if (j == 10000) {
//       printf("Hello World from Navy-apps for the %dth time!\n", i ++);
//       j = 0;
//     }
//   }
  return 0;
}
