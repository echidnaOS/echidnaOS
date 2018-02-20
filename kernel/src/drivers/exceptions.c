#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <task.h>
#include <tty.h>
#include <cio.h>
#include <panic.h>

static void generic_exception(size_t error_code, size_t fault_eip, size_t fault_cs, const char *fault_name, const char *extra) {

    kprint(KPRN_ERR, "%s occurred at: %x:%x", fault_name, fault_cs, fault_eip);

    if (extra)
        kprint(KPRN_ERR, "%s", extra);

    kprint(KPRN_ERR, "Error code: %x", error_code);

    if (fault_cs == 0x08)
        panic("Exception occurred in kernel space.", error_code);

    kprint(KPRN_INFO, "PID %u terminated.", current_task);
    task_quit(current_task, -1);

}

void except_div0(size_t fault_eip, size_t fault_cs) {

    generic_exception(0, fault_eip, fault_cs, "Division By Zero", NULL);

}

void except_gen_prot_fault(size_t error_code, size_t fault_eip, size_t fault_cs) {

    generic_exception(error_code, fault_eip, fault_cs, "General Protection Fault", NULL);

}

void except_page_fault(size_t error_code, size_t fault_eip, size_t fault_cs) {

    generic_exception(error_code, fault_eip, fault_cs, "Page Fault", NULL);

}
