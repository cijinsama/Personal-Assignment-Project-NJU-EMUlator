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


#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <debug.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_B, TYPE_R, TYPE_shamt, TYPE_P,
  TYPE_N, // none
};
#define BITMASK_SELF(high, low) (BITMASK(high) ^ BITMASK(low - 1))
#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = ( ((int32_t) SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 30, 21) <<  1) | (BITS(i, 20, 20) << 11) | (BITS(i, 19, 12) << 12) );} while(0)
#define immB() do { *imm = (( (int32_t) SEXT(BITS(i, 31, 31), 1) << 12 ) | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1)); } while(0)
#define immshamt() do { *imm = BITS(i, 24, 20); } while(0)



inline static word_t get_csr(word_t csr_num){
	word_t ret;
// 	printf("get csr_num = %04x\n", csr_num);
	switch (csr_num) {
		case 0x0341: ret = csr.mepc;										break;
		case 0x0300: ret = csr.mstatus.val;							break;
		case 0x0342: ret = csr.mcause;									break;
		case 0x0305: ret = csr.mtvec;										break;
		default : Log("Unknown csr register\n"); panic("please complete\n");
	}
	return ret;
}

inline static void set_csr(word_t csr_num, word_t imm){
// 	Log("set csr_num = %04x\n", csr_num);
// 	Log("mstatus.val = %08x",csr.mstatus.val);
// 	Log("set imm = %08x\n", imm);
	switch (csr_num) {
		case 0x0341: csr.mepc = imm;													break;
		case 0x0300: csr.mstatus.val = imm;										break;
		case 0x0342: csr.mcause = imm;												break;
		case 0x0305: csr.mtvec = imm;													break;
		default : Log("Unknown csr register\n"); panic("please complete\n");
	}
// 	Log("mstatus.val = %08x",csr.mstatus.val);
}

inline static void and_csr(word_t csr_num, word_t imm){
// 	Log("and csr_num = %04x\n", csr_num);
// 	Log("and imm = %08x\n", imm);
	switch (csr_num) {
		case 0x0341: csr.mepc = csr.mepc | imm;											break;
		case 0x0300: csr.mstatus.val = csr.mstatus.val | imm;				break;
		case 0x0342: csr.mcause = csr.mcause | imm;									break;
		case 0x0305: csr.mtvec = csr.mtvec | imm;										break;
		default : Log("Unknown csr register\n"); panic("please complete\n");
	}
}

static void decode_operand(Decode *s, int *dest, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rd  = BITS(i, 11, 7);
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *dest = rd;
  switch (type) {
    case TYPE_I:		src1R();          immI();					 break;
    case TYPE_U:			                immU();					 break;
    case TYPE_S:		src1R(); src2R(); immS();					 break;
		case TYPE_J:											immJ();					 break;
		case TYPE_B:		src1R(); src2R(); immB();					 break;
		case TYPE_R:		src1R(); src2R();									 break;
		case TYPE_shamt:src1R();					immshamt();			 break;
		case TYPE_N:																			 break;
		default: panic("Unknown instruction type\n");
  }
}

static int decode_exec(Decode *s) {
  int dest = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();//INSTPAT左边为高位，右边为低位
	INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add		 , R, R(dest) = src1 + src2);
	INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, src1 += imm, R(dest) = src1);
	INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and		 , R, R(dest) = (src1 & src2));
	INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi	 , I, R(dest) = (src1 & imm));
	INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(dest) = s->pc + imm);
	INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq		 , B, if(src1 == src2){s->dnpc = s->pc + imm;});
	INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge		 , B, if((int32_t) src1 >= (int32_t) src2){s->dnpc = s->pc + imm;});
	INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu	 , B, if((uint32_t) src1 >= (uint32_t) src2){s->dnpc = s->pc + imm;});
	INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt		 , B, if((int32_t) src1 < (int32_t) src2){s->dnpc = s->pc + imm;});
	INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu	 , B, if((uint32_t) src1 < (uint32_t) src2){s->dnpc = s->pc + imm;});
	INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne		 , B, if(src1 != src2){s->dnpc = s->pc + imm;});
	INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs	 , I, R(dest) = get_csr(imm), and_csr(imm, src1));
	INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw	 , I, R(dest) = get_csr(imm), set_csr(imm, src1));
	INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div		 , R, R(dest) = (int32_t) src1 / (int32_t) src2);
	INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu	 , R, R(dest) = (uint32_t) src1 / (uint32_t) src2);
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("0000000 00000 00000 000 00000 11100 11",	ecall	 , N, csr.mstatus.decode.MPIE = csr.mstatus.decode.MIE, csr.mstatus.decode.MIE = 1);
	INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(dest) = s->dnpc, s->dnpc = s->pc + imm);								//注意，要求跳转数为2的倍数，并只记录21位的前20位
	INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, R(dest) = s->snpc, s->dnpc = ((src1 + imm) & (~1)));	//注意同上
	INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb		 , I, R(dest) = SEXT(Mr(src1 + imm, 1), 8));
	INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu		 , I, R(dest) = Mr(src1 + imm, 1));
	INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh		 , I, R(dest) = SEXT(Mr(src1 + imm, 2), 16));
	INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu		 , I, R(dest) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(dest) = imm);
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = Mr(src1 + imm, 4));
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , N, s->dnpc = csr.mepc, csr.mstatus.decode.MIE = csr.mstatus.decode.MPIE, csr.mstatus.decode.MPIE = 1; IFDEF(CONFIG_ETRACE, Log("[etrace] : mret to pc : %x", csr.mepc); log_write("[etrace] : mret to pc : %x", csr.mepc);));
	INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul		 , R, R(dest) = src1 * src2);
	INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh	 , R, R(dest) = ((SEXT(src1, 32)) * (SEXT(src2, 32))) >> 32);
	INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or		 , R, R(dest) = (src1 | src2));
	INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori		 , I, R(dest) = (src1 | imm));
	INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem		 , R, R(dest) = (int32_t) src1 % (int32_t) src2);
	INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu	 , R, R(dest) = (uint32_t) src1 % (uint32_t) src2);
	INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb		 , S, Mw(src1 + imm, 1, src2));
	INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll		 , R, R(dest) = (src1 << src2));
	INSTPAT("000000? ????? ????? 001 ????? 00100 11", slli	 , shamt, R(dest) = src1 << imm);
	INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt		 , R, R(dest) = ( (int32_t) src1 < (int32_t) src2));
	INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti	 , I, R(dest) = ( (int32_t) src1 < (int32_t) imm));
	INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu	 , I, R(dest) = ( (uint32_t) src1 < (uint32_t) imm));
	INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu	 , R, R(dest) = ( (uint32_t) src1 < (uint32_t) src2));
	INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra		 , R, R(dest) = ((int32_t) src1 >> src2));
	INSTPAT("010000? ????? ????? 101 ????? 00100 11", srai	 , shamt, R(dest) = (int32_t) src1 >> imm);
	INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl		 , R, R(dest) = (src1 >> src2));
	INSTPAT("000000? ????? ????? 101 ????? 00100 11", srli	 , shamt, R(dest) = src1 >> imm);
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
	INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub		 , R, R(dest) = src1 - src2);
	INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor		 , R, R(dest) = (src1 ^ src2));
	INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori	 , I, R(dest) = (src1 ^ imm));

  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
