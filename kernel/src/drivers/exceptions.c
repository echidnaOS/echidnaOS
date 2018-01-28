#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <task.h>
#include <tty.h>
#include <cio.h>

static void generic_exception(uint32_t error_code, uint32_t fault_eip, uint32_t fault_cs, const char *fault_name, const char *extra) {

    text_putchar('\n', current_tty);
    tty_kputs(fault_name, current_tty);
    tty_kputs(" occurred at: ", current_tty);
    tty_kprn_x(fault_cs, current_tty);
    text_putchar(':', current_tty);
    tty_kprn_x(fault_eip, current_tty);

    if (extra) {
        text_putchar('\n', current_tty);
        tty_kputs(extra, current_tty);
    }

    tty_kputs("\nError code: ", current_tty);
    tty_kprn_ui(error_code, current_tty);

    if (fault_cs == 0x08) {
        tty_kputs("\nThe fault happened in kernel space, system will be halted.", current_tty);
        SYSTEM_HALT;
    }

    tty_kputs("\nTask terminated.\n", current_tty);
    task_quit(current_task, -1);

}

void except_div0(uint32_t fault_eip, uint32_t fault_cs) {

    generic_exception(0, fault_eip, fault_cs, "Division By Zero", NULL);

}

void except_gen_prot_fault(uint32_t error_code, uint32_t fault_eip, uint32_t fault_cs) {

    generic_exception(error_code, fault_eip, fault_cs, "General Protection Fault", NULL);

}

void except_page_fault(uint32_t error_code, uint32_t fault_eip, uint32_t fault_cs) {

    generic_exception(error_code, fault_eip, fault_cs, "Page Fault", NULL);

}
