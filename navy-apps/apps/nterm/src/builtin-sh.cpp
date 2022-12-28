#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static int cmd_help(char *args);
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
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

static char cmd_buffer[128];
static void sh_handle_cmd(const char *input) {
	strcpy(cmd_buffer, input);
	char *str = cmd_buffer;
	char *str_end = str + strlen(str);
	char* argv[8];

	/* extract the first token as the command */
	char *cmd = strtok(str, " ");
	if (cmd == NULL) {return;}
	int last_ind = strlen(cmd) - 1;
	while(cmd[last_ind] == '\n'||cmd[last_ind] == '\t'||cmd[last_ind] == ' '){
		cmd[last_ind] = '\0';
		last_ind--;
	}


	/* treat the remaining string as the arguments,
	 * which may need further parsing
	 */
	char *args = cmd + strlen(cmd) + 1;
	if (args >= str_end) {
		args = NULL;
	}

	int i;
	for (i = 0; i < NR_CMD; i ++) {
		if (strcmp(cmd, cmd_table[i].name) == 0) {
			if (cmd_table[i].handler(args) < 0) { return; }
			break;
		}
	}

	printf("debug 1\n");
	argv[0] = cmd;
	i = 1;
  char *arg = strtok(args, " ");
	while(arg != NULL){
		printf("debug 1\n");
		printf("%s\n", arg);
		argv[i] = arg;
		arg = strtok(args, " ");
	}
	argv[i] = NULL;


	printf("debug 2\n");
	//如果没有匹配，则直接执行指令(即运行对应的程序)
  execvp(cmd, argv);
	printf("debug : &argv = %p argv = %p *argv = %s", &argv, argv, *argv);
	return;

}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  setenv("PATH", "/usr/bin:/bin", 0);
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
