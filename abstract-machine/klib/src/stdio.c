#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
	va_list ap;
	char temp[512];
	
	va_start(ap, fmt);
	vsprintf(temp, fmt, ap);
	va_end(ap);

	putstr(temp);
	return strlen(temp);
	//注意，这里没有实现错误的时候返回负数
}


static int PairAInt(const char **fmt){
	int i = 0;
	while(**fmt <= '9' && **fmt >= '0'){
		i = i*10 + **fmt - '0';
		(*fmt)++;
	}
	return i;
}
static void PutAInt(int d, char **out, int zeros_padding_num){
	int i = 0;
	char buffer[32];
	if (d == 0) {
		*((*out)++) = '0';
		i++;
		while(i++ < zeros_padding_num) *(*out)++ = '0';
	}
	else {
		if (d < 0) *(*out)++ = '-', d = -d, zeros_padding_num--;
		while(d != 0) {
			buffer[i++] = d%10 + '0';
			d = d/10;
		}
		while(i < zeros_padding_num) buffer[i++] = '0';
		while(--i >= 0) *(*out)++ = buffer[i];
	}
	return;
}

static void PutAddress(uintptr_t d, char **out){
	int i = 0;
	char buffer[32];
	if (d == 0) {
		*((*out)++) = '0';
		i++;
	}
	else {
		while(d != 0) {
			buffer[i++] = d%10 + '0';
			d = d/10;
		}
		while(--i >= 0) *(*out)++ = buffer[i];
	}
	return;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
	int int_num;
	char c;
	char *s;
	char *record_out = out;
	int zeros_padding_num = 0;
	while (*fmt) {
		zeros_padding_num = 0;
		if (*fmt == '\\') {
			*out++ = *++fmt;
			fmt++;
		}
		else {
			if (*fmt == '%'){
				fmt++;

				if (*fmt == '0') fmt++;
				zeros_padding_num = PairAInt(&fmt);

			 	switch (*fmt++) {//每个分支自己管理out的变化，统一管理fmt的变化
				case 's':              /* string */
					s = va_arg(ap, char *);
					strcpy(out, s);
					out += strlen(s);
					break;
				case 'd':              /* int */
					int_num = va_arg(ap, int);
					PutAInt(int_num, &out, zeros_padding_num);
					break;
				case 'c':             /* char */
															/* need a cast here since va_arg only
															*   takes fully promoted types */
					c = (char) va_arg(ap, int);
					*out++ = c;
					break;
				case 'p':
// 					s = va_arg(ap, uintptr_t);
					PutAddress(va_arg(ap, uintptr_t), &out);
					break;
				default :
					panic("vsprintf uncompleted received args, please go to compelete it!!\n");
				}
			}
			else *out++ = *fmt++;
		}
	}
	*out = '\0';
	//注意，这里没有实现错误的时候返回负数
	return out - record_out;
}


int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int ret = vsprintf(out, fmt, ap);
	va_end(ap);
	return ret;
}


int snprintf(char *out, size_t n, const char *fmt, ...) {
	panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
	panic("Not implemented");
}

#endif
