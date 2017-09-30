#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coreinutils.h"

#define PROG_NAME "cp"

void usage(int argc, char** argv, int exitcode) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "\n"
            "%s [-v, --version, -h, --help] [SOURCE FILE] [DESTINATION]\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(exitcode);
}

int main(int argc, char** argv) {
    if (argc == 2) {
        if (!strcmp("-h", argv[1])
        || (!strcmp("--help", argv[1])))
            usage(argc, argv, 0);

        if (!strcmp("-v", argv[1])
        || (!strcmp("--version", argv[1]))) {
            fprintf(stderr, "coreinutils %s %s\n", PROG_NAME, VERSION);
            exit(EXIT_SUCCESS);
        }
    }

    if (argc != 3) usage(argc, argv, 1);

    copy(argv[1], argv[2]);

    return 0;
}
