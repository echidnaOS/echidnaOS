#include <stdio.h>
#include <string.h>

#include "cmds.h"

void cowsay_cmd(char** s_argv, int s_argc) {
    int a = 1;
    int b;
    int chars;
    char dashes[128] = "";

    if (s_argc == 1) {
        puts("What will I say? :P");
        return;
    }
    for (chars = 0; a < s_argc; b++) {
        chars = chars + strlen(s_argv[a]) + 1;
        ++a;
    }
    b = 0;
    do {
        strcat(dashes, "-");
        ++b;
    } while (b < chars);
    a = 1;    
    printf(" -%s\n", dashes);
    printf("< ");
    do {
        printf("%s ", s_argv[a]);
        ++a;
    } while (a < s_argc);
    printf(">\n");
    printf(" -%s\n", dashes);
    if (!strcmp("cowsay", s_argv[0]))
        puts("\n        \\   ^__^\n"
             "         \\  (oo)\\_______\n"
             "            (__)\\       )\\/\\\n"
             "                ||----w |\n"
             "                ||     ||");
    else if (!strcmp("tuxsay", s_argv[0]))
        puts("\n   \\\n"
             "    \\\n"
             "        .--.\n"
             "       |o_o |\n"
             "       |:_/ |\n"
             "      //   \\ \\\n"
             "     (|     | )\n"
             "    /'\\_   _/`\\\n"
             "    \\___)=(___/");
    else if (!strcmp("daemonsay", s_argv[0]))
        puts("\n   \\         ,        ,\n"
             "    \\       /(        )`\n"
             "     \\      \\ \\___   / |\n"
             "            /- _  `-/  '\n"
             "           (/\\/ \\ \\   /\\\n"
             "           / /   | `    \\\n"
             "           O O   ) /    |\n"
             "           `-^--'`<     '\n"
             "          (_.)  _  )   /\n"
             "           `.___/`    /\n"
             "<----.     __ / __   \\\n"
             "<----|====O)))==) \\) /====\n"
             "<----'    `--' `.__,' \\\n"
             "             |        |\n"
             "              \\       /\n"
             "        ______( (_  / \\______\n"
             "      ,'  ,-----'   |        \\\n"
             "      `--{__________)        \\/");
}
