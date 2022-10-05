#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MAX_NUMBER 10
#define MAX_DEPTH 5



// this should be enough
static char buf[65536] = {};
static char str_buffer[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";


uint32_t choose(uint32_t  n){
  return rand() % n;
}

void gen(char c){
  char cha_buffer[2] = {c, '\0'};
  strcat(buf, cha_buffer);
}

char gen_rand_op(){
  switch (choose(4)){
    case 0:
      gen('+');
      return '+';
    case 1:
      gen('-');
      return '-';
    case 2:
      gen('*');
      return '*';
    case 3:
      gen('/');
      return '/';
  }
  return ' ';
}

uint32_t gen_num(){
  char num_buffer[1000];
  num_buffer[0] = '\0';
  uint32_t number;
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

static void gen_rand_blank(){
  switch (choose(3))
  { 
  case 0:
    strcat(buf, " ");
    break;
  case 1:
  case 2:
    break;
  }
}

static void gen_rand_expr(int depth) {
  if (strlen(buf) > 65536 - 10000 || depth > MAX_DEPTH){
    gen('(');
    gen_rand_blank();
    gen_num();
    gen_rand_blank();
    gen(')');
    return ;
  }

  switch (choose(3)) {
    case 0: 
      gen('(');
      gen_rand_blank();
      gen_rand_expr(depth + 1);
      gen_rand_blank();
      gen(')');
      break; 
    case 1:
      gen_num();
      gen_rand_blank(); 
      break;
    default: {
      gen_rand_expr(depth + 1);
      gen_rand_blank();
      gen_rand_op();
      gen_rand_blank();
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
    
    sprintf(code_buf, code_format, buf);
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
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
