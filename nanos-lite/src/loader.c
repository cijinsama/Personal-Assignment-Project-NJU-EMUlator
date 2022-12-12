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
void ReadSectionName(Elf_Shdr shstrtab_sect_header, Elf_Shdr section, char* name);
void ReadString(Elf_Shdr shstrtab_sect_header, Elf_Off string_offset, char* dst);
void ramdisk2vmem(uintptr_t ramdisk_off, uintptr_t vmemaddr, uint32_t memsize);
void vmemset(uintptr_t vaddr,uint32_t size, uint32_t value);


uintptr_t ini_loader(){
	Elf_Ehdr elf_header;
	Elf_Off program_header_off;
	Elf_Phdr program_header;
	uintptr_t ret_addr = 0;

	//read elf header and get program header table offset
	ReadElfHeader(&elf_header);
	program_header_off = elf_header.e_phoff;

	//load ramdisk to v mem and set 0
	for (int i = 0; i < elf_header.e_phnum; i++, program_header_off += elf_header.e_phentsize){
		ReadFile(program_header_off, &program_header, sizeof(program_header), 1);
		if (program_header.p_type == PT_LOAD){
			ramdisk2vmem(program_header.p_offset, program_header.p_vaddr, program_header.p_memsz);
			vmemset(program_header.p_vaddr+program_header.p_filesz, program_header.p_vaddr+program_header.p_memsz, 0);
		}
		Log("flag位:%p",(void *)program_header.p_flags);
// 		if (program_header.p_flags
		ret_addr = program_header.p_vaddr;
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
	assert(*(uint32_t *)elf_header->e_ident == 0x7f454c46);
	return;
}

void ReadSectionName(Elf_Shdr shstrtab_sect_header, Elf_Shdr section, char* name){
	ReadString(shstrtab_sect_header, section.sh_name, name);
	return;
}

void ReadFile(int offset, void* dst, unsigned long size,int number){
	ramdisk_read(dst, offset, size * number);
	return;
}

void ReadString(Elf_Shdr shstrtab_sect_header, Elf_Off string_offset, char* dst){
	int i; 
	for (i = 0;string_offset + i < shstrtab_sect_header.sh_size;i++){
		ReadFile(shstrtab_sect_header.sh_offset + string_offset + i, dst + i, sizeof(char), 1);
		if (dst[i] == '\0') break;
	}
	return;
}

void ramdisk2vmem(uintptr_t ramdisk_off, uintptr_t vmemaddr, uint32_t memsize){
	uint8_t buffer[128];
	int this_read_size;
	for(int readsize = 0; readsize < memsize; readsize+=128){
		if (readsize + 128 < memsize) this_read_size = 128;
		else this_read_size = memsize - readsize;
		ramdisk_read(buffer, ramdisk_off, this_read_size);
		for (int i = 0; i < 128; i++){
			*(uint32_t *)(vmemaddr + 4 * i) = buffer[i];
		}
	}
	return;
}

void vmemset(uintptr_t vaddr,uint32_t size, uint32_t value){
	for (int i = 0; i < size; i++){
		*(uint32_t *)(vaddr + 4 * i) = value;
	}
	return;
}
