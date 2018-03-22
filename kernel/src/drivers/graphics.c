#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <graphics.h>
#include <cio.h>
#include <tty.h>
#include <panic.h>

vbe_info_struct_t vbe_info_struct;
edid_info_struct_t edid_info_struct;
vbe_mode_info_t vbe_mode_info;
get_vbe_t get_vbe;

int modeset_done = 0;
int gui_needs_refresh = 0;

int current_window;

uint32_t *framebuffer;
uint32_t *antibuffer;

int edid_width = 0;
int edid_height = 0;

uint16_t vid_modes[1024];

void get_vbe_info(vbe_info_struct_t *vbe_info_struct);
void get_edid_info(edid_info_struct_t *edid_info_struct);
void get_vbe_mode_info(get_vbe_t *get_vbe);
void set_vbe_mode(uint16_t mode);
void dump_vga_font(uint8_t *bitmap);

uint8_t vga_font[4096];

window_t *windows = 0;

void plot_px(int x, int y, uint32_t hex) {
    if (x > edid_width || y > edid_height || x < 0 || y < 0)
        return;

    size_t fb_i = x + edid_width * y;

    antibuffer[fb_i] = hex;
}

window_t *get_window_ptr(int id) {
    if (!windows) {
        return (window_t *)0;
    } else {
        window_t *wptr = windows;
        for (;;) {
            if (wptr->id == id) {
                return wptr;
            }
            if (!wptr->next) {
                /* not found */
                return (window_t *)0;
            } else {
                wptr = wptr->next;
            }
        }
    }
}

/* creates a new window with a title, size */
/* returns window id */
int create_window(char *title, size_t x, size_t y, size_t x_size, size_t y_size) {
    window_t *wptr;
    int id = 0;

    /* check if no windows were allocated */
    if (!windows) {
        /* allocate root window */
        windows = kalloc(sizeof(window_t));
        wptr = windows;
    } else {
        /* else crawl the linked list to the last entry */
        wptr = windows;
        for (;;) {
            if (wptr->id == id)
                id++;
            if (wptr->next) {
                wptr = wptr->next;
                continue;
            } else {
                wptr->next = kalloc(sizeof(window_t));
                wptr = wptr->next;
                break;
            }
        }
    }

    wptr->id = id;
    kstrcpy(wptr->title, title);
    wptr->x = x;
    wptr->y = y;
    wptr->x_size = x_size;
    wptr->y_size = y_size;
    wptr->framebuffer = kalloc(x_size * y_size * sizeof(uint32_t));
    wptr->grid = kalloc((x_size / 8) * (y_size / 16));
    wptr->gridbg = kalloc((x_size / 8) * (y_size / 16) * sizeof(uint32_t));
    wptr->gridfg = kalloc((x_size / 8) * (y_size / 16) * sizeof(uint32_t));
    wptr->esc_value = &wptr->esc_value0;
    wptr->esc_value0 = 0;
    wptr->esc_value1 = 0;
    wptr->esc_default = &wptr->esc_default0;
    wptr->esc_default0 = 1;
    wptr->esc_default1 = 1;
    wptr->escape = 0;
    wptr->cursor_x = 0;
    wptr->cursor_y = 0;
    wptr->cursor_status = 1;
    wptr->cursor_bg_col = TTY_DEF_CUR_BG_COL;
    wptr->cursor_fg_col = TTY_DEF_CUR_FG_COL;
    wptr->text_bg_col = TTY_DEF_TXT_BG_COL;
    wptr->text_fg_col = TTY_DEF_TXT_FG_COL;
    wptr->raw = 0;
    wptr->noblock = 0;
    wptr->noscroll = 0;
    wptr->next = 0;

    current_window = id;

    gui_needs_refresh = 1;

    return id;
}

void window_focus(int window) {
    /* moves the requested window to the foreground */
    window_t *last_wptr;
    window_t *req_wptr = get_window_ptr(window);
    window_t *prev_wptr;

    if (!windows)
        return;

    if (!req_wptr)
        return;

    window_t *next_wptr = req_wptr->next;

    if (!(req_wptr == windows))
        for (prev_wptr = windows; prev_wptr->next != req_wptr; prev_wptr = prev_wptr->next);
    else
        prev_wptr = 0;

    for (last_wptr = windows; last_wptr->next; last_wptr = last_wptr->next);

    if (last_wptr == req_wptr)
        return;

    /* all necessary variables acquired */

    /* the prev should point to the next */
    if (prev_wptr)
        prev_wptr->next = next_wptr;
    else
        windows = next_wptr;
    /* the requested one should point to NULL */
    req_wptr->next = 0;
    /* the last should point to the requested one */
    last_wptr->next = req_wptr;

    current_window = window;

    gui_needs_refresh = 1;

    return;
}

void window_move(int x, int y, int window) {
    window_t *wptr = get_window_ptr(window);

    wptr->x += x;
    wptr->y += y;

    gui_needs_refresh = 1;

    return;
}

#define BACKGROUND_COLOUR       0x00008888
#define WINDOW_BORDERS          0x00ffffff
#define TITLE_BAR_BACKG         0x00003377
#define TITLE_BAR_FOREG         0x00ffffff
#define TITLE_BAR_THICKNESS     18

void gui_refresh(void) {
    /* clear screen */
    for (size_t i = 0; i < edid_width * edid_height; i++)
        antibuffer[i] = BACKGROUND_COLOUR;

    /* draw every window */
    window_t *wptr = windows;
    for (;;) {
        if (!wptr)
            break;

        /* draw the title bar */
        for (size_t x = 0; x < TITLE_BAR_THICKNESS; x++)
            for (size_t i = 0; i < wptr->x_size + 2; i++)
                plot_px(wptr->x + i, wptr->y + x, TITLE_BAR_BACKG);

        /* draw the title */
        for (size_t i = 0; wptr->title[i]; i++)
            plot_char(wptr->title[i], wptr->x + 8 + i * 8, wptr->y + 1,
                TITLE_BAR_FOREG, TITLE_BAR_BACKG);

        /* draw the window border */
        for (size_t i = 0; i < wptr->x_size + 2; i++)
            plot_px(wptr->x + i, wptr->y, WINDOW_BORDERS);
        for (size_t i = 0; i < wptr->x_size + 2; i++)
            plot_px(wptr->x + i, wptr->y + TITLE_BAR_THICKNESS + wptr->y_size, WINDOW_BORDERS);
        for (size_t i = 0; i < wptr->y_size + TITLE_BAR_THICKNESS + 1; i++)
            plot_px(wptr->x, wptr->y + i, WINDOW_BORDERS);
        for (size_t i = 0; i < wptr->y_size + TITLE_BAR_THICKNESS + 1; i++)
            plot_px(wptr->x + wptr->x_size + 1, wptr->y + i, WINDOW_BORDERS);

        /* paint the framebuffer */
        size_t in_x = 1;
        size_t in_y = TITLE_BAR_THICKNESS;
        for (size_t i = 0; i < wptr->x_size * wptr->y_size; i++) {
            plot_px(in_x++ + wptr->x, in_y + wptr->y, wptr->framebuffer[i]);
            if (in_x - 1 == wptr->x_size) {
                in_y++;
                in_x = 1;
            }
        }

        wptr = wptr->next;
    }

    /* copy over the buffer */
    for (size_t i = 0; i < edid_width * edid_height; i++)
        framebuffer[i] = antibuffer[i];

    return;
}

void plot_px_window(int x, int y, uint32_t hex, int window) {
    window_t *wptr = get_window_ptr(window);
    size_t fb_i = x + wptr->x_size * y;
    wptr->framebuffer[fb_i] = hex;
    gui_needs_refresh = 1;
    return;
}

void init_graphics(void) {
    /* interrupts are supposed to be OFF */

    kprint(KPRN_INFO, "Dumping VGA font...");

    dump_vga_font(vga_font);

    kprint(KPRN_INFO, "Initialising VBE...");

    get_vbe_info(&vbe_info_struct);
    /* copy the video mode array somewhere else because it might get overwritten */
    for (size_t i = 0; ; i++) {
        vid_modes[i] = ((uint16_t *)vbe_info_struct.vid_modes)[i];
        if (((uint16_t *)vbe_info_struct.vid_modes)[i+1] == 0xffff) {
            vid_modes[i+1] = 0xffff;
            break;
        }
    }

    kprint(KPRN_INFO, "Version: %u.%u", vbe_info_struct.version_maj, vbe_info_struct.version_min);
    kprint(KPRN_INFO, "OEM: %s", (char *)vbe_info_struct.oem);
    kprint(KPRN_INFO, "Graphics vendor: %s", (char *)vbe_info_struct.vendor);
    kprint(KPRN_INFO, "Product name: %s", (char *)vbe_info_struct.prod_name);
    kprint(KPRN_INFO, "Product revision: %s", (char *)vbe_info_struct.prod_rev);

    kprint(KPRN_INFO, "Calling EDID...");

    get_edid_info(&edid_info_struct);

    edid_width = (int)edid_info_struct.det_timing_desc1[2];
    edid_width += ((int)edid_info_struct.det_timing_desc1[4] & 0xf0) << 4;
    edid_height = (int)edid_info_struct.det_timing_desc1[5];
    edid_height += ((int)edid_info_struct.det_timing_desc1[7] & 0xf0) << 4;

    kprint(KPRN_INFO, "EDID recommended res: %ux%u", edid_width, edid_height);

    //if (!edid_width || !edid_height) {
        kprint(KPRN_WARN, "EDID returned 0, defaulting to 1024x768");
        edid_width = 1024;
        edid_height = 768;
    //}

retry:
    /* try to set the mode */
    get_vbe.vbe_mode_info = (uint32_t)&vbe_mode_info;
    for (size_t i = 0; vid_modes[i] != 0xffff; i++) {
        get_vbe.mode = vid_modes[i];
        get_vbe_mode_info(&get_vbe);
        if (vbe_mode_info.res_x == edid_width
            && vbe_mode_info.res_y == edid_height
            && vbe_mode_info.bpp == 32) {
            /* mode found */
            kprint(KPRN_INFO, "VBE found matching mode %x, attempting to set.", get_vbe.mode);
            framebuffer = (uint32_t *)vbe_mode_info.framebuffer;
            antibuffer = kalloc(edid_width * edid_height * sizeof(uint32_t));
            kprint(KPRN_INFO, "Framebuffer address: %x", vbe_mode_info.framebuffer);
            set_vbe_mode(get_vbe.mode);
            goto success;
        }
    }

    if (edid_width > 1024 || edid_height > 768) {
        kprint(KPRN_WARN, "EDID modesetting failed, defaulting to 1024x768.");
        edid_width = 1024;
        edid_height = 768;
        goto retry;
    }

    if (edid_width == 1024 && edid_height == 768) {
        kprint(KPRN_WARN, "1024x768 modesetting failed, defaulting to 800x600.");
        edid_width = 800;
        edid_height = 600;
        goto retry;
    }

    if (edid_width == 800 && edid_height == 600) {
        kprint(KPRN_WARN, "800x600 modesetting failed, defaulting to 640x480.");
        edid_width = 640;
        edid_height = 480;
        goto retry;
    }

    panic("VBE: can't set video mode.", 0);

success:
    modeset_done = 1;
    kprint(KPRN_INFO, "VBE init done.");
    return;
}
