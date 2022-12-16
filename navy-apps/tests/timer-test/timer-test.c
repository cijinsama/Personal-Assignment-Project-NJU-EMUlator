#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include "../../libs/libndl/include/NDL.h"
int main(){
	NDL_Init(1);
	uint32_t delta = 0;
	uint32_t last_time = NDL_GetTicks();
	uint32_t cur_time = NDL_GetTicks();
	while(1){
		cur_time = NDL_GetTicks();
		if (delta > 500) {
			printf("passed 0.5s\n");
			delta = 0;
		}
		else{
			delta += cur_time - last_time;
			last_time = cur_time;
		}
	}
	return 0;
}
