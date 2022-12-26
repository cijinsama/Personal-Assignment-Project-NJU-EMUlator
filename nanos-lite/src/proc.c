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
    j ++;
    yield();
  }
}

void context_kload(void (*entry)(void *), void *arg, PCB *pcb){
	Area area;
	area.start = pcb->stack;
	area.end = pcb->stack + STACK_SIZE;
	Context *context = kcontext(area, entry, arg);
	pcb->cp = context;
	return;
}

void init_proc() {
	context_kload(hello_fun, NULL, &pcb[0]);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here

	naive_uload(current, "/bin/dummy");
}

Context* schedule(Context *prev) {
	current->cp = prev;
	current = &pcb[0];
  return current->cp;
}

