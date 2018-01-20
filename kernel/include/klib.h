#ifndef __KLIB_H__
#define __KLIB_H__

#include <stdint.h>
#include <stddef.h>


size_t kmemcpy(char*, const char*, size_t);
size_t kstrcpy(char*, const char*);
int kstrcmp(const char*, const char*);
int kstrncmp(const char*, const char*, size_t);
size_t kstrlen(const char*);
void* kalloc(size_t);
void kfree(void*);
void* krealloc(void*, size_t);
uint64_t power(uint64_t, uint64_t);
void kputs(const char*);
void tty_kputs(const char*, int);
void knputs(const char*, size_t);
void tty_knputs(const char*, size_t, int);
void kprn_ui(uint64_t);
void tty_kprn_ui(uint64_t, int);
void kprn_x(uint64_t);
void tty_kprn_x(uint64_t, int);





#endif
