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
	area.start = pcb->cp;
	area.end = pcb->cp + STACK_SIZE;

  Log("kload Jump to entry = %p",(void *)entry);

	Context *context = kcontext(area, entry, arg);
	pcb->cp = context;
	return;
}

void context_uload(PCB *pcb, char filename[],char *argv[],char *envp[]){
	Area area;
	area.start = pcb->cp;
	area.end = pcb->cp + STACK_SIZE;

  uintptr_t entry = loader(pcb, filename);
  Log("uload Jump to entry = %p",(void *)entry);

	//拷贝argv，envp
// 	((uint32_t *)area.start)[1]


	Context *context = ucontext(NULL, area,(void *) entry);
	pcb->cp = context;

	Log("The sp is supposed to be 0x%x", area.start);
	//gpr[2]是sp
  context->gpr[2]  = (uintptr_t) area.start;
	context->GPRx = (uintptr_t) area.start;
	return;
}

void init_proc() {
// 	context_uload(&pcb[0], "/bin/pal", NULL, NULL);
	context_uload(&pcb[1], "/bin/dummy", NULL, NULL);
	context_kload(&pcb[0], hello_fun, "cijin");
	
  switch_boot_pcb();

  Log("Initializing processes...");
}

Context* schedule(Context *prev) {
	current->cp = prev;
	//每次返回pcb中的第一个进程执行
	current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}

