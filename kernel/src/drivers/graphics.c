#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <graphics.h>

vbe_info_struct_t vbe_info_struct;

void graphics_init(vbe_info_struct_t *vbe_info_struct);

void init_graphics(void) {
    kprint(KPRN_INFO, "Initialising VBE...");
    graphics_init(&vbe_info_struct);

    kprint(KPRN_INFO, "Version: %u.%u", vbe_info_struct.version_maj, vbe_info_struct.version_min);
    kprint(KPRN_INFO, "OEM: %s", vbe_info_struct.oem);
    kprint(KPRN_INFO, "Graphics vendor: %s", vbe_info_struct.vendor);
    kprint(KPRN_INFO, "Product name: %s", vbe_info_struct.prod_name);
    kprint(KPRN_INFO, "Product revision: %s", vbe_info_struct.prod_rev);

    kprint(KPRN_INFO, "VBE init done.");
    return;
}
