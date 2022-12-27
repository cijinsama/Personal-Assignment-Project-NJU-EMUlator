#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	printf("argc = %d\n", argc);
  int j = 1;
	for(int i = 0; i < argc; i++){
		printf("argv %d = %s", i, argv[i]);
	}
	j ++;

//   while (1) {
//     j ++;
//     if (j == 10000) {
//       printf("Hello World from Navy-apps for the %dth time!\n", i ++);
//       j = 0;
//     }
//   }
  return 0;
}
