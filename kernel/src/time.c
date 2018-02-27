#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <tty.h>
#include <apic.h>
#include <acpi.h>

volatile uint64_t uptime_raw = 0;
volatile uint64_t uptime_sec = 0;

void timer_interrupt(void) {

    if (!(++uptime_raw % KRNL_PIT_FREQ))
        uptime_sec++;

    /* raise vector 32 for all APs */
    for (size_t i = 1; i < local_apic_ptr; i++) {
        lapic_write(APICREG_ICR1, ((uint32_t)local_apics[i]->apic_id) << 24);
        lapic_write(APICREG_ICR0, 0x20);
    }

    if (tty_needs_refresh != -1) {
        if (!(uptime_raw % TTY_REDRAW_LIMIT)) {
            tty_refresh(tty_needs_refresh);
            tty_needs_refresh = -1;
        }
    }

    return;
}
