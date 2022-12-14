#ifndef ARCH_H__
#define ARCH_H__

#define INTR_Machine_software_interrupt		0x80000003
#define INTR_Machine_timer_interrupt			0x80000007
#define INTR_Environment									0x80000011
#define EXCP_Environment									0x0000000b
struct Context {
  // TODO: fix the order of these members to match trap.S
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir;
};

#define GPR1 gpr[17] // a7
#define GPR2 gpr[10] // 10是a0
#define GPR3 gpr[11]
#define GPR4 gpr[12]
#define GPRx gpr[10]

#endif
