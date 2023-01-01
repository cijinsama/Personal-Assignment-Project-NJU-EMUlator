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

void event_for_timer(Event *ev, Context *c){
	ev->event = EVENT_IRQ_TIMER;
}

Context* __am_irq_handle(Context *c) {
	uintptr_t mscratch = 0;
	uintptr_t sp = (uintptr_t)c;
  asm volatile("csrr %0, mscratch" : "=r"(mscratch));
	sp += sizeof(Context);
	if(sp == mscratch) {c->np = 3;}
	else {c->np = 0;}
// 	printf("$sp = %08x, scrach = %08x\n", sp, mscratch);
// 	printf("& = %08x\n", &c);


// 	uintptr_t temp2 = c->gpr[2];
// 	assert(temp2 == mscratch);


	
	uintptr_t temp = 0;
  asm volatile("csrw mscratch, %0" : : "r"(temp));


	__am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
			case EXCP_Environment:	c->mepc += 4; event_for_excp_ecvironment(&ev, c); break;
			case IRQ_TIMER:					event_for_timer(&ev, c); break;
      default:								c->mepc += 4; ev.event = EVENT_ERROR; break;
    }
    c = user_handler(ev, c);
    assert(c != NULL);
  }
	__am_switch(c);
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
	context->mstatus = 0x88;
	context->gpr[10] = (uintptr_t) arg;//用a0传参数
	context->pdir = NULL;
	context->np = 3;
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
