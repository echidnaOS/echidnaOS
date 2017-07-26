#include <stdint.h>
#include <kernel.h>

void except_div0(uint32_t fault_eip, uint32_t fault_cs) {

    tty_kputs("\nDivision by zero occurred at: ", task_table[current_task]->tty);
    tty_kxtoa(fault_cs, task_table[current_task]->tty);
    text_putchar(':', task_table[current_task]->tty);
    tty_kxtoa(fault_eip, task_table[current_task]->tty);
    tty_kputs("\nTask terminated.", task_table[current_task]->tty);
    task_table[current_task]->status = KRN_STAT_RES_TASK;
    task_switch(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

}
