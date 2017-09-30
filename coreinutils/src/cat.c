#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_FILES 4096

const uint16_t LINE_PAD_SIZE = 6;

int line_num = 1, nopt = 0, fnum = 0;

char* files[MAX_FILES];

void cat(FILE* stream) {
    int c = 0;
    int start_of_line = 1;
    while ((c = fgetc(stream)) != EOF) {
        if (nopt && start_of_line) {
            int num = line_num++;
            char buf[21] = "";     /* 2^64 is 20 digits long */
            char* str = buf + 21;   /* where to print from */
            char line_num_size = 0; /* amount of chars printed */

            for (; num > 0; num = num / 10) {
                *(--str) = (num % 10) + '0';
                line_num_size++;
            }

            while (line_num_size++ < LINE_PAD_SIZE)
                putchar(' ');

            fputs(str, stdout);
            putchar('\t');

            start_of_line = 0;
        }

        if (c == '\n')
            start_of_line = 1;

        putchar(c);
    }
}

void parseargs(int argc, char** argv) {
    for (uint16_t i = 1; i < argc; i++) {
        if (!strcmp("-", argv[i])) {
            files[fnum] = malloc(7);
            strcpy(files[fnum], "stdin");
            fnum++;
        } else if (!strcmp("-n", argv[i])) {
            nopt = 1;
        } else {
            if (fopen(argv[i], "r+b") == NULL) {
                if (fopen(argv[i], "rb") != NULL)
                    fputs("cannot open file: is directory\n", stderr);
                else
                    fputs("cannot open file.\n", stderr);

                exit(EXIT_FAILURE);
            } else {
                files[fnum] = malloc(strlen(argv[i]) + 1);
                strcpy(files[fnum], argv[i]);
                fnum++;
            }
        }
    }
}

int main(int argc, char** argv) {
    parseargs(argc, argv);

    if (argc == 1)
        cat(stdin);
    else
        for (uint16_t i = 0; i < fnum; i++) {
            if (!strcmp("stdin", files[i])) {
                cat(stdin);
            } else {
                FILE* fp = fopen(files[i], "rb");
                cat(fp);
            }
        }
    return 0;
}
