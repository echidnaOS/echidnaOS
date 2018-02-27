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
#include <graphics.h>
#include <smp.h>

size_t memory_size;

void kernel_init(void) {
    /* interrupts disabled */

    /* mask all PIC IRQs */
    kprint(KPRN_INFO, "PIC: Masking and remapping the legacy PICs...");
    set_PIC0_mask(0b11111111);
    set_PIC1_mask(0b11111111);
    /* remap PIC where it doesn't bother us */
    map_PIC(0xa0, 0xa8);
    kprint(KPRN_INFO, "PIC: PIC 0 and 1 disabled.");

    /* build descriptor tables */
    load_IDT();

    /* detect memory */
    memory_size = detect_mem();

    /* initialise paging */
    init_paging();

    #ifdef _SERIAL_KERNEL_OUTPUT_
      /* initialise kernel serial debug console */
      debug_kernel_console_init();
    #endif

    /* initialise graphics mode and TTYs */
    init_graphics();
    init_tty();

    /* set PIT frequency */
    set_pit_freq(KRNL_PIT_FREQ);

    /* disable scheduler */
    kprint(KPRN_INFO, "INIT: ts_enable = 0;");
    ts_enable = 0;

    /* initialise ACPI */
    init_acpi();

    /* initialise APIC */
    init_apic();

    /* init CPU 0 */
    init_cpu0();

    /* enable interrupts for the first time */
    kprint(KPRN_INFO, "INIT: ENABLE INTERRUPTS");
    ENABLE_INTERRUPTS;

    /****** END OF EARLY BOOTSTRAP ******/

    init_aps();

    /* initialise keyboard driver */
    keyboard_init();

    /* initialise scheduler */
    task_init();

    kprint(KPRN_INFO, "INIT: Initialising drivers...");
    /******* DRIVER INITIALISATION CALLS GO HERE *******/
    init_streams();
    init_initramfs();
    init_ata();
    init_tty_drv();
    init_fb();
    init_com();
    init_stty();
    init_pcspk();


    /******* END OF DRIVER INITIALISATION CALLS *******/

    kprint(KPRN_INFO, "INIT: Initialising file systems...");
    /******* FILE SYSTEM INSTALLATION CALLS *******/
    install_devfs();
    install_echfs();


    /******* END OF FILE SYSTEM INSTALLATION CALLS *******/

    /* mount essential filesystems */
    if (vfs_mount("/", ":://initramfs", "echfs") == -2)
        panic("Unable to mount initramfs on /", 0);
    if (vfs_mount("/dev", "devfs", "devfs") == -2)
        panic("Unable to mount devfs on /dev", 0);
    if (vfs_mount("/mnt", "/dev/hda", "echfs") == -2)
        ;

    kprint(KPRN_INFO, "INIT: Kernel initialisation complete, starting init...");

    /* launch PID 0 */
    static char *env[] = { (char *)0 };
    static char *argv[] = { "/sys/init", (char *)0 };
    if (kexec("/sys/init", argv, env, "/dev/tty0", "/dev/tty0", "/dev/tty0", "/") == -1)
        panic("Unable to start /sys/init", 0);
    if (kexec("/sys/init", argv, env, "/dev/stty0", "/dev/stty0", "/dev/stty0", "/") == -1)
        panic("Unable to start /sys/init", 0);
    if (kexec("/sys/init", argv, env, "/dev/tty1", "/dev/tty1", "/dev/tty1", "/") == -1)
        panic("Unable to start /sys/init", 0);
    if (kexec("/sys/init", argv, env, "/dev/tty2", "/dev/tty2", "/dev/tty2", "/") == -1)
        panic("Unable to start /sys/init", 0);
    if (kexec("/sys/init", argv, env, "/dev/tty3", "/dev/tty3", "/dev/tty3", "/") == -1)
        panic("Unable to start /sys/init", 0);
    if (kexec("/sys/init", argv, env, "/dev/tty4", "/dev/tty4", "/dev/tty4", "/") == -1)
        panic("Unable to start /sys/init", 0);

    /* launch scheduler for the first time */
    kprint(KPRN_INFO, "INIT: DISABLE INTERRUPTS");
    DISABLE_INTERRUPTS;
    kprint(KPRN_INFO, "INIT: ts_enable = 1;");
    ts_enable = 1;
    kprint(KPRN_INFO, "INIT: Calling scheduler...");
    task_scheduler();

}
