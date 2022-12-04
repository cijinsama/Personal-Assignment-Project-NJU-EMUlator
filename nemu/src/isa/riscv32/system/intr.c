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

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
	//中断时记录下一位，异常时记录本位
// 	if(BITS(NO,31,31) == 1){
// 		csr.mepc = epc;
// 	}
// 	else {
		csr.mepc = epc - 4;
// 	}
	csr.mstatus.decode.MIE = 0;
	csr.mcause = NO;
	cpu.pc = csr.mtvec;
  return 0;
}

word_t isa_query_intr() {
	if (csr.mstatus.decode.MIE == 1) {
		return EXCP_Environment;
	}
	return INTR_EMPTY;
}
