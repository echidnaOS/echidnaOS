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

uint32_t *framebuffer;

int edid_width = 0;
int edid_height = 0;

uint16_t vid_modes[1024];

void get_vbe_info(vbe_info_struct_t *vbe_info_struct);
void get_edid_info(edid_info_struct_t *edid_info_struct);
void get_vbe_mode_info(get_vbe_t *get_vbe);
void set_vbe_mode(uint16_t mode);
void dump_vga_font(uint8_t *bitmap);

uint8_t vga_font[4096];

void plot_px(int x, int y, uint32_t hex, uint8_t which_tty) {
    size_t fb_i = x + edid_width * y;

    if (current_tty == which_tty)
        framebuffer[fb_i] = hex;

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
