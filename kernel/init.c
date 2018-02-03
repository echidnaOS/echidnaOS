#include <stdint.h>
#include <kernel.h>
#include <paging.h>
#include <klib.h>
#include <task.h>
#include <vfs.h>
#include <inits.h>
#include <cio.h>
#include <tty.h>
#include <system.h>
#include <panic.h>

size_t memory_size;
extern int ts_enable;

void kernel_init(void) {

    init_paging();

    #ifdef _SERIAL_KERNEL_OUTPUT_
      debug_kernel_console_init();
    #endif

    // setup the PIC's mask
    set_PIC0_mask(0b11111111); // disable all IRQs
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
    
    // increase speed of the PIT
    set_pit_freq(KRNL_PIT_FREQ);
    
    task_init();
    
    // print intro to tty0
    kputs("Welcome to echidnaOS!\n");
    
    kputs("\n"); kprn_ui(memory_size); kputs(" bytes ("); kprn_ui(memory_size / 0x100000); kputs(" MiB) of memory detected.\n");
    
    kputs("\nInitialising drivers...");
    // ******* DRIVER INITIALISATION CALLS GO HERE *******
    init_streams();
    init_initramfs();
    init_tty_drv();
    init_com();
    init_stty();
    init_pcspk();
    init_graphics();


    // ******* END OF DRIVER INITIALISATION CALLS *******
    
    kputs("\nInitialising file systems...");
    // ******* FILE SYSTEM INSTALLATION CALLS *******
    install_devfs();
    install_echfs();
    
    
    // ******* END OF FILE SYSTEM INSTALLATION CALLS *******
    
    
    // END OF EARLY BOOTSTRAP
    
    // setup the PIC's mask
    ts_enable = 0;
    set_PIC0_mask(0b11111100); // disable all IRQs but timer and keyboard
    set_PIC1_mask(0b11111111);
    
    ENABLE_INTERRUPTS;

    static char *env[] = { (char *)0 };
    static char *argv[] = { "/sys/init", (char *)0 };

    if (vfs_mount("/", ":://initramfs", "echfs") == -2)
        panic("Unable to mount initramfs on /");
    if (vfs_mount("/dev", "devfs", "devfs") == -2)
        panic("Unable to mount devfs on /dev");

    // launch the shell
    kputs("\nKERNEL INIT DONE!\n");

    if (kexec("/sys/init", argv, env, "/dev/tty0", "/dev/tty0", "/dev/tty0", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/tty1", "/dev/tty1", "/dev/tty1", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/tty2", "/dev/tty2", "/dev/tty2", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/tty3", "/dev/tty3", "/dev/tty3", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/tty4", "/dev/tty4", "/dev/tty4", "/") == -1)
        panic("Unable to start /sys/init");

    // wait for task scheduler
    ts_enable = 1;
    ENTER_IDLE;

}
