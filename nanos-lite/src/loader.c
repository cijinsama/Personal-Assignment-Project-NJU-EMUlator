#include <proc.h>
#include <elf.h>
#include <stdio.h>
#include <common.h>
#include <fs.h>
#include "../../nemu/src/monitor/ftrace.h"


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



void ReadSectionName(int fp, Elf32_Shdr shstrtab_sect_header, Elf32_Shdr section, char* name);
void ReadString(int fp, Elf32_Shdr shstrtab_sect_header, Elf32_Off string_offset, char* dst);
void ReadFile(int fd, int offset, void* dst, unsigned long size,int number);
void ReadElfHeader(int fd, Elf_Ehdr *elfheader);
void program2vmem(int fd, uintptr_t program_off, uintptr_t vmemaddr, uint32_t memsize);
void vmemset(uint8_t* vaddr,uint32_t size, uint32_t value);

static uintptr_t loader(PCB *pcb, const char *filename) {
	Log("load file name : %s", filename);
	Elf_Ehdr elf_header;
	Elf_Off program_header_off;
	Elf_Phdr program_header;
	// open file
	int fd = fs_open(filename, 0, 0);
	//read elf header and get program header table offset
	ReadElfHeader(fd, &elf_header);
	program_header_off = elf_header.e_phoff;

	//load ramdisk to v mem and set 0
	for (int i = 0; i < elf_header.e_phnum; i++, program_header_off += sizeof(program_header)){
		ReadFile(fd, program_header_off, &program_header, sizeof(program_header), 1);
		if (program_header.p_type == PT_LOAD){
			program2vmem(fd, program_header.p_offset, program_header.p_vaddr, program_header.p_memsz);
			vmemset((uint8_t *) (program_header.p_vaddr+program_header.p_filesz), program_header.p_memsz - program_header.p_filesz, 0);
		}
	}
	fs_close(fd);
	return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p",(void *)entry);
  ((void(*)())entry) ();
}

void ReadElfHeader(int fd, Elf_Ehdr *elf_header){
// 	Log("e_ident : %x",*(uint32_t *)elf_header->e_ident);
	ReadFile(fd, 0, elf_header, sizeof(Elf_Ehdr), 1);
// 	Log("e_ident : %x",*(uint32_t *)elf_header->e_ident);
	assert(*(uint32_t *)elf_header->e_ident == 0x464c457f);
	return;
}


void ReadFile(int fd, int offset, void* dst, unsigned long size,int number){
	fs_lseek(fd, offset, SEEK_SET);
	fs_read(fd, dst, size * number);
	return;
}


void program2vmem(int fd, uintptr_t program_off, uintptr_t vmemaddr, uint32_t memsize){
	Log("load program to vaddr : %x", vmemaddr);
	ReadFile(fd, program_off, (uint8_t *)vmemaddr, memsize, 1);
// 	ramdisk_read((uint8_t *)vmemaddr, ramdisk_off, memsize);
	return;
}

void vmemset(uint8_t* vaddr,uint32_t size, uint32_t value){
	Log("set Vmem value %p---%p to %x", vaddr, vaddr + size, value);
	for (int i = 0; i < size; i++){
		*(vaddr + i) = value;
	}
	return;
}

int add_func_table(char *file){
	int fp = -1;
	Elf32_Ehdr elf_header;
	Elf32_Shdr sect_header;
	Elf32_Shdr symb_sect_header;
	Elf32_Shdr strb_sect_header;
	Elf32_Shdr shstrtab_sect_header;
	Elf32_Off section_header_table_off;
	char name[64];
	fp = fs_open(file, 0, 0);
	//read elf header and get section header table offset
	ReadElfHeader(fp, &elf_header);
	section_header_table_off = elf_header.e_shoff;

	//检查是否是elf文件
	assert(*(uint32_t *)elf_header.e_ident == 0x464c457f);

	//read section header table and get section string tab
	ReadFile(fp, section_header_table_off + elf_header.e_shstrndx * sizeof(sect_header), &shstrtab_sect_header, sizeof(sect_header), 1);
	
	//find symtab section header in section header table
	for (int idx = 0 ; idx < elf_header.e_shnum; idx++) {
		ReadFile(fp, section_header_table_off + idx * sizeof(sect_header), &sect_header, sizeof(sect_header), 1);
		//read section name from string table
		ReadSectionName(fp, shstrtab_sect_header, sect_header, name);
		if (strcmp(name, ".symtab") == 0){
			memcpy(&symb_sect_header, &sect_header, sizeof(sect_header));
		}
		if (strcmp(name, ".strtab") == 0){
			memcpy(&strb_sect_header, &sect_header, sizeof(sect_header));
		}
	}

	//read the symtab name and print the name from strtab
	Elf32_Sym symbo;
	for (int idx = 0; idx < symb_sect_header.sh_size / symb_sect_header.sh_entsize; idx++) {
		ReadFile(fp, symb_sect_header.sh_offset + symb_sect_header.sh_entsize*idx, &symbo, sizeof(symbo), 1);
		ReadString(fp, strb_sect_header, symbo.st_name, name);
		if (ELF32_ST_TYPE(symbo.st_info) == STT_FUNC) {
			strcpy(func_table[func_table_size].name, name);
			func_table[func_table_size].min = symbo.st_value;
			func_table[func_table_size].max = symbo.st_value + symbo.st_size;
			func_table_size++;
		}
	}
	fs_close(fp);
	return 0;
}

void ReadString(int fp, Elf32_Shdr shstrtab_sect_header, Elf32_Off string_offset, char* dst){
	int i; 
	for (i = 0;string_offset + i < shstrtab_sect_header.sh_size;i++){
		ReadFile(fp, shstrtab_sect_header.sh_offset + string_offset + i, dst + i, sizeof(char), 1);
		if (dst[i] == '\0') break;
	}
	return;
}

void ReadSectionName(int fp, Elf32_Shdr shstrtab_sect_header, Elf32_Shdr section, char* name){
	ReadString(fp, shstrtab_sect_header, section.sh_name, name);
	return;
}
