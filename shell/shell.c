#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys_api.h>

void getstring(char* string, uint32_t limit);
int get_argc(const char* string);
void get_argv(char** argv, char* string);

void help_cmd(void);
void echo_cmd(char** s_argv, int s_argc);

// built in shell

int main(int argc, char** argv) {
    char* input = malloc(256);
    int s_argc;
    char* s_argv[128];

    puts(
        "\n"
        "Kernel built-in shell.\n"
        "\n"
        "Type `help` to get started.\n"
    );

    while (1) {
        printf("# ");

        getstring(input, 256);
        s_argc = get_argc(input);
        get_argv(s_argv, input);

        putchar('\n');

        if (!strcmp("clear", s_argv[0]))
            OS_cls();          // direct call to the echidnaOS API

        else if (!strcmp("help", s_argv[0]))
            help_cmd();

        else if (!strcmp("echo", s_argv[0]))
            echo_cmd(s_argv, s_argc);

        // return to prompt if no input
        else if (!input[0]) continue;

        // if the input did not match any command
        else printf("shell: invalid command: `%s`.\n", input);
    }
    
    return 0;
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
    return;
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
        } else if (c=='\n') break;
        else if (x<(limit-1)) {
            string[x] = c;
            x++;
            putchar(c);
        }
    }
    string[x] = 0x00;
    return;
}

void help_cmd(void) {
    puts(
        "echidnaOS built-in shell.\n"
        "Available commands:\n"
        "\n"
        "clear - clears the screen.\n"
        "help - prints this guide.\n"
        "echo - prints stuff on the screen.\n"
        "\n"
        "Use shift+function keys to switch tty!"
    );
    return;
}

void echo_cmd(char** s_argv, int s_argc) {
    int a = 1;

    if (s_argc == 1) return;
    do {
        printf("%s ", s_argv[a]);
        ++a;
    } while (a < s_argc);
    putchar('\n');
    return;
}
