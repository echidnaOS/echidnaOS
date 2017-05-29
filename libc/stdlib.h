#ifndef __STDLIB_H__
#define __STDLIB_H__


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


#endif
