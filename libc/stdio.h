#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define SEEK_SET 0
#define SEEK_END 1
#define SEEK_CUR 2

typedef struct {
    char* path;
    uint64_t stream_ptr;
    uint64_t stream_begin;
    uint64_t stream_end;
} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int putchar(int);
int getchar(void);
int fputc(int, FILE*);
int fgetc(FILE*);
int puts(const char*);
int fputs(const char*, FILE*);
int printf(const char*, ...);
int fprintf(FILE*, const char*, ...);
int vsnprintf(char*, size_t, const char*, va_list);
int fseek(FILE*, long int, int);
long int ftell(FILE*);
void rewind(FILE*);
FILE* fopen(const char*, const char*);
int fclose(FILE*);



#endif
