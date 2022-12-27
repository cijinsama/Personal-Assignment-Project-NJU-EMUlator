#include <common.h>
#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    Log("arg is %s", ((char *) arg), j);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg){
	Area area;
	area.start = pcb->stack;
	area.end = pcb->stack + STACK_SIZE;

  Log("kload Jump to entry = %p",(void *)entry);

	Context *context = kcontext(area, entry, arg);
	pcb->cp = context;
	return;
}

void context_uload(PCB *pcb, char filename[],char *argv[],char *envp[]){
	Area area;
	area.start = heap.end - STACK_SIZE;
	area.end = heap.end;
	Log("start %x", area.start);
	Log("end %x", area.end);


	//假设main上面的argc从这个开始
	uintptr_t main_ebp = (uintptr_t)area.start - PGSIZE;
	char** environ = (char**) (main_ebp + 64);

	//搜索argcenv的大小，并且获得储存完过后的地址
	char* current_addr = area.end;
	char* env_str_addr = NULL;
	char* arg_str_addr = NULL;
	int argc = 0;
	int envc = 0;
	Log("1");
	Log("%s", argv[0]);
	if(argv){
		for(;argv[argc]!=NULL; argc++){
			current_addr -= strlen(argv[argc]);
		}
		env_str_addr = current_addr;
	}
	if(envp){
		for(;envp[envc]!=NULL; envc++){
			current_addr -= strlen(envp[envc]);
		}
		arg_str_addr = current_addr;
	}
	Log("2::::%x", argc);
	//把字符串copy进取
//   char *envp_ustack[envc];
//   char *argp_ustack[argc];
	current_addr = area.end;
	if(envp){
		for(int i = envc-1; i>=0 ; i--){
			current_addr -= strlen(envp[i]);
			strcpy(current_addr, envp[i]);
	// 		envp_ustack[i] = current_addr;
		}
	}
	Log("2.5");
	if(argv){
		for(int i = argc-1; i>=0 ; i--){
			current_addr -= strlen(argv[i]) + 1;
			strcpy(current_addr, argv[i]);
	// 		argp_ustack[i] = current_addr;
		}
	}

	Log("3");
	//把字符串对应的指针copy进取
	//首先正向copyenv
	if(envp){
		environ[0] = env_str_addr;
		for(int i = 1; i < envc; i++){
			environ[i] = environ[i-1] + strlen(envp[i]);
		}
	}
	environ[envc] = NULL;
	//然后正向copyargv
	char** argvp = environ - argc - 1;
	if(argv){
		argvp[0] = arg_str_addr;
		for(int i = 1; i < argc; i++){
			argvp[i] = argvp[i-1] + strlen(argv[i]);
		}
	}
	argvp[envc] = NULL;

	Log("4");
	//开始写下方的东西
	uintptr_t temp = main_ebp + 4;
	*(int *)(temp + 4) = argc;
	temp += sizeof(int);
	*(char ***)temp = argvp;
	temp += sizeof(char **);
	*(char ***) temp = environ;
	

  uintptr_t entry = loader(pcb, filename);
  Log("uload Jump to entry = %p",(void *)entry);

	//拷贝argv，envp


	Context *context = ucontext(NULL, area,(void *) entry);
	pcb->cp = context;

	Log("The sp is supposed to be 0x%x", main_ebp);
	Log("pcb->cp = 0x%x", context);
	//gpr[2]是sp
  context->gpr[2]  = main_ebp;
	context->GPRx = main_ebp + 4;
	Log("5");
	return;
}

void init_proc() {
// 	context_uload(&pcb[0], "/bin/pal", NULL, NULL);
	context_kload(&pcb[0], hello_fun, "cijin");
  char *argv1[] = {"liuyi", NULL};
  char *envp1[] = {NULL};
	context_uload(&pcb[1], "/bin/dummy", argv1, envp1);
	
  switch_boot_pcb();

  Log("Initializing processes...");
}

Context* schedule(Context *prev) {
	current->cp = prev;
	//每次返回pcb中的第一个进程执行
	current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}

