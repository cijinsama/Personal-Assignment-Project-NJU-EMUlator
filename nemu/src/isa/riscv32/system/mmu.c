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
#include <memory/paddr.h>
#include <memory/vaddr.h>

static inline uint32_t get_PAGE_DIRECTORY(vaddr_t addr){
	return addr >> 22;
}
static inline uint32_t get_PAGE_TABLE(vaddr_t addr){
	return (addr & 0x003ff000u) >> 12;
}
static inline uint32_t get_PAGE_INSIDE(vaddr_t addr){
	return addr & 0xfff;
}
#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_A 0x40
#define PTE_D 0x80
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
	if(isa_mmu_check(vaddr, len, type) == MMU_TRANSLATE){
		Log("vaddr = %08x", vaddr);
		Log("translate vaddr");
		uint32_t base = csr.satp.decode.base << 12;
		Log("base = %08x",(uint32_t) base);
		uint32_t pd_item = base + (get_PAGE_DIRECTORY(vaddr) << 2);
		uint32_t pt_addr = paddr_read(pd_item, 4);
		Assert(pt_addr & PTE_V, "page_table_entry not valid, vaddr: %#x", vaddr);
		uint32_t pt_item = (pt_addr >> 10 << 12) | (get_PAGE_TABLE(vaddr) << 2);
		Log("pd_item = %08x", (uint32_t)pd_item);
		Log("pt_addr = %08x", (uint32_t)pt_addr >> 10 << 12);
		Log("pt_item = %08x", (uint32_t)pt_item);
		uint32_t pg_addr = paddr_read(pt_item, 4);
		Assert(pg_addr & PTE_V, "page_table_entry not valid, vaddr: %#x", vaddr);
		Log("pg_addr = %08x", (uint32_t)pg_addr >> 10 << 12);
		Assert(((pg_addr>>10 << 12) | get_PAGE_INSIDE(vaddr)) == vaddr, "paddr = %08x, vaddr = %08x", (uint32_t)((pg_addr>>10 << 12) | get_PAGE_INSIDE(vaddr)), vaddr);
		if(type == 0){
			paddr_write(pt_item, 4, pg_addr | PTE_A);
		}
		else {
			paddr_write(pt_item, 4, pg_addr | PTE_D);
		}
		return (pg_addr>>10 << 12) | get_PAGE_INSIDE(vaddr);
	}
	else return vaddr;
}

inline int isa_mmu_check(vaddr_t vaddr, int len, int type){
	if(csr.satp.decode.V) return MMU_TRANSLATE;
	else return MMU_DIRECT;
}
