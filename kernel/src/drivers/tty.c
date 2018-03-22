#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <tty.h>
#include <graphics.h>
#include <panic.h>
#include <bios_io.h>

#define TTY_COLS (wptr->x_size / 8)
#define TTY_ROWS (wptr->y_size / 16)

static void escape_parse(char c, int window);
static void text_set_cursor_pos(uint32_t x, uint32_t y, int window);
static void draw_cursor(int window);
static void clear_cursor(int window);

void plot_char(char c, int x, int y, uint32_t hex_fg, uint32_t hex_bg) {
    int orig_x = x;

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if ((vga_font[c * 16 + i] >> 7 - j) & 1)
                plot_px(x++, y, hex_fg);
            else
                plot_px(x++, y, hex_bg);
        }
        y++;
        x = orig_x;
    }

    return;
}

static void plot_char_window(char c, int x, int y, uint32_t hex_fg, uint32_t hex_bg, int window) {
    int orig_x = x;

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if ((vga_font[c * 16 + i] >> 7 - j) & 1)
                plot_px_window(x++, y, hex_fg, window);
            else
                plot_px_window(x++, y, hex_bg, window);
        }
        y++;
        x = orig_x;
    }

    return;
}

static void plot_char_grid(char c, int x, int y, uint32_t hex_fg, uint32_t hex_bg, int window) {
    window_t *wptr = get_window_ptr(window);
    plot_char_window(c, x * 8, y * 16, hex_fg, hex_bg, window);
    wptr->grid[x + y * TTY_COLS] = c;
    wptr->gridfg[x + y * TTY_COLS] = hex_fg;
    wptr->gridbg[x + y * TTY_COLS] = hex_bg;
    gui_needs_refresh = 1;
    return;
}

static void tty_refresh(int window) {
    window_t *wptr = get_window_ptr(window);
    for (size_t i = 0; i < TTY_COLS * TTY_ROWS; i++) {
        plot_char_grid(wptr->grid[i], i % TTY_COLS, i / TTY_COLS, wptr->gridfg[i], wptr->gridbg[i], window);
    }
    draw_cursor(window);
    gui_needs_refresh = 1;
    return;
}

static void scroll(int window) {
    window_t *wptr = get_window_ptr(window);
    /* notify grid */
    for (int i = TTY_COLS; i < TTY_COLS * TTY_ROWS; i++) {
        wptr->grid[i - TTY_COLS] = wptr->grid[i];
        wptr->gridbg[i - TTY_COLS] = wptr->gridbg[i];
        wptr->gridfg[i - TTY_COLS] = wptr->gridfg[i];
    }
    /* clear the last line of the screen */
    for (int i = TTY_ROWS * TTY_COLS - TTY_COLS; i < TTY_ROWS * TTY_COLS; i++) {
        wptr->grid[i] = ' ';
        wptr->gridbg[i] = wptr->text_bg_col;
        wptr->gridfg[i] = wptr->text_fg_col;
    }
    tty_refresh(window);
    return;
}

static void text_clear(int window) {
    window_t *wptr = get_window_ptr(window);
    for (size_t i = 0; i < (TTY_ROWS * TTY_COLS); i++) {
        wptr->grid[i] = ' ';
        wptr->gridbg[i] = wptr->text_bg_col;
        wptr->gridfg[i] = wptr->text_fg_col;
    }
    wptr->cursor_x = 0;
    wptr->cursor_y = 0;
    tty_refresh(window);
    return;
}

static void text_clear_no_move(int window) {
    window_t *wptr = get_window_ptr(window);
    for (size_t i = 0; i < (TTY_ROWS * TTY_COLS); i++) {
        wptr->grid[i] = ' ';
        wptr->gridbg[i] = wptr->text_bg_col;
        wptr->gridfg[i] = wptr->text_fg_col;
    }
    tty_refresh(window);
    return;
}

static void clear_cursor(int window) {
    window_t *wptr = get_window_ptr(window);
    plot_char_window(wptr->grid[wptr->cursor_x + wptr->cursor_y * TTY_COLS],
        wptr->cursor_x * 8, wptr->cursor_y * 16,
        wptr->text_fg_col, wptr->text_bg_col, window);
    gui_needs_refresh = 1;
    return;
}

static void draw_cursor(int window) {
    window_t *wptr = get_window_ptr(window);
    if (wptr->cursor_status)
        plot_char_window(wptr->grid[wptr->cursor_x + wptr->cursor_y * TTY_COLS],
            wptr->cursor_x * 8, wptr->cursor_y * 16,
            wptr->cursor_fg_col, wptr->cursor_bg_col, window);
    gui_needs_refresh = 1;
    return;
}

static void text_enable_cursor(int window) {
    window_t *wptr = get_window_ptr(window);
    wptr->cursor_status = 1;
    draw_cursor(window);
    return;
}

static void text_disable_cursor(int window) {
    window_t *wptr = get_window_ptr(window);
    wptr->cursor_status = 0;
    tty_refresh(window);
    return;
}

static void text_set_cursor_pos(uint32_t x, uint32_t y, int window) {
    window_t *wptr = get_window_ptr(window);
    wptr->cursor_x = x;
    wptr->cursor_y = y;
    tty_refresh(window);
    gui_needs_refresh = 1;
    return;
}

extern int kernel_log_window;

void text_putchar(char c, int window) {
    window_t *wptr = get_window_ptr(window);

    if (!modeset_done) {
        char cstr[2] = {0,0};
        cstr[0] = c;
        bios_print(cstr);
        return;
    }

    if (!kernel_log_window)
        return;

    if (wptr->escape) {
        escape_parse(c, window);
        return;
    }
    switch (c) {
        case 0x00:
            break;
        case 0x1B:
            wptr->escape = 1;
            return;
        case 0x0A:
            if (wptr->cursor_y == (TTY_ROWS - 1)) {
                text_set_cursor_pos(0, (TTY_ROWS - 1), window);
                scroll(window);
            } else
                text_set_cursor_pos(0, (wptr->cursor_y + 1), window);
            break;
        case 0x08:
            if (wptr->cursor_x || wptr->cursor_y) {
                clear_cursor(window);
                if (wptr->cursor_x)
                    wptr->cursor_x--;
                else {
                    wptr->cursor_y--;
                    wptr->cursor_x = TTY_COLS - 1;
                }
                plot_char_grid(' ', wptr->cursor_x, wptr->cursor_y,
                    wptr->text_fg_col, wptr->text_bg_col, window);
                draw_cursor(window);
            }
            break;
        default:
            plot_char_grid(c, wptr->cursor_x++, wptr->cursor_y,
                wptr->text_fg_col, wptr->text_bg_col, window);
            if (wptr->cursor_x == TTY_COLS) {
                wptr->cursor_x = 0;
                wptr->cursor_y++;
            }
            if (wptr->cursor_y == TTY_ROWS) {
                wptr->cursor_y--;
                if (!wptr->noscroll)
                    scroll(window);
            }
            draw_cursor(window);
    }

    gui_needs_refresh = 1;

    return;
}

static uint32_t ansi_colours[] = {
    0x00000000,              /* black */
    0x00aa0000,              /* red */
    0x0000aa00,              /* green */
    0x00aa5500,              /* brown */
    0x000000aa,              /* blue */
    0x00aa00aa,              /* magenta */
    0x0000aaaa,              /* cyan */
    0x00aaaaaa,              /* grey */
};

static void sgr(int window) {
    window_t *wptr = get_window_ptr(window);

    if (wptr->esc_value0 >= 30 && wptr->esc_value0 <= 37) {
        wptr->text_fg_col = ansi_colours[wptr->esc_value0 - 30];
        return;
    }

    if (wptr->esc_value0 >= 40 && wptr->esc_value0 <= 47) {
        wptr->text_bg_col = ansi_colours[wptr->esc_value0 - 40];
        return;
    }

    return;
}

static void escape_parse(char c, int window) {
    window_t *wptr = get_window_ptr(window);

    if (c >= '0' && c <= '9') {
        *wptr->esc_value *= 10;
        *wptr->esc_value += c - '0';
        *wptr->esc_default = 0;
        return;
    }

    switch (c) {
        case '[':
            return;
        case ';':
            wptr->esc_value = &wptr->esc_value1;
            wptr->esc_default = &wptr->esc_default1;
            return;
        case 'A':
            if (wptr->esc_default0) wptr->esc_value0 = 1;
            if (wptr->esc_value0 > wptr->cursor_y)
                wptr->esc_value0 = wptr->cursor_y;
            text_set_cursor_pos(wptr->cursor_x,
                                wptr->cursor_y - wptr->esc_value0,
                                window);
            break;
        case 'B':
            if (wptr->esc_default0) wptr->esc_value0 = 1;
            if ((wptr->cursor_y + wptr->esc_value0) >
                (TTY_ROWS - 1))
                wptr->esc_value0 = (TTY_ROWS - 1) - wptr->cursor_y;
            text_set_cursor_pos(wptr->cursor_x,
                                wptr->cursor_y + wptr->esc_value0,
                                window);
            break;
        case 'C':
            if (wptr->esc_default0) wptr->esc_value0 = 1;
            if ((wptr->cursor_x + wptr->esc_value0) >
                (TTY_COLS - 1))
                wptr->esc_value0 = (TTY_COLS - 1) - wptr->cursor_x;
            text_set_cursor_pos(wptr->cursor_x + wptr->esc_value0,
                                wptr->cursor_y,
                                window);
            break;
        case 'D':
            if (wptr->esc_default0) wptr->esc_value0 = 1;
            if (wptr->esc_value0 > wptr->cursor_x)
                wptr->esc_value0 = wptr->cursor_x;
            text_set_cursor_pos(wptr->cursor_x - wptr->esc_value0,
                                wptr->cursor_y,
                                window);
            break;
        case 'H':
            wptr->esc_value0 -= 1;
            wptr->esc_value1 -= 1;
            if (wptr->esc_default0) wptr->esc_value0 = 0;
            if (wptr->esc_default1) wptr->esc_value1 = 0;
            if (wptr->esc_value1 >= TTY_COLS)
                wptr->esc_value1 = TTY_COLS - 1;
            if (wptr->esc_value0 >= TTY_ROWS)
                wptr->esc_value0 = TTY_ROWS - 1;
            text_set_cursor_pos(wptr->esc_value1, wptr->esc_value0, window);
            break;
        case 'm':
            sgr(window);
            break;
        case 'J':
            switch (wptr->esc_value0) {
                case 2:
                    text_clear_no_move(window);
                    break;
                default:
                    break;
            }
            break;
        /* non-standard sequences */
        case 'r': /* enter/exit raw mode */
            wptr->raw = !wptr->raw;
            break;
        case 'b': /* enter/exit non-blocking mode */
            wptr->noblock = !wptr->noblock;
            break;
        case 's': /* enter/exit non-scrolling mode */
            wptr->noscroll = !wptr->noscroll;
            break;
        /* end non-standard sequences */
        default:
            wptr->escape = 0;
            text_putchar('?', window);
            break;
    }

    wptr->esc_value = &wptr->esc_value0;
    wptr->esc_value0 = 0;
    wptr->esc_value1 = 0;
    wptr->esc_default = &wptr->esc_default0;
    wptr->esc_default0 = 1;
    wptr->esc_default1 = 1;
    wptr->escape = 0;

    return;
}
