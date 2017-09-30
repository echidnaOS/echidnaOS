#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

void exit(int);

char *itoa(int, char *, int);
char *ltoa(long, char *, int);
char *lltoa(long long, char *, int);
int atoi(const char *);
long atol(const char *);
//long long atoll(const char *);
long strtol(char *, char **, int);
long strtoul(char *, char **, int);
void *endian_swap(void *, int);
char *ftoa(float, char *, int);
char *dtoa(double, char *, int);

void* malloc(size_t size);
void* realloc(void* addr, size_t new_size);
void free(void* ptr);


#endif
