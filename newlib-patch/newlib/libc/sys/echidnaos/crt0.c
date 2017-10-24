#include <fcntl.h>

asm (
    ".global _start\n\t"
    "_start:\n\t"
    "jmp _start_c\n\t"
    ".ascii \"ECH_EXEC\\0\"\n\t"
    ".align 16\n\t"
);

#include "sys_api.h"

int main(int, char**);
void _init_signal(void);

void _start_c(void) {

    int argc = *((int*)0x1000);
    char** argv = (char**)0x1010;
    
    _init_signal();

    int exit_code = main(argc, argv);

    OS_exit(exit_code);

}
