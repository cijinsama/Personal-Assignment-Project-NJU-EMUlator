/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#define max_number 99
// this should be enough
static char buf[65536] = {};
static int tokens_num = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = (%s); "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static char operator[] = {'+', '-', '*'};

uint32_t choose(uint32_t n) {
	uint32_t ans = rand();
	ans = ans % n;
	return ans;
}
void gen(char input) {
	char strin[32];
	sprintf(strin,"%c",input);
	strcat(buf,strin);
}


void gen_num() {
	uint32_t ans = rand();
	char number_char[32];
	ans = ans % max_number;
	ans = ans + 1;
	sprintf(number_char,"%u",ans);
	strcat(buf,number_char);
	tokens_num++;
}
void gen_rand_op() {
	uint32_t ans = rand();
	ans = ans % (sizeof(operator)/sizeof(operator[0]));
	gen(operator[ans]);
	tokens_num++;
}

void gen_rand_expr() {
	switch (choose(3)) {
		case 0: gen_num(); tokens_num++; break;
		case 1: gen('('); gen_rand_expr(); gen(')'); tokens_num+=2; break;
		default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
	}
	if (tokens_num >= 32) {
		return;
	}
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
		int temp = 1, temp2 = 2;
		temp = temp2 + temp;
		temp2 = temp + temp2;
    temp = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
