#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <task.h>
#include <tty.h>
#include <cio.h>
#include <panic.h>
#include <system.h>

static void generic_exception(size_t error_code, size_t fault_rip, size_t fault_cs, const char *fault_name, const char *extra) {

    kprint(KPRN_ERR, "%s occurred at: %x:%x", fault_name, fault_cs, fault_rip);

    if (extra)
        kprint(KPRN_ERR, "%s", extra);

    kprint(KPRN_ERR, "Error code: %x", error_code);

    if (fault_cs == 0x08)
        panic("Exception occurred in kernel space.", error_code);

    kprint(KPRN_INFO, "Exception on CPU #%u", get_cpu_number());

    kprint(KPRN_INFO, "PID %u terminated.", current_task);
    task_quit(current_task, -1);

}

void except_div0(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Division by zero", NULL);

}

void except_debug(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Debug", NULL);

}

void except_nmi(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "NMI", NULL);

}

void except_breakpoint(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Breakpoint", NULL);

}

void except_overflow(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Overflow", NULL);

}

void except_bound_range_exceeded(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Bound range exceeded", NULL);

}

void except_invalid_opcode(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Invalid opcode", NULL);

}

void except_device_not_available(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Device not available", NULL);

}

void except_double_fault(size_t error_code, size_t fault_rip, size_t fault_cs) {

    panic("Double fault", error_code);

}

void except_coprocessor_segment_overrun(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Coprocessor segment overrun", NULL);

}

void except_invalid_tss(size_t error_code, size_t fault_rip, size_t fault_cs) {

    generic_exception(error_code, fault_rip, fault_cs, "Invalid TSS", NULL);

}

void except_segment_not_present(size_t error_code, size_t fault_rip, size_t fault_cs) {

    generic_exception(error_code, fault_rip, fault_cs, "Segment not present", NULL);

}

void except_stack_segment_fault(size_t error_code, size_t fault_rip, size_t fault_cs) {

    generic_exception(error_code, fault_rip, fault_cs, "Stack-segment fault", NULL);

}

void except_gen_prot_fault(size_t error_code, size_t fault_rip, size_t fault_cs) {

    generic_exception(error_code, fault_rip, fault_cs, "General protection fault", NULL);

}

void except_page_fault(size_t error_code, size_t fault_rip, size_t fault_cs) {

    generic_exception(error_code, fault_rip, fault_cs, "Page fault", NULL);

}

void except_x87_exception(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "x87 exception", NULL);

}

void except_alignment_check(size_t error_code, size_t fault_rip, size_t fault_cs) {

    generic_exception(error_code, fault_rip, fault_cs, "Alignment check", NULL);

}

void except_machine_check(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Machine check", NULL);

}

void except_simd_exception(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "SIMD exception", NULL);

}

void except_virtualisation_exception(size_t fault_rip, size_t fault_cs) {

    generic_exception(0, fault_rip, fault_cs, "Virtualisation exception", NULL);

}

void except_security_exception(size_t error_code, size_t fault_rip, size_t fault_cs) {

    generic_exception(error_code, fault_rip, fault_cs, "Security exception", NULL);

}
