#include <stdlib.h>
#include <stdint.h>
#include <kernel.h>
#include "shell/shell.h"

#include "globalvars.h"

void done_msg(void);

void kernel_main(void) {
    char buf[16];

    text_clear();

    text_putstring("echidnaOS\n\n");
    
    // initialise keyboard
    keyboard_init();

    map_PIC(0x20, 0x28);

    load_GDT();
    create_IDT();
    enable_ints();

    memory_size = detect_mem();
    text_putstring(itoa(memory_size, buf, 10));
    text_putstring(" bytes of memory detected.\n\n");
    
    // use 80x50 text mode
    vga_80_x_50();
    
    // disable VGA cursor
    vga_disable_cursor();

    // pass control to the shell
	kernel_shell();
}
