#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coreinutils.h"

#define PROG_NAME "cp"

void usage(int argc, char** argv) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "\n"
            "%s [SOURCE FILE] [DESTINATION]\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(EXIT_FAILURE);
}

void print_help(int argc, char** argv);
void print_version(void);

int main(int argc, char** argv) {
    FILE* src;
    FILE* dest;
    char* tmp_dest;
    char* fsrc;
    char* fdest;

    if (argc == 2) {
        if (!strcmp("-h", argv[1])
        || (!strcmp("--help", argv[1])))
            print_help(argc, argv);
        if (!strcmp("-v", argv[1])
        || (!strcmp("--version", argv[1])))
            print_version();
    }

    if (argc != 3)
        usage(argc, argv);

    if (fopen(argv[1], "r+b") == NULL) {
        fprintf(stderr, "%s - no such file.\n", argv[1]);
        exit(EXIT_FAILURE);
    } else 
        fsrc = argv[1];

    if (fopen(argv[2], "w+b") == NULL) {
        tmp_dest = malloc(strlen(argv[2]) + strlen(argv[1]) + 25);
        if (tmp_dest == NULL) {
            fputs("Malloc error.\n", stderr);
            exit(EXIT_FAILURE);
        }
        strcpy(tmp_dest, argv[2]);
        if (tmp_dest[strlen(tmp_dest) - 1] != '/')
            strcat(tmp_dest, "/");
        strcat(tmp_dest, argv[1]);
        if (fopen(tmp_dest, "w+b") == NULL) {
            fputs("Cannot write.\n", stderr);
            exit(EXIT_FAILURE);
        } else
            fdest = tmp_dest;
    } else
        fdest = argv[2];

    src = fopen(fsrc, "rb");
    dest = fopen(fdest, "w+b");

    for (int c = fgetc(src); c != EOF; c = fgetc(src))
        fputc(c, dest);

    fclose(src);
    fclose(dest);

    return 0;
}

void print_help(int argc, char** argv) {
    fprintf(stdout,
            "coreinutils %s %s\n"
            "Usage:\n"
            "cp (OPTION) [SOURCE FILE] [DESTINATION]\n"
            "\n"
            "List of options:\n"
            "--help, -h     Print help and exit\n"
            "--version, -v  Print version number and exit\n",
            PROG_NAME, VERSION
            );

    exit(EXIT_SUCCESS);
}

void print_version(void) {
    fprintf(stderr, "coreinutils %s %s\n", PROG_NAME, VERSION);
    exit(EXIT_SUCCESS);
}
