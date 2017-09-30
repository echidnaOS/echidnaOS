#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "coreinutils.h"

#define PROG_NAME "yes"

void print_help(int argc, char** argv);

int help_opt = 0, version_opt = 0, custom_opt = 0,
    default_opt = 0, len = 0;

int main(int argc, char** argv) {
    if (argc >= 2) {
        if (!strcmp("--help", argv[1]))
            help_opt = 1;
        else if (!strcmp("--version", argv[1]))
            version_opt = 1;
        else
            custom_opt = 1;
    } else if (argc == 1)
        default_opt = 1;

    if (default_opt)
        for (;;)
            puts("y");
    else if (version_opt)
        fprintf(stderr, "coreinutils %s %s\n", PROG_NAME, VERSION);
    else if (help_opt)
        print_help(argc, argv);
    else if (custom_opt)
        for (;;) {
            for (uint16_t i = 1; i < argc; i++) {
                fputs(argv[i], stdout);
                if (i != argc - 1)
                    putchar(' ');
            }
            putchar('\n');
        }

    return 0;
}

void print_help(int argc, char** argv) {
    fprintf(stdout,
            "coreinutils %s %s\n"
            "Usage: %s [STRING]...\n"
            "   or: %s OPTION\n"
            "Repeatedly output a line with all specified STRING(s), or 'y'.\n"
            "\n"
            "      --help     display this help and exit\n"
            "      --version  output version information and exit\n",
            PROG_NAME, VERSION, argv[0], argv[0]
            );

    exit(EXIT_SUCCESS);
}
