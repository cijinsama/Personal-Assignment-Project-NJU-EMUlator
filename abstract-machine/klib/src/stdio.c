#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
	va_list ap;
	char temp[512];
	va_start(ap, fmt);
	
	sprintf(temp, fmt, ap);
	putstr(temp);

	va_end(ap);
	return strlen(temp);
	//注意，这里没有实现错误的时候返回负数
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}


int sprintf(char *out, const char *fmt, ...) {
	char buffer[32];
	va_list ap;
	int d;
	char c;
	char *s;
	char *record_out = out;
	va_start(ap, fmt);
	while (*fmt) {
		if (*fmt == '\\') {
			*out++ = *++fmt;
			fmt++;
		}
		else {
			if (*fmt == '%'){
				fmt++;
			 	switch (*fmt++) {//每个分支自己管理out的变化，统一管理fmt的变化
				case 's':              /* string */
					s = va_arg(ap, char *);
					strcpy(out, s);
					out += strlen(s);
					break;
				case 'd':              /* int */
					//这里应该倒序输出！！！
					d = va_arg(ap, int);
					if (d == 0) *out++ = '0';
					else {
						if (d < 0) *out++ = '-', d = -d;
						int i = 0;
						while(d != 0) {
							buffer[i++] = d%10 + '0';
							d = d/10;
						}
						while(--i >= 0) *out++ = buffer[i];
					}
					break;
				case 'c':             /* char */
															/* need a cast here since va_arg only
															*   takes fully promoted types */
					c = (char) va_arg(ap, int);
					*out++ = c;
					break;
				default :
					panic("uncompleted\n");
				}
			}
			else *out++ = *fmt++;
		}
	}
	*out = '\0';
	va_end(ap);
	return out - record_out;
}


int snprintf(char *out, size_t n, const char *fmt, ...) {
	panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
	panic("Not implemented");
}

#endif
