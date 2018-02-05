#include <stdint.h>
#include <graphics.h>
#include <tty.h>
#include <klib.h>
#include <dev.h>

static char *fb_names[] = {
    "fb0", "fb1", "fb2", "fb3", "fb4", "fb5", "fb6", "fb7",
    "fb8", "fb9", "fb10", "fb11", "fb12", "fb13", "fb14", "fb15"
};

static int fb_io_wrapper(uint32_t which_tty, uint64_t loc, int type, uint8_t payload) {
    if (loc >= edid_width * edid_height * 4)
        return -1;
    if (type == 0) {
        return *((uint8_t *)tty[which_tty].field + loc);
    }
    else if (type == 1) {
        *((uint8_t *)tty[which_tty].field + loc) = payload;
        if (current_tty == which_tty)
            *((uint8_t *)framebuffer + loc) = payload;
        return 0;
    }
}

void init_fb(void) {

    kprint(KPRN_INFO, "Initialising framebuffer devices...");

    for (int i = 0; i < KRNL_TTY_COUNT; i++)
        kernel_add_device(fb_names[i], i, edid_width * edid_height * 4, &fb_io_wrapper);

}
