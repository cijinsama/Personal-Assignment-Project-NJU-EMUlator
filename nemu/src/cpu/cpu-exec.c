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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <cpu/ifetch.h>
#include <locale.h>
#include "../../monitor/ftrace.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {.INTR = 0};
// CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

CSR_state csr = {.mstatus.val = 0x1800, .satp.decode.V = 0};

#ifdef CONFIG_FTRACE
int func_stack = 0;
int last_pc_in_which_func = -1;
unsigned char func_stack_container[1024];
#endif

void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
#ifdef CONFIG_WATCHPOINT
	uint32_t new_value;
	bool successful;
	WP* watcher_pointer = watcher_head;
	while (watcher_pointer != NULL) {
		new_value = expr(watcher_pointer.expr,&successful);
		if (!successful) {
			fprintf(stderr, "evaluation error\n");
			panic();
			break;
		}
		if (new_value != watcher_pointer.last_value) {
			printf("Hardware watchpoint %d : %s\n", watcher_pointer.NO, watcher_pointer.str);
			printf("Old value : %d\n", watcher_pointer.last_value);
			printf("New value : %d\n", new_value);
			watcher_pointer.last_value = new_value;
			nemu_state.state = NEMU_STOP;
		}
		watcher_pointer = watcher_pointer.next;
	}
	if (nemu_state.state == NEMU_STOP) {
		printf("trigered the watchpoint, program stop.\n");
	}
#endif 
//
}
#ifdef CONFIG_IRINGBUF
#define IRINGBUFSIZE 4
typedef struct {
	vaddr_t pc;
	vaddr_t snpc;
	ISADecodeInfo isa;
} iring;
iring iringbuf[IRINGBUFSIZE];
static int iringbufcounter = 0;
static void cpiring(Decode *s){
	iringbuf[iringbufcounter].pc = s->pc;
	iringbuf[iringbufcounter].snpc = s->snpc;
	iringbuf[iringbufcounter].isa.inst.val = s->isa.inst.val;
	iringbufcounter++;
	iringbufcounter = iringbufcounter % IRINGBUFSIZE;
}
#endif

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);//执行前pc，snpc指向当前指令//执行过后dnpc指向下一条指令
#ifdef CONFIG_TRACE
	int i = 0;
#endif
#ifdef CONFIG_FTRACE
	for (i = 0; i < func_table_size; i++){
		if (s->dnpc == func_table[i].min) {
			func_stack_container[func_stack] = i;
			log_write("0x%08x:",s->pc);
			last_pc_in_which_func = i;
			for (int j = 0; j < func_stack; j++) log_write("\t");
			log_write("[ftrace]: call [%s@0x%08x]\n", func_table[i].name, s->dnpc);
		}
		else if (last_pc_in_which_func != -1 && s->dnpc > func_table[i].min && s->dnpc < func_table[i].max && i != last_pc_in_which_func) {
		}
	}
// 	for (i = 0; i < func_table_size; i++){
// 		if (s->dnpc == func_table[i].min) {
// 			func_stack_container[func_stack] = i;
// 			func_stack++;
// 			log_write("0x%08x:",s->pc);
// 			last_pc_in_which_func = i;
// 			for (int j = 0; j < func_stack; j++) log_write("\t");
// 			log_write("[ftrace]: call [%s@0x%08x]\n", func_table[i].name, s->dnpc);
// 		}
// 		else if (last_pc_in_which_func != -1 && s->dnpc > func_table[i].min && s->dnpc < func_table[i].max && i != last_pc_in_which_func) {
// 			func_stack--;
// 			if (func_stack > 0) last_pc_in_which_func = func_stack_container[func_stack-1];
// 			if (func_stack < 0){
// 				assert(0);
// 			}
// 			log_write("0x%08x:",s->pc);
// 			for (int j = -1; j < func_stack; j++) log_write("\t");
// 			log_write("[ftrace]: ret  [%s@0x%08x]\n", func_table[(int) func_stack_container[func_stack]].name, s->dnpc);
// 		}
// 	}
#endif
  cpu.pc = s->dnpc;
#ifdef CONFIG_IRINGBUF
	cpiring(s);
#endif
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
   for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#endif
}

static bool temp_flag = false;
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) { 
		exec_once(&s, cpu.pc);//执行前pc指向将要执行的指令.执行过后pc++
    g_nr_guest_inst ++;
		if(csr.mstatus.decode.MIE != temp_flag){
// 			Log("MIE set to %d, cpu.pc = %08x, s.pc = %08x",csr.mstatus.decode.MIE, cpu.pc, s.pc);
			temp_flag = csr.mstatus.decode.MIE;
		}
// 		Log("csr.MIE %d",csr.mstatus.decode.MIE);
		uint32_t NO = isa_query_intr();
		if (NO == IRQ_TIMER){
			cpu.pc = isa_raise_intr(NO, cpu.pc);
// 			Log("cpu.pc set to %08x", cpu.pc);
		}
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;
		case NEMU_ABORT:
    case NEMU_END:
			#ifdef CONFIG_IRINGBUF
			char *p;
			int ilen;
			int i;
			uint8_t *inst;
			int ilen_max;
			int space_len;
			char logbuf[128];
			int j;
			iring s;
			log_write("==========================\n");
			for (j = 0; j < IRINGBUFSIZE - 1; j++) {//输出过去的几个指令
				p = logbuf;
				s = iringbuf[(j + iringbufcounter) % IRINGBUFSIZE];
				if (s.isa.inst.val == 0) continue;
				p += snprintf(p, sizeof(logbuf), FMT_WORD ":", s.pc);
				ilen = s.snpc - s.pc;
				inst = (uint8_t *)&s.isa.inst.val;
				for (i = ilen - 1; i >= 0; i --) {
					p += snprintf(p, 4, " %02x", inst[i]);
				}
				ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
				space_len = ilen_max - ilen;
				if (space_len < 0) space_len = 0;
				space_len = space_len * 3 + 1;
				memset(p, ' ', space_len);
				p += space_len;

				void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
				disassemble(p, logbuf + sizeof(logbuf) - p,
					MUXDEF(CONFIG_ISA_x86, s.snpc, s.pc), (uint8_t *)&s.isa.inst.val, ilen);
				log_write("    \t");
				log_write("%s\n", logbuf); 
			}
			s = iringbuf[(j + iringbufcounter) % IRINGBUFSIZE];
			for (j = 0; j < IRINGBUFSIZE; j++) {//输出之后的几个指令
				p = logbuf;
				p += snprintf(p, sizeof(logbuf), FMT_WORD ":", s.pc);
				ilen = s.snpc - s.pc;
				inst = (uint8_t *)&s.isa.inst.val;
				for (i = ilen - 1; i >= 0; i --) {
					p += snprintf(p, 4, " %02x", inst[i]);
				}
				ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
				space_len = ilen_max - ilen;
				if (space_len < 0) space_len = 0;
				space_len = space_len * 3 + 1;
				memset(p, ' ', space_len);
				p += space_len;

				void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
				disassemble(p, logbuf + sizeof(logbuf) - p,
					MUXDEF(CONFIG_ISA_x86, s.snpc, s.pc), (uint8_t *)&s.isa.inst.val, ilen);
				if (j == 0) log_write("===>\t");
				else log_write("    \t");
				log_write("%s\n", logbuf); 
				s.pc = s.snpc;
				s.isa.inst.val = inst_fetch( &s.snpc, sizeof(s.isa.inst.val));//这个地方要加一个判断是否出界，如果出界就break；
			}
			log_write("==========================\n");
			#endif
    //case NEMU_END:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
