/*
 * Currently,
 * 'rm' is only a placeholder.
 */

#include <stdio.h>
#include <stdlib.h>

#include "coreinutils.h"

#define PROG_NAME "rm"

void usage(int argc, char** argv) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "\n"
            "%s [FILES]...\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if (argc == 1)
        usage(argc, argv);

    for (size_t i = 1; i < argc; i++)
        if (remove(argv[i])) {
            fputs("Error deleting file: No such file or directory\n", stderr);
            exit(EXIT_FAILURE);
        }

    return 0;
}
