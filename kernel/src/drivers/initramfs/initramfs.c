#include <stdint.h>
#include <kernel.h>
#include <klib.h>
#include <paging.h>
#include <dev.h>

#define SUCCESS 0
#define EOF -1

extern unsigned char initramfs[];
extern unsigned int initramfs_len;

int initramfs_io_wrapper(uint32_t unused0, uint64_t loc, int type, uint8_t payload) {
    if (loc >= initramfs_len)
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

    kputs("\nInitialising initramfs driver...");

    kernel_add_device("initramfs", 0, initramfs_len, &initramfs_io_wrapper);

    kputs("\nInitialised initramfs.");

}
