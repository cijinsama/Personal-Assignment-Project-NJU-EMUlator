#include <common.h>
#include <proc.h>
#include <memory.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

PCB *get_free_PCB(){
	for(int i = 0; i < sizeof(pcb)/sizeof(pcb[0]); i++){
		if(pcb[i].cp == NULL)	return &pcb[i];
	}
	return NULL;
}


void switch_boot_pcb() {
	current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
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

#define gap_between 4
#define gap_between_context_string 64
#define gap_between_main_context 512
#define gap_between_main_env 128
void context_uload(PCB *pcb, char filename[],char *argv[],char *envp[]){
	protect(&pcb->as);
  void *user_stack_top = new_page(STACK_SIZE / PGSIZE) + STACK_SIZE;
	Log("user_stack_top %08x - %08x",user_stack_top - STACK_SIZE ,user_stack_top);
	for(int i = 0; i < (STACK_SIZE / PGSIZE); i++){
		map(&pcb->as, pcb->as.area.end - (i+1) * PGSIZE, user_stack_top - (i+1) * PGSIZE, 0); 
	}


	Area area;//这一段area指代用户数据区域
	area.end = user_stack_top;
	area.start = area.end - STACK_SIZE;

	//假设main上面的argc从这个开始
	uintptr_t main_ebp = (uintptr_t)area.end - gap_between_context_string - gap_between_main_context;//这两个地方用到了end
	char** environ = (char**) (main_ebp + gap_between_main_env);

	//搜索argcenv的大小，并且获得储存完过后的地址
	char* current_addr = area.end - gap_between_context_string;//这两个地方用到了end
	char* env_str_addr = NULL;
	char* arg_str_addr = NULL;
	int argc = 0;
	int envc = 0;
	if(argv){
		for(;argv[argc]!=NULL; argc++){
			current_addr -= strlen(argv[argc]) + gap_between;
		}
	}
	env_str_addr = current_addr;
	if(envp){
		for(;envp[envc]!=NULL; envc++){
			current_addr -= strlen(envp[envc]) + gap_between;
		}
	}
	arg_str_addr = current_addr;
	//把字符串copy进取
  char *envp_ustack[envc];
  char *argp_ustack[argc];
	current_addr = env_str_addr;
	if(envp){
		for(int i = 0; i < envc; i++){
			strcpy(current_addr, envp[i]);
	 		envp_ustack[i] = current_addr;
			Log("uload envp[%d]: %s at %p - %p ,gap to %p", i,current_addr, current_addr, current_addr + strlen(envp[i]),current_addr + strlen(envp[i]) + gap_between );
			current_addr += strlen(envp[i]) + gap_between;
		}
	}
	current_addr = arg_str_addr;
	if(argv){
		for(int i = 0; i < argc ; i++){
			strcpy(current_addr, argv[i]);
	 		argp_ustack[i] = current_addr;
			Log("uload argv[%d]: %s at %p - %p ,gap to %p", i,current_addr, current_addr, current_addr + strlen(argv[i]),current_addr + strlen(argv[i]) + gap_between );
			current_addr += strlen(argv[i]) + gap_between;
		}
	}

	//把字符串对应的指针copy进取
	//首先正向copyenv
	if(envp){
		for(int i = 0; i < envc; i++){
			environ[i] = envp_ustack[i];
			Log("uload environ[%d]: %p at %p", i,envp_ustack[i], &envp_ustack[i]);
		}
	}
	environ[envc] = NULL;
	//然后正向copyargv
	char** argv_ = environ - argc - 1;
	if(argv){
		for(int i = 0; i < argc; i++){
			argv_[i] = argp_ustack[i];
		}
	}
	argv_[argc] = NULL;


	//开始写下方的东西
	uintptr_t temp = main_ebp + 4;
	*(int *)temp  = argc;
	temp += sizeof(int);
	*(char ***)temp = argv_;
	temp += sizeof(char **);
	*(char ***) temp = environ;

  uintptr_t entry = loader(pcb, filename);
  Log("uload Jump to entry = %p",(void *)entry);

	area.start = &pcb->cp;//从这开始area指代karea，储存context的
	area.end = area.start + STACK_SIZE;
	Log("karea start %08x- %08x", area.start, area.end);
	Context *context = ucontext(&pcb->as, area,(void *) entry);
	pcb->cp = context;

	context->GPRx = main_ebp + 4;
	return;
}

static int front_proc = 1;

void switch_prog(int index){
	front_proc = index;
}

Context* schedule(Context *prev) {
	current->cp = prev;
	current = (current == &pcb[0] ? &pcb[front_proc] : &pcb[0]);
// 	int i = 0;
// 	for(i = 0; i < MAX_NR_PROC; i++){
// 		if(pcb[i].cp != NULL && current != &pcb[i]){
// 			Log("switch to pcb %d", i);
// 			current = &pcb[i];
// 			return current->cp;
// 		}
// 	}
// 	if(i == MAX_NR_PROC) panic("lack of available pcb");
  return current->cp;
}


size_t execve(const char * filename, char *const argv[], char *const envp[]){
	Log("loading program : %s", filename);
	Log("&argv = %p, argv = %p, &envp = %p, envp = %p", &argv, argv, &envp, envp);
	for(int i = 0; i < 4 && argv[i] != NULL; i++){
		Log("argv %d = %s", i ,argv[i]);
	}
	for(int i = 0; i < 4 && envp[i] != NULL; i++){
		Log("envp %d = %s", i ,envp[i]);
	}
	PCB* newpcb = get_free_PCB();
	if(newpcb) context_uload(newpcb, (char *) filename, (char **) argv, (char **) envp);
	else panic("lack of free pcb");
	current->cp = NULL;
	switch_boot_pcb();
	yield();
	return 0;
}

#define prog_hello "/bin/hello"
#define prog_nterm "/bin/nterm"
#define prog_exectest "/bin/exec-test"
#define prog_pal "/bin/pal"
#define prog_dummy "/bin/dummy"
#define prog_bird "/bin/bird"

void init_proc() {
  Log("Initializing processes...");
	context_kload(&pcb[0], hello_fun, "cijin");


  char *argv1[] = {prog_pal,"--skip" ,NULL};
  char *envp1[] = {NULL};
	context_uload(&pcb[1], prog_pal, argv1, envp1);



  char *argv2[] = {prog_nterm, NULL};
  char *envp2[] = {NULL};
	context_uload(&pcb[2], prog_nterm, argv2, envp2);
	
  char *argv3[] = {prog_pal, NULL};
  char *envp3[] = {NULL};
	context_uload(&pcb[3], prog_pal, argv3, envp3);

  switch_boot_pcb();

  Log("Complete Initializing processes...");
}

