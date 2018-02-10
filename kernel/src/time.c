#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <tty.h>

uint64_t uptime_raw = 0;
uint64_t uptime_sec = 0;
static int tty_refresh_timeout = 0;

void timer_interrupt(void) {

    if (!(uptime_raw % KRNL_PIT_FREQ))
        uptime_sec++;
    uptime_raw++;

    if (tty_needs_refresh != -1) {
        if (!(uptime_raw % TTY_REDRAW_LIMIT)) {
            tty_refresh(tty_needs_refresh);
            tty_needs_refresh = -1;
        }
    }

    return;
}
