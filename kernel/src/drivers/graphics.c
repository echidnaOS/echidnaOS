#include <stdint.h>
#include <kernel.h>

vbe_info_struct_t vbe_info_struct;

void init_graphics(void) {
    kputs("\nInitialising VBE...");
    graphics_init(&vbe_info_struct);

    kputs("\nVersion: "); kuitoa(vbe_info_struct.version_maj);
    kputs("."); kuitoa(vbe_info_struct.version_min);
    kputs("\nOEM: "); kputs(vbe_info_struct.oem);
    kputs("\nGraphics vendor: "); kputs(vbe_info_struct.vendor);
    kputs("\nProduct name: "); kputs(vbe_info_struct.prod_name);
    kputs("\nProduct revision: "); kputs(vbe_info_struct.prod_rev);

    kputs("\nVBE init done.");
    return;
}
