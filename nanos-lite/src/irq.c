#include <common.h>
#include "syscall.h"

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
		case EVENT_YIELD:			Log("irq.c get parameter: event_yield"); break;
		case EVENT_SYSCALL:		Log("irq.c get parameter: event_syscall"); do_syscall(c); break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
