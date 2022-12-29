#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};//kas是页表
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}


static inline uintptr_t get_PAGE_DIRECTORY(uintptr_t addr){
	return addr >> 22;
}
static inline uintptr_t get_PAGE_TABLE(uintptr_t addr){
	return (addr & 0x003ff000u) >> 12;
}
static inline uintptr_t get_PAGE_INSIDE(uintptr_t addr){
	return addr & 0xfff;
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
// 	printf("pd : %08x;pt : %08x\n", va, pa);
	uintptr_t pd_bias = get_PAGE_DIRECTORY((uintptr_t)va);
	uintptr_t pt_bias = get_PAGE_TABLE((uintptr_t)va);
	uint32_t pd_item = *(uint32_t *)((uintptr_t)as->ptr + (pd_bias << 2));//由于每个表项大小为4B
// 	printf("pdbias : %08x;ptbias : %08x\n", pd_bias, pt_bias);
// 	printf("ptr = %08x\n", as->ptr);
// 	printf("pd_item = %08x\n", pd_item);
// 	printf("*pd_item = %08x\n", (*(uint32_t *)pd_item));
	if(!(pd_item & PTE_V)){
    uintptr_t new_page_table = (uintptr_t)pgalloc_usr(PGSIZE);//每个table有1024项，每项4B
// 		printf("new page = %08x\n", new_page_table);
    pd_item = (pd_item & 0x3ff) | (0xfffffc00u & (new_page_table));//把新开的page地址放到对应的PD里面
    pd_item = (pd_item | PTE_V);
		*(uint32_t *)((uintptr_t)as->ptr + (pd_bias << 2)) = pd_item;
	}
// 	printf("pd_item = %08x\n", (pd_item));
	uintptr_t pt_item = *(uint32_t *)((pd_item >> 12 << 12) | (pt_bias << 2));
  pt_item = 0xfffffc00u & ((uintptr_t)pa & ~0xfff);
  pt_item = ((uintptr_t)pa) & 0xfffffc00u;
	pt_item = pt_item | (PTE_V | PTE_X | PTE_W | PTE_R);
	*(uint32_t *)((pd_item >> 12 << 12) + (pt_bias << 2)) = pt_item;
	if( ((pt_item >> 12 << 12) | get_PAGE_INSIDE((uint32_t)va)) != (uint32_t)pa ){
		printf("va : %08x, pa : %08x\n",((pt_item >> 12 << 12) | get_PAGE_INSIDE((uint32_t)va)), pa);
	}
// 	printf("va : %08x, pa : %08x\n",((pt_item >> 12 << 12) | get_PAGE_INSIDE((uint32_t)va)), pa);
// 	printf("pd : %08x;pt : %08x\n", pd_item, pt_item);
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
	Context *context = kstack.end - sizeof(Context);
	context->mepc = (uintptr_t)	entry;
	context->mstatus = 0x1800;
  return context;
}
