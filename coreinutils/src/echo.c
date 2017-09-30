#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "coreinutils.h"

#define PROG_NAME "echo"

void usage(int argc, char** argv, int exitcode) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "\n"
            "%s [-h, --help || -v, --version] (-n) [TEXT]...\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(exitcode);
}

int main(int argc, char** argv) {
    int nopt = 0;

    if (!strcmp("-n", argv[1]))
        nopt = 1;
    else if ((!strcmp("--help", argv[1]))
    || (!strcmp("-h", argv[1])))
        usage(argc, argv, 0);
    else if ((!strcmp("--version", argv[1]))
    || (!strcmp("-v", argv[1]))) {
        fprintf(stderr, "coreinutils %s %s\n", PROG_NAME, VERSION);
        exit(EXIT_SUCCESS);
    }

    if (nopt)
        for (uint16_t i = 2; i < argc; i++) {
            fputs(argv[i], stdout);
            if (i != argc - 1)
                putchar(' ');
        }
    else {
        for (uint16_t i = 1; i < argc; i++) {
            fputs(argv[i], stdout);
            if (i != argc - 1)
                putchar(' ');
        }
        putchar('\n');
    }

    return 0;
}
