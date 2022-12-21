#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
	int *a = malloc(i);
	printf("%x\n", a);
	free(a);
	i = 3;
	a = malloc(sizeof(int));
	int *b = malloc(i);
	printf("%x\n", a);
	printf("%x\n", b);
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
