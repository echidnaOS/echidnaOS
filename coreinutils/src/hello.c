#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coreinutils.h"

#define PROG_NAME "hello"

void grab_arguments(int argc, char** argv);
void print_help(int argc, char** argv);
void print_greeting(int argc, char** argv);

int version_opt = 0, greeting_opt = 0, default_opt = 0,
    help_opt = 0;

int main(int argc, char** argv) {
        grab_arguments(argc, argv);

        if (version_opt) {
                fprintf(stderr, "coreinutils %s %s\n", PROG_NAME, VERSION);
                exit(EXIT_SUCCESS);
        } else if (default_opt) {
                fputs("Hello, world!\n", stdout);
                exit(EXIT_SUCCESS);
        } else if (help_opt) {
                print_help(argc, argv);
        } else if (greeting_opt) {
                print_greeting(argc, argv);
                exit(EXIT_SUCCESS);
        }

        return 0;
}

void grab_arguments(int argc, char** argv) {
        if (argc == 2) {
                if ((!strcmp("-v", argv[1]))
                || (!strcmp("--version", argv[1])))
                        version_opt = 1;
                else if ((!strcmp("-t", argv[1]))
                || (!strcmp("--traditional", argv[1])))
                        default_opt = 1;
                else if ((!strcmp("-h", argv[1]))
                || (!strcmp("--help", argv[1])))
                        help_opt = 1;
                else
                        print_help(argc, argv);
        } else if (argc > 2) {
                if ((!strcmp("-g", argv[1]))
                || (!strcmp("--greeting", argv[1])))
                        greeting_opt = 1;
                else
                        print_help(argc, argv);
        } else if (argc == 1) {
                default_opt = 1;
        } 
}

void print_help(int argc, char** argv) {
        fprintf(stderr,
                "coreinutils %s %s\n"
                "Usage: %s [OPTION] ...\n"
                "Print a friendly, customizable greeting.\n"
                "\n"
                "  -h, --help       display this and exit\n"
                "  -v, --version    display version and exit\n"
                "\n"
                "  -t, --traditional"
                "         use traditional greeting\n"
                "  -g, --greeting [TEXT...]"
                "  use TEXT as the greeting message\n",
                PROG_NAME, VERSION, argv[0]
                );
        exit(EXIT_SUCCESS);
}

void print_greeting(int argc, char** argv) {
        for (int i = 2; i < argc; i++)
                fprintf(stdout, "%s ", argv[i]);
        putchar('\n');

        return;
}
