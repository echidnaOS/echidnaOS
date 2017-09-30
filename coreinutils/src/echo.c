#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "coreinutils.h"

#define PROG_NAME "echo"

void usage(int argc, char** argv) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "\n"
            "%s (-n) [TEXT]...\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(EXIT_FAILURE);
}

void print_help(int argc, char** argv);
void print_version(void);

int nonl = 0;

int main(int argc, char** argv) {
    if (!strcmp("-n", argv[1]))
        nonl = 1;
    else if ((!strcmp("--help", argv[1]))
    || (!strcmp("-h", argv[1])))
        print_help(argc, argv);
    else if ((!strcmp("--version", argv[1]))
    || (!strcmp("-v", argv[1])))
        print_version();

    if (!nonl) {
        for (size_t i = 1; i < argc; i++)
            printf("%s ", argv[i]);
        putchar('\n');
    } else {
        for (size_t i = 2; i < argc; i++)
            printf("%s ", argv[i]);
    }

    return 0;
}

void print_help(int argc, char** argv) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "\n"
            "%s (-n) [TEXT]...\n"
            "echo - prints TEXT and exits.\n"
            "List of options:\n"
            "\n"
            "   -h, --help      display this help and exit.\n"
            "   -v, --version   display version info and exit\n"
            "\n"
            "   -n          don't output the trailing newline\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(EXIT_SUCCESS);
}

void print_version(void) {
    fprintf(stderr,
            "coreinutils %s %s\n",
            PROG_NAME, VERSION
            );

    exit(EXIT_SUCCESS);
}
