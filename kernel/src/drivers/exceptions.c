#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <task.h>

void except_div0(uint32_t fault_eip, uint32_t fault_cs) {

    tty_kputs("\nDivision by zero occurred at: ", 0);
    tty_kprn_x(fault_cs, 0);
    text_putchar(':', 0);
    tty_kprn_x(fault_eip, 0);
    tty_kputs("\nTask terminated.\n", 0);
    task_quit(current_task, -1);

}
