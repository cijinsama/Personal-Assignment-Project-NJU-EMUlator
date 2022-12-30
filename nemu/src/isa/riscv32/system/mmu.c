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
		uint32_t base = csr.satp.decode.base << 12;
		uint32_t pd_item_addr = base + (get_PAGE_DIRECTORY(vaddr) << 2);
		uint32_t pd_item = paddr_read(pd_item_addr, 4);
		uint32_t pt_item_addr = (pd_item >> 10 << 12) | (get_PAGE_TABLE(vaddr) << 2);
		uint32_t pt_item = paddr_read(pt_item_addr, 4);
		if(type == 0){
			paddr_write(pt_item_addr, 4, pt_item | PTE_A);
		}
		else {
			paddr_write(pt_item_addr, 4, pt_item | PTE_D);
		}
		return (pt_item>>10 << 12) | get_PAGE_INSIDE(vaddr);
	}
	else return vaddr;
}

inline int isa_mmu_check(vaddr_t vaddr, int len, int type){
	if(csr.satp.decode.V) return MMU_TRANSLATE;
	else return MMU_DIRECT;
}
