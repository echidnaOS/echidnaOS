#include <stdio.h>

#include "cmds.h"

void help_cmd(void) {
    puts(
        "echidnaOS built-in shell.\n"
        "Available commands:\n"
        "\n"
        "clear - clears the screen.\n"
        "help - prints this guide.\n"
        "echo - prints stuff on the screen.\n"
        "cowsay - make someone say stuff\n"
        "\n"
        "Use shift+function keys to switch tty!"
    );
    return;
}
