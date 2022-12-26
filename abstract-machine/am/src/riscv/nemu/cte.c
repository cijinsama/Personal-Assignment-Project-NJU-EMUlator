#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;


void event_for_excp_ecvironment(Event *ev, Context *c){
	if(c->GPR1 == -1){
		ev->event = EVENT_YIELD;
	}
	else{
		ev->event = EVENT_SYSCALL;
	}
}


Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
			case EXCP_Environment:	c->mepc += 4; event_for_excp_ecvironment(&ev, c); break;
      default:								c->mepc += 4; ev.event = EVENT_ERROR; break;
    }
    c = user_handler(ev, c);
    assert(c != NULL);
  }
  return c;
}


extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
	Context *context = kstack.end - sizeof(Context);
	context->mepc = (uintptr_t)	entry;
	//context->mstatus = 0x1880;
	context->mstatus = 0x1800;
	//context->gpu[10] = arg;
  return context;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
