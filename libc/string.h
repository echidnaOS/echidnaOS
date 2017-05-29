#ifndef __STRING_H__
#define __STRING_H__


#include "stddef.h"

size_t strlen(const char *);

int    strcmp(const char *, const char *);
int    strcasecmp(const char *, const char *);
int    strncmp(const char *, const char *, size_t);
int    strncasecmp(const char *, const char *, size_t);

char  *strcpy(char *, const char *);
char  *strncpy(char *, const char *, size_t);
char  *strcat(char *, const char *);
char  *strncat(char *, const char *, size_t);

char  *strchr(char *, int);
char  *strrchr(char *, int);
char  *strstr(char *, const char *);
char  *strpbrk(char *, char *);

void  *memset(void *, int, size_t);
int    memcmp(const void *, const void *, size_t);
void  *memcpy(void *, const void *, size_t);
void  *memmove(void *, const void *, size_t);

void  *memchr(void *, int, size_t);


#endif
