#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

#define STACK_SIZE (8 * PGSIZE)

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    Context *cp;
    AddrSpace as;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk;
  };
} PCB;
PCB *get_free_PCB();

void context_uload(PCB *pcb, char filename[],char *argv[],char *envp[]);
extern PCB *current;
uintptr_t loader(PCB *pcb, const char *filename);

void naive_uload(PCB *pcb, const char *filename);
size_t execve(const char * filename, char *const argv[], char *const envp[]);
#endif
