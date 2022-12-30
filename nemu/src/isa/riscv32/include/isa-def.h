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

#ifndef __ISA_RISCV32_H__
#define __ISA_RISCV32_H__

#include <common.h>

typedef struct {
  word_t gpr[32];
  vaddr_t pc;
	bool INTR;
} riscv32_CPU_state;

// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} riscv32_ISADecodeInfo;

// regiter
typedef struct {
  word_t mepc;
  word_t mcause;
	word_t mtvec;
  union {
    struct{
      word_t UIE    : 1;
      word_t SIE    : 1;
      word_t WPRI_0 : 1;
      word_t MIE    : 1;
      word_t UPIE   : 1;
      word_t SPIE   : 1;
      word_t WPRI   : 1;
      word_t MPIE   : 1;
      word_t SPP    : 1;
      word_t WPRI_1_2 : 2;
      word_t MPP    : 2;
      word_t FS     : 2;
      word_t XS     : 2;
      word_t MPRV   : 1;
      word_t SUM    : 1;
      word_t MXR    : 1;
      word_t TVM    : 1;
      word_t TW     : 1;
      word_t TSR    : 1;
      word_t WPRI_3_10 : 8;
      word_t SD     : 1;
    } decode;
    word_t val;
  } mstatus;
  union {
    struct{
      word_t base  : 20;
			word_t other : 11;
      word_t V     : 1;
    } decode;
    word_t val;
  } satp;
} riscv32_CSR_state;


//# define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif
