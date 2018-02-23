#include <kernel.h>
#include <klib.h>
#include <tty.h>
#include <cio.h>

void panic(const char *msg, int code) {
    DISABLE_INTERRUPTS;
    kprint(KPRN_ERR, "!!! KERNEL PANIC !!!");
    kprint(KPRN_ERR, "Panic info: %s", msg);
    kprint(KPRN_ERR, "Error code: %x", (size_t)code);
    kprint(KPRN_ERR, "SYSTEM HALTED");
    SYSTEM_HALT;
}
