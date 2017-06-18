#include <stdio.h>
#include <string.h>

#include "cmds.h"

void help_cmd(int s_argc, char** s_argv) {
    if (s_argc == 1)
        puts(
            "echidnaOS built-in shell.\n"
            "Available commands:\n"
            "\n"
            "clear - clears the screen.\n"
            "help - prints this guide.\n"
            "echo - prints stuff on the screen.\n"
            "cowsay - make someone say stuff\n"
            "\n"
            "Use Shift+Function_keys to switch tty\n"
            "Type `help [command]` to print some info about the command"
        );
    if ((s_argc == 2) && (!strcmp("clear", s_argv[1])))
        puts(
            "clear uses the system's API to clear the screen.\n"
            "Usage:\n"
            "clear"
        );
    else if ((s_argc == 2) && (!strcmp("help", s_argv[1])))
        puts(
            "help, well... prints the help.\n"
            "Usage:\n"
            "help [arguments]"
        );
    else if ((s_argc == 2) && (!strcmp("echo", s_argv[1])))
        puts(
            "echo prints stuff on your screen\n"
            "Usage:\n"
            "echo [text to print...]"
        );
    else if ((s_argc == 2) && (!strcmp("cowsay", s_argv[1])))
        puts(
            "cowsay prints your text with a selected character.\n"
            "Usage:\n"
            "{d,b,g,p,s,t}cowsay [text to say...]\n"
            "tuxsay [text to say...]\n"
            "daemonsay [text to say...]"
        );
    return;
}
