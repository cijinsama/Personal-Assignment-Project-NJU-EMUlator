/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <utils.h>
#include <debug.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
	//中断时记录下一位，异常时记录本位
	csr.mepc = epc;
	csr.mcause = NO;
	cpu.pc = csr.mtvec;
	if(NO == IRQ_TIMER){
		csr.mepc = epc - 4;
		csr.mstatus.decode.MPIE = csr.mstatus.decode.MIE;
	}
  IFDEF(CONFIG_ETRACE, Log("[etrace]: raise exception at csr.mepc = %x, csr.mtvec = %x, csr.mstatus = %x, csr.mcause = %x", csr.mepc, csr.mtvec, csr.mstatus.val, csr.mcause); log_write("[etrace]: raise exception at csr.mepc = %x, csr.mtvec = %x, csr.mstatus = %x, csr.mcause = %x", csr.mepc, csr.mtvec, csr.mstatus.val, csr.mcause););
	csr.mstatus.decode.MIE = 0;
  return 0;
}

word_t isa_query_intr() {
	if (csr.mstatus.decode.MIE == 1) {
		if(cpu.INTR){
			cpu.INTR = false;
			return IRQ_TIMER;
		}
		else{
			return EXCP_Environment;
		}
// 		switch(csr.mstatus.val){
// 			case 0x1808:
// 				return EXCP_Environment;
// 			case 0x21808://这个是syscall
// 				return EXCP_Environment;
// 			default : 
// 				return EXCP_Environment;
// 		}
	}
	return INTR_EMPTY;
}
