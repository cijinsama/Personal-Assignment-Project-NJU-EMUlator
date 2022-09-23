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
#include <memory/paddr.h>
#include "sdb.h"
static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

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
	if (*point == '-'){ return -1; }
	for (; *point <= '9' && *point >= '0' ; point++){
		number = number*10 + *point-'0';
	}
	cpu_exec(number);
	return 0;
}

static int cmd_info(char *args) {
	if (strlen(args) != 1) {
		printf("args error\n");
		return 1;
	}
	if (*args == 'r'){
		isa_reg_display();
		return 0;
	}
	else if (*args == 'w') {
	//TODO
	}
	printf("args error\n");
	return 1;
}


static int cmd_x(char *args) {
	char *n = strtok(args, " ");
	if (n == NULL) {
		printf("lack of arg1\n");
		return 1;
	}

	args += strlen(n);
	char *exp = strtok(NULL," ");
	if (exp == NULL) {
		printf("lack of arg2\n");
		return 1;
	}

	//if (!exists(exp)) {
	//	printf("Don't exist such exp\n");
	//	return 1;
	//}
	int num = 0;
	int i;
	char tem;
	for (i=0; i<strlen(n); i++) {
		if (n[i] <= '9' && n[i] >= '0'){
			num = num*10 + n[i] - '0';
		}
		else {printf("input args error"); return 0;}
	}
	
	paddr_t addr= 0;
	/*解析地址：将string转换成paddr_t */	
	for (i=2; i<strlen(exp); i++) {
		tem = exp[i];
		if (tem >= '0' && tem <= '9') {addr = addr*16 + tem - '0';}
		else if (tem >= 'a' && tem <= 'f') {addr = addr*16 + tem - 'a' + 10;}
		else {printf("error input\n");}
	}

	/* 输出地址对应的数据 */
	for (i=0; i< num; i++, addr+=4) {
		//printf("%08x",(uint32_t)paddr_read(addr ,4));
		printf("0x%08x\n",(uint32_t)paddr_read(addr ,4));
	}
	printf("\n");	
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
	{ "x", "scan the memory for given addr string", cmd_x },

  /* TODO: Add more commands */

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
