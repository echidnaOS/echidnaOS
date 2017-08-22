#include <stdint.h>
#include <kernel.h>

void except_div0(uint32_t fault_eip, uint32_t fault_cs) {

    tty_kputs("\nDivision by zero occurred at: ", 0);
    tty_kxtoa(fault_cs, 0);
    text_putchar(':', 0);
    tty_kxtoa(fault_eip, 0);
    tty_kputs("\nTask terminated.\n", 0);
    task_terminate(current_task);
    task_scheduler();

}
