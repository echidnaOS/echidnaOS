#include <kernel.h>

void panic(const char *msg) {
    text_set_text_palette(0x4E, current_tty);
    text_clear(current_tty);
    text_disable_cursor(current_tty);
    tty_kputs("\n!!! KERNEL PANIC !!!\n\nError info: ", current_tty);
    tty_kputs(msg, current_tty);
    tty_kputs("\n\nSYSTEM HALTED", current_tty);
    asm volatile ("hlt;");
}
