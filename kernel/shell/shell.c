#include <stdio.h>
#include <string.h>
#include "shell.h"
#include <kernel.h>

// kernel side shell, internal shell, started by kernel init

void kernel_shell(void) {
    char input[256]={0};

    puts("\nKernel shell: \n");

    while (1) {
        printf("> ");

        keyboard_getstring(input, 256);

        text_putchar('\n');

        if (!strcmp("clear", input))
            text_clear();

        else if (!strcmp("panic", input))
            panic("manually triggered panic");

        /*
        else if (!strcmp("peek", input))
            peek(&input[4]);
        */

        /*
        else if (!strcmp("serial_send", input))
            message_serial(&input[11]);
        */

        // return to prompt if no input
        else if (input[0]==0)
            continue;

        // if the input did not match any command
        else
            puts("Invalid command.");
    }
}
