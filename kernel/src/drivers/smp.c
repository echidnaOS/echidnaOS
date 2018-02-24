#include <kernel.h>
#include <stdint.h>
#include <klib.h>
#include <apic.h>
#include <acpi.h>
#include <system.h>
#include <paging.h>

#define CPU_STACK_SIZE 4096

void ap_kernel_entry(int cpu_number) {
    /* APs jump here after initialisation */

    //kprint(KPRN_INFO, "SMP: Started up AP #%u", cpu_number);
    for (;;);

    return;
}

static int start_ap(uint8_t, int);

void init_aps(void) {
    /* start up the APs and place them in a "sleep" state until further notice */

    for (size_t i = 1; i < local_apic_ptr; i++) {
        kprint(KPRN_INFO, "SMP: Starting up AP #%u", i);
        if (start_ap(local_apics[i]->apic_id, i)) {
            kprint(KPRN_ERR, "SMP: Failed to start AP #%u", i);
            continue;
        }
        /* wait a bit */
        sleep(10);
    }

    return;
}

void *prepare_smp_trampoline(void *, void *, pt_entry_t *, uint8_t *, int);
int check_ap_flag(void);
extern void *GDT;

static int start_ap(uint8_t target_apic_id, int cpu_number) {
    /* allocate a new stack for the CPU */
    uint8_t *cpu_stack = kalloc(CPU_STACK_SIZE);

    void *trampoline = prepare_smp_trampoline(ap_kernel_entry, &GDT, kernel_pagemap, cpu_stack + CPU_STACK_SIZE - 0x10, cpu_number);

    /* Send the INIT IPI */
    lapic_write(APICREG_ICR1, ((uint32_t)target_apic_id) << 24);
    lapic_write(APICREG_ICR0, 0x4500);
    /* wait 10ms */
    sleep(10);
    /* Send the Startup IPI */
    lapic_write(APICREG_ICR1, ((uint32_t)target_apic_id) << 24);
    lapic_write(APICREG_ICR0, 0x4600 | (uint32_t)trampoline);
    /* wait 1ms */
    sleep(1);
    if (check_ap_flag()) {
        return 0;
    } else {
        /* Send the Startup IPI again */
        lapic_write(APICREG_ICR1, ((uint32_t)target_apic_id) << 24);
        lapic_write(APICREG_ICR0, 0x4600 | (uint32_t)trampoline);
        /* wait 1s */
        sleep(1000);
        if (check_ap_flag())
            return 0;
        else
            return -1;
    }
}
