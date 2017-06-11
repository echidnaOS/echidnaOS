#include <kernel.h>

// prints a char to the current standard output
// for now, it will just print raw to the text driver
void char_to_stdout(int c) {
    text_putchar(c);
    return;
}

// gets a char from the current standard input
// for now, it will get the char from the keyboard buffer
// returns -2 (NO_INPUT) if no input
// returns char on input
// returns EOF on EOF (not implemented yet)
// DOES NOT HALT EXECUTION UNTIL INPUT
#define NO_INPUT -2

int char_from_stdin(void) {
    int c;
    if ((c=keyboard_fetch_char())=='\0')
        c = NO_INPUT;
    return c;
}
