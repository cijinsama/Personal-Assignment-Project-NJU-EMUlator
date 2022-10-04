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
#include "local-include/reg.h"
#include "string.h"
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

static int regs_amount = sizeof(regs)/sizeof(regs[0]);
void isa_reg_display() {
	for (int i=0;i<regs_amount;i++){
		printf("%-4s\t0x%08x\t%08lu\n",reg_name(i,0),cpu.gpr[i],(unsigned long)cpu.gpr[i]);
	}	
}

word_t isa_reg_str2val(const char *s, bool *success) {
	int len_s = strlen(s),len_reg;
	*success = false;
	if (strcmp(s,regs[0]+1) == 0) {
		return cpu.gpr[0];
	}
	for (int i = 1; i < regs_amount; i++) {
		len_reg = strlen(regs[i]);
		if (len_reg == len_s) {
			if (memcmp(s,regs[i],strlen(regs[i]))==0){
				*success = true;
				return cpu.gpr[i];
			}
		}
	}
	panic();
  return 0;
}
