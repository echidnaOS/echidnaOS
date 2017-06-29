#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "cmds.h"

void getstring(char* string, uint32_t limit);
int get_argc(const char* string);
void get_argv(char** argv, char* string);

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

        if (!strcmp("help", s_argv[0]))
            help_cmd(s_argc, s_argv);

        else if (!strcmp("echo", s_argv[0]))
            echo_cmd(s_argv, s_argc);

        else if ((!strcmp("cowsay", s_argv[0])) || (!strcmp("tuxsay", s_argv[0])) || (!strcmp("daemonsay", s_argv[0])))
            cowsay_cmd(s_argv, s_argc);

        else if ((!strcmp("bcowsay", s_argv[0])) || (!strcmp("dcowsay", s_argv[0])) || (!strcmp("gcowsay", s_argv[0])))
            cowsay_cmd(s_argv, s_argc);

        else if ((!strcmp("pcowsay",s_argv[0])) || (!strcmp("scowsay", s_argv[0])) || (!strcmp("tcowsay", s_argv[0])))
            cowsay_cmd(s_argv, s_argc);

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
    int c;
    while (1) {
        c = getchar();
        if (c=='\b') {
            if (x) {
                x--;
            }
        } else if (c=='\n') break;
        else if (x<(limit-1)) {
            string[x] = c;
            x++;
        }
    }
    string[x] = 0x00;
    return;
}
