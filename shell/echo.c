#include <stdio.h>

#include "cmds.h"

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
