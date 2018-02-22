#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <paging.h>
#include <dev.h>

#define SUCCESS 0
#define EOF -1

extern unsigned char initramfs[];
extern void *initramfs_len;

int initramfs_io_wrapper(uint32_t unused0, uint64_t loc, int type, uint8_t payload) {
    if (loc >= (uintptr_t)&initramfs_len)
        return EOF;
    if (type == 0) {
        return (int)(initramfs[loc]);
    }
    else if (type == 1) {
        initramfs[loc] = (unsigned char)payload;
        return SUCCESS;
    }
}

void init_initramfs(void) {

    kprint(KPRN_INFO, "Initialising initramfs driver...");

    kernel_add_device("initramfs", 0, (uintptr_t)&initramfs_len, &initramfs_io_wrapper);

    kprint(KPRN_INFO, "Initialised initramfs.");

}
