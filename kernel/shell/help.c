#include <stdio.h>
#include "shell.h"

void help_cmd(void) {
    puts(
        "echidnaOS internal shell.\n"
        "Available commands:\n"
        "\n"
        "clear - clears the screen.\n"
        "panic - triggers a kernel panic.\n"
        "help - prints this guide."
    );
    return;
}
