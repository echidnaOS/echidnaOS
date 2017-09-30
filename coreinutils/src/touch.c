#include <stdio.h>
#include <stdlib.h>

#include "coreinutils.h"

#define PROG_NAME "touch"

void usage(int argc, char** argv) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "%s [FILE]\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if (argc != 2)
        usage(argc, argv);

    if (fopen(argv[1], "w+b") == NULL) {
        fputs("Cannot touch file\n", stderr);
        exit(EXIT_FAILURE);
    }

    return 0;
}
