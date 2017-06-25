#include <stdint.h>
#include <kernel.h>

void kernel_shell(void);

void kernel_init(uint8_t boot_drive) {
    task_t empty_task = {0};
    task_t* krnl_task = (task_t*)0x1000000;

    // setup PIC
    map_PIC(0x20, 0x28);
    
    // enable desc tables
    load_GDT();
    load_IDT();
    load_TSS();
    
    // initialise keyboard driver
    keyboard_init();
    
    // use 80x50 text mode
    vga_80_x_50();
    
    // disable VGA cursor
    vga_disable_cursor();
    
    init_textdrv();

    // detect memory
    memory_size = detect_mem();
    
    // increase speed of the PIT
    set_pit_freq(0x8000);
    
    // create dummy kernel task
    memory_bottom += sizeof(task_t);
    *krnl_task = empty_task;
    krnl_task->status = 0x12121212;
    
    current_tty = 1;
    tty_refresh_force();
    
    // print intro to tty0
    kputs("Welcome to echidnaOS!\n");
    
    kputs("\n"); kuitoa(memory_size); kputs(" bytes ("); kuitoa(memory_size / 0x100000); kputs(" MiB) of memory detected.");
    
    init_disk(boot_drive);

    // launch the shells
    kernel_shell();
	
    // start scheduler
    task_switch(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
}
