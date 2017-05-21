#ifndef __SHELL_H__
#define __SHELL_H__


#include <stdint.h>
#include "peek.h"

void kernel_shell(void);
void help_cmd(void);
void sect_cmd(void);
void getstring(char* string, uint32_t limit);
void get_args(char *args, char *string);


#endif

