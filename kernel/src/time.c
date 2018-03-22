#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <graphics.h>
#include <apic.h>
#include <acpi.h>
#include <mouse.h>

volatile uint64_t uptime_raw = 0;
volatile uint64_t uptime_sec = 0;

void timer_interrupt(void) {

    if (!(++uptime_raw % KRNL_PIT_FREQ))
        uptime_sec++;

    /* raise vector 32 for all APs */
    lapic_write(APICREG_ICR0, 0x20 | (1 << 18) | (1 << 19));

    if (!(uptime_raw % MOUSE_POLL_LIMIT)) {
        poll_mouse();
    }

    if (gui_needs_refresh) {
        if (!(uptime_raw % GUI_REDRAW_LIMIT)) {
            gui_refresh();
            gui_needs_refresh = 0;
        }
    }

    return;
}
