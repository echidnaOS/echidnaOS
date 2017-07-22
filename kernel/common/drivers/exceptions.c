#include <stdint.h>
#include <kernel.h>

void except_div0(uint32_t fault_eip, uint32_t fault_cs) {

    tty_kputs("\nDivision by zero occurred at: ", current_task->tty);
    tty_kxtoa(fault_cs, current_task->tty);
    text_putchar(':', current_task->tty);
    tty_kxtoa(fault_eip, current_task->tty);
    tty_kputs("\nTask terminated.", current_task->tty);
    current_task->status = KRN_STAT_RES_TASK;
    task_switch(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

}
