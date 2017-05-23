#include <stdio.h>
#include <string.h>
#include "shell.h"
#include <kernel.h>

// built in shell

void kernel_shell(void) {
    char input[256];
    int argc;
    char* argv[128];

    puts(
        "\n"
        "Kernel shell.\n"
        "\n"
        "Type `help` to get started.\n"
    );

    while (1) {
        printf("# ");

        getstring(input, 256);
        argc = get_argc(input);
        get_argv(argv, input);

        putchar('\n');

        if (!strcmp("clear", argv[0]))
            text_clear();

        else if (!strcmp("help", argv[0]))
            help_cmd();

        // return to prompt if no input
        else if (!input[0]) continue;

        // if the input did not match any command
        else printf("shell: invalid command: `%s`.\n", input);
    }

}

int get_argc(const char* string) {
    uint32_t index=0;
    int argc=0;
    
    while (string[index]) {
        if (string[index] == ' ') {
            index++;
            continue;
        }
        while ((string[index]!=' ') && (string[index]!='\0'))
            index++;
        argc++;
    }
    return argc;
}

void get_argv(char** argv, char* string) {
    uint32_t index=0;
    uint8_t arg=0;
    
    while (string[index]) {
        if (string[index] == ' ') {
            string[index++] = 0;
            continue;
        }
        argv[arg++] = &string[index];
        while ((string[index]!=' ') && (string[index]!='\0'))
            index++;
        string[index++] = 0;
    }
}

void getstring(char* string, uint32_t limit) {
	uint32_t x=0;
	char c;
	while (1) {
		c = getchar();
		if (c=='\b') {
			if (x) {
				x--;
				putchar(c);
			}
		} else if (c=='\n') {
			break;
		} else if (x<(limit-1)) {
			string[x] = c;
			x++;
			putchar(c);
		}
	}
	string[x] = 0x00;
}
