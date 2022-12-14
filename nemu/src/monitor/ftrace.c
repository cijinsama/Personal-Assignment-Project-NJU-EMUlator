#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>
#include "ftrace.h"
void ReadFile(FILE* fp, int offset, void* dst, unsigned long size,int number);
void ReadElfHeader(FILE *fp, Elf32_Ehdr *elfheader);
void ReadSectionName(FILE *fp, Elf32_Shdr shstrtab_sect_header, Elf32_Shdr section, char* name);
void ReadString(FILE *fp, Elf32_Shdr shstrtab_sect_header, Elf32_Off string_offset, char* dst);
#ifdef CONFIG_FTRACE
func_add func_table[512];
#else
func_add func_table[0];
#endif
int func_table_size = 0;

int ini_func_table(char *file){
#ifdef CONFIG_FTRACE
	FILE *fp = NULL;
	Elf32_Ehdr elf_header;
	Elf32_Shdr sect_header;
	Elf32_Shdr symb_sect_header;
	Elf32_Shdr strb_sect_header;
	Elf32_Shdr shstrtab_sect_header;
	Elf32_Off section_header_table_off;
	char name[64];
	Log("read elf file : %s", file);
	fp = fopen(file, "rb");
	if (fp == NULL) {
		printf("error\n");
		assert(0);
	}
	else {
		//read elf header and get section header table offset
		ReadElfHeader(fp, &elf_header);
		section_header_table_off = elf_header.e_shoff;

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
	}
	fclose( fp );
#endif
	return 0;
}

void ReadElfHeader(FILE *fp, Elf32_Ehdr *elf_header){
	ReadFile(fp, 0, elf_header, sizeof(Elf32_Ehdr), 1);
	return;
}

void ReadSectionName(FILE *fp, Elf32_Shdr shstrtab_sect_header, Elf32_Shdr section, char* name){
	ReadString(fp, shstrtab_sect_header, section.sh_name, name);
	return;
}

void ReadFile(FILE* fp, int offset, void* dst, unsigned long size,int number){
	int i;
	fseek(fp, offset, SEEK_SET);
	i = fread(dst, size, number, fp);
	i += i;
	return;
}


void ReadString(FILE *fp, Elf32_Shdr shstrtab_sect_header, Elf32_Off string_offset, char* dst){
	int i; 
	for (i = 0;string_offset + i < shstrtab_sect_header.sh_size;i++){
		ReadFile(fp, shstrtab_sect_header.sh_offset + string_offset + i, dst + i, sizeof(char), 1);
		if (dst[i] == '\0') break;
	}
	return;
}
