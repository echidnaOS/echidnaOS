#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <graphics.h>
#include <cio.h>

vbe_info_struct_t vbe_info_struct;
edid_info_struct_t edid_info_struct;

int edid_width = 0;
int edid_height = 0;

void get_vbe_info(vbe_info_struct_t *vbe_info_struct);
void get_edid_info(edid_info_struct_t *edid_info_struct);

void init_graphics(void) {
    kprint(KPRN_INFO, "Initialising VBE...");

    DISABLE_INTERRUPTS;
    get_vbe_info(&vbe_info_struct);
    get_edid_info(&edid_info_struct);
    ENABLE_INTERRUPTS;

    edid_width = (int)edid_info_struct.det_timing_desc1[2];
    edid_width += ((int)edid_info_struct.det_timing_desc1[4] & 0xf0) << 4;
    edid_height = (int)edid_info_struct.det_timing_desc1[5];
    edid_height += ((int)edid_info_struct.det_timing_desc1[7] & 0xf0) << 4;

    if (!edid_width || !edid_height) {
        kprint(KPRN_WARN, "EDID returned 0, defaulting to 800x600");
        edid_width = 800;
        edid_height = 600;
    }

    kprint(KPRN_INFO, "Version: %u.%u", vbe_info_struct.version_maj, vbe_info_struct.version_min);
    kprint(KPRN_INFO, "OEM: %s", vbe_info_struct.oem);
    kprint(KPRN_INFO, "Graphics vendor: %s", vbe_info_struct.vendor);
    kprint(KPRN_INFO, "Product name: %s", vbe_info_struct.prod_name);
    kprint(KPRN_INFO, "Product revision: %s", vbe_info_struct.prod_rev);

    kprint(KPRN_INFO, "EDID recommended res: %ux%u", edid_width, edid_height);

    kprint(KPRN_INFO, "VBE init done.");
    return;
}
