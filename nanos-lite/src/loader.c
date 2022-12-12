#include <proc.h>
#include <elf.h>
#include <stdio.h>
#include <common.h>


#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Shdr Elf64_Shdr
# define Elf_Off  Elf64_Off
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Shdr Elf32_Shdr
# define Elf_Off  Elf32_Off
#endif



void ReadFile(int offset, void* dst, unsigned long size,int number);
void ReadElfHeader(Elf_Ehdr *elfheader);
void ramdisk2vmem(uintptr_t ramdisk_off, uintptr_t vmemaddr, uint32_t memsize);
void vmemset(uint8_t* vaddr,uint32_t size, uint32_t value);


uintptr_t ini_loader(){
	Elf_Ehdr elf_header;
	Elf_Off program_header_off;
	Elf_Phdr program_header;
	uintptr_t ret_addr = 0;

	//read elf header and get program header table offset
	ReadElfHeader(&elf_header);
	program_header_off = elf_header.e_phoff;

	//load ramdisk to v mem and set 0
	Log("header p num:%d", elf_header.e_phnum);
	Log("header p entsize:%d", elf_header.e_phentsize);
	Log("header p entsize:%d", sizeof(program_header));
	for (int i = 0; i < elf_header.e_phnum; i++, program_header_off += sizeof(program_header)){
		Log("read file");
		Log("addr: %x",program_header_off);
		ReadFile(program_header_off, &program_header, sizeof(program_header), 1);
		if (program_header.p_type == PT_LOAD){
			ramdisk2vmem(program_header.p_offset, program_header.p_vaddr, program_header.p_memsz);
			vmemset((uint8_t *) (program_header.p_vaddr+program_header.p_filesz), program_header.p_memsz - program_header.p_filesz, 0);
		}
		if (program_header.p_flags == PF_X + PF_R) ret_addr = program_header.p_vaddr;
	}
	return ret_addr;
}

static uintptr_t loader(PCB *pcb, const char *filename) {
	//需要返回这段程序的首地址
  return ini_loader();
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p",(void *)entry);
  ((void(*)())entry) ();
}

void ReadElfHeader(Elf_Ehdr *elf_header){
	ReadFile(0, elf_header, sizeof(Elf_Ehdr), 1);
	Log("GOT e_ident = %x, expected 0x464c457f", *(uint32_t *)elf_header->e_ident);
	assert(*(uint32_t *)elf_header->e_ident == 0x464c457f);
	return;
}


void ReadFile(int offset, void* dst, unsigned long size,int number){
	ramdisk_read(dst, offset, size * number);
	return;
}


void ramdisk2vmem(uintptr_t ramdisk_off, uintptr_t vmemaddr, uint32_t memsize){
	ramdisk_read((uint8_t *)vmemaddr, ramdisk_off, memsize);
	return;
}

void vmemset(uint8_t* vaddr,uint32_t size, uint32_t value){
	Log("memset size%d", size);
	for (int i = 0; i < size; i++){
		*(vaddr + i) = value;
	}
	return;
}
