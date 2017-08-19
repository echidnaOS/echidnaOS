#include <stdint.h>
#include <kernel.h>

uint32_t memory_size;

void init_tasks(void);

void kernel_init(void) {
    task_t empty_task = {0};

    // setup the PIC's mask
    set_PIC0_mask(0b11111100); // disable all IRQs but timer and keyboard
    set_PIC1_mask(0b11111111);

    // setup PIC
    map_PIC(0x20, 0x28);
    
    // enable desc tables
    load_GDT();
    load_IDT();
    load_TSS();
    
    // initialise keyboard driver
    keyboard_init();
    
    #ifndef _BIG_FONTS_
      vga_80_x_50();
    #endif
    
    // disable VGA cursor
    vga_disable_cursor();
    
    init_tty();
    switch_tty(0);

    // detect memory
    memory_size = detect_mem();
    init_kalloc();
    
    // increase speed of the PIT
    set_pit_freq(KRNL_PIT_FREQ);
    
    task_init();
    
    // print intro to tty0
    kputs("Welcome to echidnaOS!\n");
    
    kputs("\n"); kuitoa(memory_size); kputs(" bytes ("); kuitoa(memory_size / 0x100000); kputs(" MiB) of memory detected.\n");
    
    kputs("\nInitialising drivers...");
    // ******* DRIVER INITIALISATION CALLS GO HERE *******
    init_streams();
    init_tty_drv();
    init_ata();
    init_com();
    init_stty();
    init_pcspk();


    // ******* END OF DRIVER INITIALISATION CALLS *******
    
    kputs("\nInitialising file systems...");
    // ******* FILE SYSTEM INSTALLATION CALLS *******
    install_devfs();
    install_echfs();
    
    
    // ******* END OF FILE SYSTEM INSTALLATION CALLS *******

    // launch the tasks
    init_tasks();
    
    vfs_mount("/", ":://hda", "echfs");
    vfs_mount("/dev", "devfs", "devfs");
	
    // start scheduler
    task_scheduler();
}
