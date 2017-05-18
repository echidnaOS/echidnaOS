#include <stdlib.h>
#include <stdint.h>
#include <kernel.h>
#include "shell/shell.h"

#include "globalvars.h"

void done_msg(void);

void kernel_main(void) {
    char buf[16];

/*
    memory_size = mem_load_d(0x7DF9);
    kernel_size = mem_load_d(0x7DF5);
    available_page = (0x1000000+kernel_size)/0x400000;
    if ((0x1000000+kernel_size)%0x400000) available_page++;
*/

    text_clear();

    text_putstring("echidnaOS\n\n");

/*
    text_putstring(itoa(memory_size, buf, 10));
    text_putstring(" bytes of memory detected.\n");
    text_putstring("The kernel is ");
    text_putstring(itoa(kernel_size, buf, 10));
    text_putstring(" bytes long.\n\n");
*/
    
    // initialise keyboard
    keyboard_init();

    // map the PIC0 at int 0x20-0x27 and PIC1 at 0x28-0x2F
    text_putstring("Initialising PIC...");
    map_PIC(0x20, 0x28);
    done_msg();

    // create the GDT and IDT
    text_putstring("Loading descriptor tables...");
    load_GDT();
    create_IDT();
    enable_ints();
    done_msg();
    
    // test real mode bullshit
    real_test();

    // pass control to the shell
	kernel_shell();
}

void done_msg(void) {
    char old_palette = text_get_text_palette();
    text_set_cursor_pos(74, text_get_cursor_pos_y());
    text_putchar('[');
    text_set_text_palette(0x02);
    text_putstring("DONE");
    text_set_text_palette(old_palette);
    text_putchar(']');
}
