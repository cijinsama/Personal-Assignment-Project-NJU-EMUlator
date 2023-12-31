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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/vaddr.h>
#include "sdb.h"
#define ERROR_GOON 1
static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

WP* watcher_head = NULL;


#define dataset "./test_p"
static int test_cmd_p() {
  int counter = 0;
  char buffer[65535]; 
	int ans = 0;
	bool success = false;
  char *expression;
	int result;
  FILE *fp = fopen(dataset, "r");
	if (fp == NULL) {
		fprintf(stderr,"open dataset file error\n");
		panic();
	}
  char* input = fgets(buffer, ARRLEN(buffer), fp);
  while (input != NULL){
    input[strlen(input) - 1] = '\0';
    char* ans_text = strtok(input, " ");
    sscanf(ans_text, "%u", &ans);
    expression = input + strlen(ans_text) + 1;
		result = expr(expression, &success);
		if (result != ans) {
			printf("error expression \n%s\n",expression);
			printf("expected: %s\nbut got %u\n",ans_text,result);
			assert(result == ans);
		}
    input = fgets(buffer, ARRLEN(buffer), fp);
    counter++;
		if (counter %10 == 0) {
			printf("%d/unknown\n",counter);
		}
  }
	printf("\npassed %d\n",counter);
	return 0;
}


/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_si(char *args) {
	char *point=args; 
	uint64_t number = 0; 
	if (args == NULL) {
		number = 1;
	}
	else if (*point == '-'){ return ERROR_GOON; }
	else {
		for (; (*point <= '9') && (*point >= '0') ; point++){
			number = number*10 + *point-'0';
		}
	}
	cpu_exec(number);
	return 0;
}

static int cmd_px(char *args) {
	bool success;
	uint32_t ans = 0;
	ans = expr(args, &success);
	if (!success) {
		printf("please retry\n");
		return ERROR_GOON;
	 }
	else {
		printf("0x%08x\n",ans);
		return 0;
	} 
	return ERROR_GOON;
} 


static int cmd_p(char *args) {
	bool success;
	uint32_t ans = 0;
	ans = expr(args, &success);
	if (!success) {
		printf("please retry\n");
		return ERROR_GOON;
	 }
	else {
		printf("%u\n",ans);
		return 0;
	} 
	return ERROR_GOON;
}


static int cmd_info(char *args) {
	if (args == NULL) {
		printf("please input 'r' or 'w'\n");
		return ERROR_GOON;
	}
	if (strlen(args) != 1) {
		printf("args error\n");
		return ERROR_GOON;
	} 
	else if (*args == 'r'){
		isa_reg_display();
		return 0;
	} 
	else if (*args == 'w') {
		WP* temp = watcher_head;
		while (temp != NULL){
			printf("Hardware watchpoint %d : %s\n", temp->NO, temp->expr);
			printf("value :\n");
			cmd_p(temp->expr);
			cmd_px(temp->expr);
			temp = temp->next;
		}
		return 0;
	}
	printf("args error\n");
	return ERROR_GOON;
}


static int cmd_x(char *args) {
	char *n = strtok(args, " ");
	if (n == NULL) {
		printf("lack of arg1\n");
		return ERROR_GOON;
	} 

	args += strlen(n);
	char *exp = strtok(NULL," ");
	if (exp == NULL) {
		printf("lack of arg2\n");
		return ERROR_GOON;
	} 

	int num = 0;
	int i;
	char tem;
	for (i=0; i<strlen(n); i ++) {
		if (n[i] <= '9' && n[i] >= '0'){
			num = num*10 + n[i] - '0';
		} 
		else {printf("input args error\n"); return ERROR_GOON;}
	}
	
	vaddr_t addr= 0;
	/*解析地址：将string转换成paddr_t 地址为0x开头*/	
	for (i=2; i<strlen(exp); i++) {
		tem = exp[i];
		if (tem >= '0' && tem <= '9') {addr = addr*16 + tem - '0';}
		else if (tem >= 'a' && tem <= 'f') {addr = addr*16 + tem - 'a' + 10;}
		else {printf("error input\n");}
	} 

	/* 输出地址对应的数据  */
	for (i=0; i< num; i++, addr+=4) {
		printf("0x%08x\n",vaddr_read(addr ,4));
	} 
	printf("\n");	
	return 0;
}

 

static void print_pc(int i, char *out_str) {
	if (cpu.pc + i * 4 < 0x80000000) {
		return;
	}
	sprintf(out_str, "0x%08x", vaddr_read(cpu.pc + i*4, 4));
	if (i == 0) printf("---> 0x%08x : %s\n",cpu.pc + i*4, out_str);
	else				printf("     0x%08x : %s\n",cpu.pc + i*4, out_str);
	return;
}

static int cmd_list(char *args) {
	char out_str[30];
	for(int i = -3; i <= 3; i++) {
		print_pc(i,out_str);
	}
	return 0;
}

static int cmd_w(char *args) {
	watcher_head = new_wp( args ); 
	return 0;
}

static int cmd_d(char *args) {
	int single_number = 0;
	for (int i=0; args[i] != '\0'; i++){
		single_number = single_number*10 + args[i] - '0';
	}
	WP* temp_watchpoint_pointer = watcher_head;
	while(temp_watchpoint_pointer != NULL) {
		if (temp_watchpoint_pointer->NO == single_number) break;
		temp_watchpoint_pointer = temp_watchpoint_pointer->next;
	}	
	if(temp_watchpoint_pointer != NULL) {
		free(temp_watchpoint_pointer);
	}
	else{
		printf("does't exist this watchpoint\n");
	}

	return 0;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
	nemu_state.state = NEMU_QUIT;
  return -1;
}


static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
	{ "si", "Execute n steps of the program then stop", cmd_si },
	{ "info", "Print status", cmd_info },
	{ "x", "scan the memory for given addr string,\nused as 'x 4 0x80000000',\nthe first arg stands for how many word you want, the second arg stands for the first of the addrs", cmd_x },
	{ "p", "evaluate the expr", cmd_p },
	{ "w", "set a new watcher", cmd_w },
	{ "d", "del a watcher", cmd_d },
	{ "px", "evaluate the expr printf 0x***", cmd_px},
	{ "test_cmd_p", "test_cmd_p", test_cmd_p},
	{ "list", "list the code around current pc", cmd_list}
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
