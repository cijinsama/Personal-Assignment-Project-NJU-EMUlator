#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
int main(){
	struct timeval tv;
	struct timezone tz;
	uint64_t delta = 0;
	gettimeofday(&tv, &tz);
	uint64_t last_time = tv.tv_usec;
	while(1){
		gettimeofday(&tv, &tz);
		if (delta > 500000) {
			printf("passed 0.5 ms\n");
			delta = 0;
		}
		else{
			delta += tv.tv_usec - last_time;
			last_time = tv.tv_usec;
		}
	}
	return 0;
}
