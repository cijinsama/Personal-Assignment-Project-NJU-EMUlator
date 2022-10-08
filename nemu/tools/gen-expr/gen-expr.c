#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MAX_NUMBER 10
#define MAX_DEPTH 5


static char buf[70000] = {};
static char str_buffer[70000] = {};
static char code_buffer[100000] = {};
static int ans = 0;
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


int choose(int  n){
	if (n < 0) {
		assert(0);
	}
  return rand() % n;
}

bool gen(char expr){
  char cha_buffer[2];
	cha_buffer[0] = c;
	cha_buffer[1] = '\0';
  strcat(buf, cha_buffer);
	return true;
}

bool gen_rand_op(){
  switch (choose(4)){
    case 0:
      gen('*');
			break;
    case 1:
      gen('/');
			break;
    case 2:
      gen('-');
			break;
    case 3:
      gen('+');
			break;
  }
  return true;
}

int gen_num(){
  char num_buffer[1000];
  num_buffer[0] = '\0';
  int number;
	number = rand() % MAX_NUMBER  + 1;
	switch(choose(4)){
		case 0:
		case 1:
		case 2:
			sprintf(num_buffer ,"%u", number);
			break;
		case 3:
			sprintf(num_buffer ,"-%u", number);
			break;
	}
  strcat(buf, num_buffer);
  return number;
}

void generate_output(){
  int j = 0;
  for (int i = 0; buf[i] != '\0'; ++i){
    if (buf[i] != 'u'){
      str_buffer[j++] = buf[i];
    }
  }
  str_buffer[j] = '\0';
}


static void gen_rand_expr(int depth) {
  if (strlen(buf) > 65000 || depth > MAX_DEPTH){
    gen('(');
    gen_num();
    gen(')');
    return ;
  }

  switch (choose(3)) {
    case 0: 
      gen_num();
      break; 
    case 1:
      gen('(');
      gen_rand_expr(depth + 1);
      gen(')');
      break;
    default: {
      gen_rand_expr(depth + 1);
      gen_rand_op();
      gen_rand_expr(depth + 1);
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  //stderr = NULL;
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';
    gen_rand_expr(0);
    generate_output();
    
    sprintf(code_buffer, code_format, buf);
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buffer, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr -Werror 2> /tmp/.error.txt");
    if (ret != 0){ 
      i--;
      continue;
    }

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    uint32_t result = 0u;
    int b = fscanf(fp, "%u", &result);
    pclose(fp);
    if (b != 1){
      i--;
      continue;
    }

    printf("%u %s\n", result, str_buffer);
  }
  return 0;
}
