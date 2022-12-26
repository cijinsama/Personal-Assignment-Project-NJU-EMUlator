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

void context_uload(PCB *pcb, char filename[]){
	Area area;
	area.start = heap.end - STACK_SIZE;
	area.end = heap.end;

  uintptr_t entry = loader(pcb, filename);
  Log("uload Jump to entry = %p",(void *)entry);

	Context *context = ucontext(NULL, area,(void *) entry);
	pcb->cp = context;
	return;
}

void init_proc() {
	context_uload(&pcb[0], "/bin/nterm");
	context_kload(&pcb[1], hello_fun, "cijin");
	
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
// 	naive_uload(current, "/bin/dummy");
	context_uload(&pcb[0], "/bin/nterm");
}

Context* schedule(Context *prev) {
	current->cp = prev;
	//每次返回pcb中的第一个进程执行
	current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}

