#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "coreinutils.h"

#define PROG_NAME "printf"

void usage(int argc, char** argv) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "%s [STRING_FORMAT] [ARGUMENTS]...\n"
            "\n"
            "STRING_FORMAT:\n"
            "\\: \" \\ a b n t v e r f\n"
            "%%: %% s d\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    char* str;
    uint32_t len = 0, octal = 0, curargs = 2;

    uint64_t integer = 0;

    if (argc == 1)
        usage(argc, argv);
    else if (argc == 2)
        curargs = 0;

    str = argv[1];
/*
    for (uint16_t i = 1; i < argc; i++)
        len += strlen(argv[i]) + 2;

    str = malloc(len + 1);
    if (str == NULL) {
        fputs("malloc error\n", stderr);
        exit(EXIT_FAILURE);
    }
    strcpy(str, "");

    for (uint16_t i = 1; i < argc; i++) {
        strcat(str, argv[i]);
        if (i != argc - 1)
            strcat(str, " ");
    }
*/

    for (; *str; str++) {
        if (*str == '\\') {
            str++;
            switch (*str) {
                case '"':
                    putchar('"');
                    break;
                case '\\':
                    putchar('\\');
                    break;
                case 'a':
                    putchar('\a');
                    break;
                case 'b':
                    putchar('\b');
                    break;
                case 'n':
                    putchar('\n');
                    break;
                case 't':
                    putchar('\t');
                    break;
                case 'v':
                    putchar('\v');
                    break;
                case 'e':
                    putchar('\e');
                    break;
                case 'f':
                    putchar('\f');
                    break;
                case 'r':
                    putchar('\r');
                    break;
                default:
                    putchar(*str);
                    break;
            }
        } else if (*str == '%') {
            str++;
            switch (*str) {
                case 's':
                    if (!curargs) break;
                    if (curargs >= argc) break;
                    fputs(argv[curargs], stdout);
                    curargs++;
                    break;
                case 'd':
                    if (!curargs) break;
                    if (curargs >= argc) break;
                    integer = atoi(argv[curargs]);
                    printf("%d", integer);
                    curargs++;
                    break;
                case '%':
                    putchar('%');
                    break;
                    
            }
        } else {
            putchar(*str);
        }
    }

    return 0;
}
