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

typedef struct {
    int id;
    int rel_x;
    int rel_y;
    int titlebar;
    int top_border;
    int bottom_border;
    int right_border;
    int left_border;
} window_click_data_t;

window_click_data_t window_from_coordinates(int x, int y) {
    window_click_data_t ret = {0};
    window_t *wptr = windows;

    if (!wptr)
        goto fail;

    /* get number of nodes */
    size_t nodes;
    for (nodes = 1; wptr->next; nodes++, wptr = wptr->next);

    for (;; nodes--) {
        wptr = windows;
        for (size_t i = 0; i < nodes; i++)
            wptr = wptr->next;

        if (x >= wptr->x && x < wptr->x + wptr->x_size + 2 &&
            y >= wptr->y && y < wptr->y + wptr->y_size + 1 + TITLE_BAR_THICKNESS) {
            int titlebar = 0;
            int top_border = 0;
            int bottom_border = 0;
            int right_border = 0;
            int left_border = 0;
            if (y - wptr->y < TITLE_BAR_THICKNESS
                && y - wptr->y > 0 && y - wptr->y < wptr->y_size + TITLE_BAR_THICKNESS
                && x - wptr->x > 0 && x - wptr->x < wptr->x_size)
                titlebar = 1;
            if (y - wptr->y == 0)
                top_border = 1;
            else if (y - wptr->y == wptr->y_size + TITLE_BAR_THICKNESS)
                bottom_border = 1;
            if (x - wptr->x == 0)
                left_border = 1;
            else if (x - wptr->x == wptr->x_size + 1)
                right_border = 1;
            ret.id = wptr->id;
            ret.rel_x = x - (wptr->x - 1);
            ret.rel_y = y - (wptr->y - TITLE_BAR_THICKNESS);
            ret.titlebar = titlebar;
            ret.top_border = top_border;
            ret.bottom_border = bottom_border;
            ret.right_border = right_border;
            ret.left_border = left_border;
            return ret;
        }

        if (!nodes)
            break;
    }

fail:
    ret.id = -1;
    return ret;
}

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
                plot_px(mouse_x + x, mouse_y + y, cursor.bitmap[x * 16 + y]);
        }
    }
    return;
}

int right_click_pressed = 0;
int drag_lock = -1;
int vertical_resize_lock = -1;
int horizontal_resize_lock = -1;

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
        window_click_data_t clicked_window = window_from_coordinates(mouse_x, mouse_y);

        if (packet.flags & (1 << 0)) {
            if (clicked_window.id != -1 && current_window != clicked_window.id)
                window_focus(clicked_window.id);
            if (clicked_window.titlebar)
                drag_lock = clicked_window.id;
            else
                drag_lock = -1;
            if (clicked_window.bottom_border)
                vertical_resize_lock = clicked_window.id;
            else
                vertical_resize_lock = -1;
            if (clicked_window.right_border)
                horizontal_resize_lock = clicked_window.id;
            else
                horizontal_resize_lock = -1;
            right_click_pressed = 1;
        } else {
            right_click_pressed = 0;
            drag_lock = -1;
            vertical_resize_lock = -1;
            horizontal_resize_lock = -1;
        }

        if (packet.flags & (1 << 1));

        if (packet.flags & (1 << 2));

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

        if (right_click_pressed && drag_lock != -1) {
            window_move(mouse_x - old_mouse_x, mouse_y - old_mouse_y, drag_lock);
        }
        if (right_click_pressed && vertical_resize_lock != -1) {
            window_resize(0, mouse_y - old_mouse_y, vertical_resize_lock);
        }
        if (right_click_pressed && horizontal_resize_lock != -1) {
            window_resize(mouse_x - old_mouse_x, 0, horizontal_resize_lock);
        }

        gui_needs_refresh = 1;
        //put_mouse_cursor(1);

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
