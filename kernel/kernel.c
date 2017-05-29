#include <stdint.h>
#include <kernel.h>

void kernel_shell(void);

void kernel_init(uint8_t boot_drive) {
    char buf[16];

    text_clear();

    text_putstring("echidnaOS\n\n");
    
    // clear keyboard buffer
    keyboard_init();

    // setup PIC
    map_PIC(0x20, 0x28);

    // enable desc tables
    load_GDT();
    create_IDT();
    enable_ints();

    // detect memory
    memory_size = memory_bottom = detect_mem();
    
    // use 80x50 text mode
    vga_80_x_50();
    
    // disable VGA cursor
    vga_disable_cursor();

    // pass control to the shell
	kernel_shell();
}
