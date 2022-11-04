#include <elf.h>

typedef struct{
	char name[32];
	Elf32_Addr min;
	Elf32_Addr max;
} func_add;
extern func_add func_table[];

extern int func_table_size;

int ini_func_table(char *file);
