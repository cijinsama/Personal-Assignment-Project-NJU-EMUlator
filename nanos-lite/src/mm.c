#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
	void *temp = pf;
	pf += nr_page * PGSIZE;//每次增加一个页的大小4kB
	printf("ret = %p", temp);
  return temp;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  panic("not implement yet");
  return NULL;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  panic("not implement yet");
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
