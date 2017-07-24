#include <kernel.h>

// prints a char to the current standard output
// for now, it will just print raw to the text driver
void char_to_stdout(int c) {
    text_putchar((char)c, current_task->tty);
    return;
}

void enter_iowait_status(void) {
    current_task->status = KRN_STAT_IOWAIT_TASK;
    return;
}
