#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
	size_t i;
	for(i = 0; src[i] != '\0'; i++) {
		dst[i] = src[i];
	}
	dst[i] = '\0';
	return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
	size_t i;
	for(i = 0; (i < n) && (src[i] != '\0'); i++) {
		dst[i] = src[i];
	}
	for(; i < n; i++){
		dst[i] = '\0';
	}
	return dst;
}

char *strcat(char *dst, const char *src) {
  panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
	char *ps1 = (char *)s1, *ps2 = (char *)s2;
	while(*ps1 != '\0' && *ps2 != '\0') {
		if (*ps1 > *ps2){
			return 1;
		}
		else if (*ps1 < *ps2){
			return -1;
		}
		ps1++, ps2++;
	}
	if (*ps1 != '\0') {
		return 1;
	}
	else if (*ps2 != '\0'){
		return -1;
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	char *ps1 = (char *)s1, *ps2 = (char *)s2;
	while(*ps1 != '\0' && *ps2 != '\0' && n>0) {
		if (*ps1 > *ps2){
			return 1;
		}
		else if (*ps1 < *ps2){
			return -1;
		}
		ps1++, ps2++, n--;
	}
	if (n == 0){
		return 0;
	}
	else if (*ps1 != '\0') {
		return 1;
	}
	else if (*ps2 != '\0'){
		return -1;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
