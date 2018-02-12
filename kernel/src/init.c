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

size_t memory_size;

void kernel_init(void) {
    /* interrupts disabled */

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
    ts_enable = 0;

    /* initialise ACPI */
    init_acpi();

    /* initialise APIC */
    init_apic();

    /* enable interrupts for the first time */
    ENABLE_INTERRUPTS;

    /****** END OF EARLY BOOTSTRAP ******/

    /* initialise keyboard driver */
    keyboard_init();

    /* initialise scheduler */
    task_init();

    kprint(KPRN_INFO, "Initialising drivers...");

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

    kprint(KPRN_INFO, "Initialising file systems...");
    /******* FILE SYSTEM INSTALLATION CALLS *******/
    install_devfs();
    install_echfs();


    /******* END OF FILE SYSTEM INSTALLATION CALLS *******/

    /* mount essential filesystems */
    if (vfs_mount("/", ":://initramfs", "echfs") == -2)
        panic("Unable to mount initramfs on /");
    if (vfs_mount("/dev", "devfs", "devfs") == -2)
        panic("Unable to mount devfs on /dev");
    if (vfs_mount("/mnt", "/dev/hda", "echfs") == -2)
        ;

    kprint(KPRN_INFO, "Kernel initialisation complete, starting init...");

    /* launch PID 0 */
    static char *env[] = { (char *)0 };
    static char *argv[] = { "/sys/init", (char *)0 };
    if (kexec("/sys/init", argv, env, "/dev/tty0", "/dev/tty0", "/dev/tty0", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/stty0", "/dev/stty0", "/dev/stty0", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/tty1", "/dev/tty1", "/dev/tty1", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/tty2", "/dev/tty2", "/dev/tty2", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/tty3", "/dev/tty3", "/dev/tty3", "/") == -1)
        panic("Unable to start /sys/init");
    if (kexec("/sys/init", argv, env, "/dev/tty4", "/dev/tty4", "/dev/tty4", "/") == -1)
        panic("Unable to start /sys/init");

    /* launch scheduler for the first time */
    DISABLE_INTERRUPTS;
    ts_enable = 1;
    task_scheduler();

}
