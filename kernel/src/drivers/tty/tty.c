#include <stdint.h>
#include <kernel.h>
#include <dev.h>
#include <tty.h>

static char *tty_names[] = {
    "tty0", "tty1", "tty2", "tty3",
    "tty4", "tty5", "tty6", "tty7",
    "tty8", "tty9", "tty10", "tty11"
};

int tty_io_wrapper(uint32_t tty, uint64_t unused, int type, uint8_t payload) {

    if (type == 1) {
        text_putchar(payload, tty);
        return 0;
    } else if (type == 0) {
        return keyboard_fetch_char(tty);
    }

}

void init_tty_drv(void) {

    for (int i = 0; i < KRNL_TTY_COUNT; i++)
        kernel_add_device(tty_names[i], i, 0, &tty_io_wrapper);

    return;

}
