#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15) f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

//最后+1是因为自己加入的东西
#define CONTEXT_SIZE  ((32 + 3 + 1 + 1) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  (32 * XLEN)
#define OFFSET_STATUS (33 * XLEN)
#define OFFSET_EPC    (34 * XLEN)
#define OFFSET_NP			(35 * XLEN)

.align 3
.globl __am_asm_trap
__am_asm_trap:
	//实现ksp != 0? $sp = ksp
  csrrw sp, mscratch, sp   //交换sp和mscratch
	bnez sp, save_context    //现在sp里面的东西代表ksp，如果是0就是内核态
	csrr sp, mscratch        //用户态就把sp读回来



save_context:
	// now sp is pointing to the kernel stack
	//save context
	//这一步过后sp指向的是将要给irq_handle的参数context
  addi sp, sp, -CONTEXT_SIZE


  MAP(REGS, PUSH)

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc
	//实现c->sp = $sp
	csrr t3, mscratch

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)
	//实现c->sp = $sp
	STORE t3, OFFSET_SP(sp)

	//调用的参数context准备完成

  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp
  jal __am_irq_handle
	//如果当前进行了程序替换则
	//a0是返回值寄存器,指向的是PCB储存的context
	//否则a0还是原来的a0
	//恢复context
	mv sp, a0
	//
	LOAD t3, OFFSET_NP(sp)
  bnez t3, restore_context
  addi t3, sp, CONTEXT_SIZE//如果是用户态就要把sp读到scratch中
  csrw mscratch, t3

restore_context:
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw mstatus, t1
  csrw mepc, t2

  MAP(REGS, POP)

	LOAD t3, OFFSET_SP(sp)//+++
  addi sp, sp, CONTEXT_SIZE
	mv t3, sp

  mret
