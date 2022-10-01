#include <stdio.h>
#include <stdlib.h>
#define strlengen 1024
#define max_number 999990
#define uint32_t int
static char str[1024];
static int i = 0;
static char operator[] = {'+', '-', '*'};
uint32_t choose(uint32_t n) {
	uint32_t ans = rand();
	ans = ans % n;
	return ans;
}
void gen(char input) {
	str[i] = input;
	i++;
}
void gen_num() {
	uint32_t ans = rand();
	ans = ans % max_number;
	ans = ans + 1;
	while (ans != 0) {
		gen(ans % 10);
		ans = ans / 10;
	}
}
void gen_rand_op() {
	uint32_t ans = rand();
	ans = ans % (sizeof(operator)/sizeof(operator[0]));
	gen(operator[ans]);
}
void gen_rand_expr() {
	switch (choose(3)) {
		case 0: gen_num(); break;
		case 1: gen('('); gen_rand_expr(); gen(')'); break;
		default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
	}
}
int main() {
	for (int i = 0; i < strlengen; i++) {
		str[i] = '\0';
	}
	gen_rand_expr();
	unsigned result =  // 把???替换成表达式
	printf("%u", result);
	return 0;
}
