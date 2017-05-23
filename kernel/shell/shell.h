#ifndef __SHELL_H__
#define __SHELL_H__


#include <stdint.h>

void kernel_shell(void);
void help_cmd(void);
void getstring(char* string, uint32_t limit);
int get_argc(const char* string);
void get_argv(char** argv, char* string);


#endif
