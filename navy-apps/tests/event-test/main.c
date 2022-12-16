#include <stdio.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
	int i = 0;
  while (i++ < 5) {
    char buf[64];
    if (NDL_PollEvent(buf, sizeof(buf))) {
      printf("receive event: %s\n", buf);
    }
  }
  return 0;
}
