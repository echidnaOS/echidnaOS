#include <cio.h>
#include <stdint.h>
#include <klib.h>
#include <graphics.h>
#include <mouse.h>

typedef struct {
    uint8_t flags;
    uint8_t x_mov;
    uint8_t y_mov;
    uint8_t res;
} mouse_packet_t;

int mouse_x = 0;
int mouse_y = 0;
int old_mouse_x = 0;
int old_mouse_y = 0;

typedef struct {
    uint32_t bitmap[16 * 16];
} cursor_t;

#define X 0x00ffffff

cursor_t cursor = {
    {
        X, X, X, X, X, X, X, X, 0, 0, 0, 0, 0, 0, 0, 0,
        X, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        X, 0, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        X, 0, 0, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        X, 0, 0, 0, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        X, 0, 0, 0, 0, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        X, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        X, 0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, X, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, X, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, X, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, X, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, X,
    }
};

cursor_t anticursor;

void put_mouse_cursor(int type) {
    if (type) {
        for (size_t x = 0; x < 16; x++) {
            for (size_t y = 0; y < 16; y++) {
                plot_px_direct(old_mouse_x + x, old_mouse_y + y, anticursor.bitmap[x * 16 + y]);
            }
        }
        for (size_t x = 0; x < 16; x++) {
            for (size_t y = 0; y < 16; y++) {
                anticursor.bitmap[x * 16 + y] = get_px_direct(mouse_x + x, mouse_y + y);
            }
        }
    }
    for (size_t x = 0; x < 16; x++) {
        for (size_t y = 0; y < 16; y++) {
            if (cursor.bitmap[x * 16 + y])
                plot_px_direct(mouse_x + x, mouse_y + y, ~(get_px_direct(mouse_x + x, mouse_y + y)) & 0xffffff);
        }
    }
    return;
}

void poll_mouse(void) {
    uint8_t b;
    mouse_packet_t packet;

    int x_mov;
    int y_mov;

    if (((b = port_in_b(0x64)) & 1) && (b & (1 << 5))) {
        /* packet comes from the mouse */
        /* read packet */
        packet.flags = port_in_b(0x60);
        packet.x_mov = port_in_b(0x60);
        packet.y_mov = port_in_b(0x60);

        if (packet.flags & (1 << 0));
            //kprint(0, "left click pressed");

        if (packet.flags & (1 << 1));
            //kprint(0, "right click pressed");

        if (packet.flags & (1 << 2));
            //kprint(0, "mid click pressed");

        if (packet.flags & (1 << 4))
            x_mov = (int8_t)packet.x_mov;
        else
            x_mov = packet.x_mov;

        if (packet.flags & (1 << 5))
            y_mov = (int8_t)packet.y_mov;
        else
            y_mov = packet.y_mov;

        old_mouse_x = mouse_x;
        old_mouse_y = mouse_y;

        if (!(mouse_x + x_mov < 0) && !(mouse_x + x_mov >= edid_width)) {
            mouse_x += x_mov;
            //kprint(0, "mouse_x = %u", mouse_x);
        }

        if (!(mouse_y - y_mov < 0) && !(mouse_y - y_mov >= edid_height)) {
            mouse_y -= y_mov;
            //kprint(0, "mouse_y = %u", mouse_y);
        }

        put_mouse_cursor(1);

    }

    return;

}

static inline void mouse_wait(int type) {
    long timeout = 100000;
    if (!type) {
        while (timeout--) {
            if ((port_in_b(0x64) & 1) == 1) {
                return;
            }
        }
        return;
    } else {
        while (timeout--) {
            if ((port_in_b(0x64) & 2) == 0) {
                return;
            }
        }
        return;
    }
}

static inline void mouse_write(uint8_t value) {
    mouse_wait(1);
    port_out_b(0x64, 0xD4);
    mouse_wait(1);
    port_out_b(0x60, value);
    return;
}

static inline uint8_t mouse_read(void) {
    mouse_wait(0);
    return port_in_b(0x60);
}

void init_mouse(void) {
    mouse_x = edid_width / 2;
    mouse_y = edid_height / 2;

    mouse_wait(1);
    port_out_b(0x64, 0xA8);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();

    return;
}
