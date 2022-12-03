#include <common.h>
// #include <intr.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
		case EVENT_YIELD: Log("correcly received interupt environment"); break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
