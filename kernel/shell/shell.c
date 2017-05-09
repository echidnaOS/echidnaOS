#include <stdio.h>
#include <string.h>
#include "shell.h"
#include <kernel.h>

// kernel side shell, internal shell, started by kernel init

void kernel_shell(void) {
    uint32_t command_count=0;
    char input[256]={0};
    char args[256]={0};

    puts(
        "\n"
        "Kernel shell.\n"
        "\n"
        "Type `help` to get started.\n"
    );

    while (1) {
        printf("(%d)# ", command_count++);

        keyboard_getstring(input, 256);
        get_args(args, input);

        text_putchar('\n');

        if (!strcmp("clear", input))
            text_clear();

        else if (!strcmp("panic", input))
            panic("manually triggered panic");

        else if (!strcmp("help", input))
            help_cmd();

        else if (!strcmp("peek", input))
            peek(args);
        
        else if (!strcmp("page", input))
            printf("%d\n", available_page);

        /*
        else if (!strcmp("serial_send", input))
            message_serial(&input[11]);
        */

        // return to prompt if no input
        else if (!input[0]) {
            command_count--;
            continue;
        }

        // if the input did not match any command
        else {
            command_count--;
            printf("shell: invalid command: `%s`.\n", input);
        }

    }
}

void get_args(char *args, char *string) {
    uint32_t index=0;
    
    while ((string[index]!=' ') && (string[index]!='\0'))
        index++;
    
    if (string[index] == ' ')
        string[index++] = 0;
    else {
        args[0] = 0;
        return;
    }
    
    strcpy(args, &string[index]);
    
    return;
}
