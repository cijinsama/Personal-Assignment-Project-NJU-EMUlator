#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>
#define countint size_t

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	countint i = 0;
	while(s[i] != '\0') { i++; }
	return i;
}

char *strcpy(char *dst, const char *src) {
	countint i;
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
	size_t dest_len = strlen(dst);
	countint i = 0;
	for(i = 0; src[i] != '\0'; i++)
		dst[dest_len + i] = src[i];
	dst[dest_len + i] = '\0';
	return dst;
}

char *strncat(char *dest, const char *src, size_t n)
{
	size_t dest_len = strlen(dest);
	size_t i;
	for (i = 0 ; i < n && src[i] != '\0' ; i++)
		dest[dest_len + i] = src[i];
	dest[dest_len + i] = '\0';
	return dest;
}

int strcmp(const char *s1, const char *s2) {
	unsigned char *ps1 = (unsigned char *)s1, *ps2 = (unsigned char *)s2;
	while(*ps1 != '\0' && *ps2 != '\0') {
		if (*ps1 > *ps2){
			return *ps1 - *ps2;
		}
		else if (*ps1 < *ps2){
			return *ps1 - *ps2;
		}
		ps1++, ps2++;
	}
	return *ps1 - *ps2;
	//if (*ps1 != '\0') {
	//	return *ps1;
	//}
	//else if (*ps2 != '\0'){
	//	return -*ps2;
	//}11
	//return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	unsigned char *ps1 = (unsigned char *)s1, *ps2 = (unsigned char *)s2;
	size_t i = 0;
	while(*ps1 != '\0' && *ps2 != '\0' && i < n) {
		if (*ps1 > *ps2){
			return *ps1 - *ps2;
		}
		else if (*ps1 < *ps2){
			return *ps1 - *ps2;
		}
		ps1++, ps2++, i++;
	}
	if (i == n){
		return 0;
	}
	return *ps1 - *ps2;
	//else if (*ps1 != '\0') {
	//	return *ps1;
	//}
	//else if (*ps2 != '\0'){
	//	return *ps2;
	//}
	//return 0;
}

void *memset(void *s, int c, size_t n) {
	size_t i;
	//这里可能会有错，需要byte的type
	char *src = s;
	for (i = 0; i < n; i++) {
		src[i] = c;
	}
	return 0;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
	unsigned char *ps1 = (unsigned char *)s1, *ps2 = (unsigned char *)s2;
	size_t i = 0;
	while(i < n) {
		if (*ps1 > *ps2){
			return *ps1 - *ps2;
		}
		else if (*ps1 < *ps2){
			return *ps1 - *ps2;
		}
		ps1++, ps2++, i++;
	}
	if (i == n){
		return 0;
	}
	return *ps1 - *ps2;
}

#endif
