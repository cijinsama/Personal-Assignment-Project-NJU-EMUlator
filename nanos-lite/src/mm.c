#include <memory.h>
#include <proc.h>

extern PCB *current;
static void *pf = NULL;//空闲物理页的首地址

void* new_page(size_t nr_page) {
	void *temp = pf;
	pf += nr_page * PGSIZE;//每次增加一个页的大小4kB
  return temp;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {//n是字节大小
  void *ret = new_page(n / PGSIZE);
  memset(ret, 0, n);
  return ret;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
	if(brk >= current->max_brk){
		int new_page_num = (brk>>12) - (current->max_brk>>12) + 1;
    void *allocted_page =  new_page(new_page_num);
    for (int i = 0; i < new_page_num; ++i){
			Log("mm_brk, map vaddr %08x paddr %08x", (current->max_brk + i * PGSIZE), (allocted_page + i * PGSIZE));
      map(&current->as, (void *)(current->max_brk + i * PGSIZE), (void *)(allocted_page + i * PGSIZE), 1);
    }
    current->max_brk += new_page_num << 12;
	}
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
