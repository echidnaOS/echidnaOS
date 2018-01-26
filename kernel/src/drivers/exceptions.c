#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <task.h>
#include <tty.h>

static void generic_exception(uint32_t error_code, uint32_t fault_eip, uint32_t fault_cs, const char *fault_name, const char *extra) {

    text_putchar('\n', 0);
    tty_kputs(fault_name, 0);
    tty_kputs(" occurred at: ", 0);
    tty_kprn_x(fault_cs, 0);
    text_putchar(':', 0);
    tty_kprn_x(fault_eip, 0);

    if (extra) {
        text_putchar('\n', 0);
        tty_kputs(extra, 0);
    }

    tty_kputs("\nError code: ", 0);
    tty_kprn_ui(error_code, 0);

    tty_kputs("\nTask terminated.\n", 0);
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
