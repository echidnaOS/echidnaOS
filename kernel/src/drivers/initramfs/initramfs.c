#include <stdint.h>
#include <kernel.h>
#include <klib.h>

#define SUCCESS 0
#define EOF -1
#define FAILURE -2

int initramfs_io_wrapper(uint32_t dev, uint64_t loc, int type, uint8_t payload) {
    if (loc >= INITRAMFS_SIZE)
        return EOF;
    if (type == 0) {
        return (int)(*((uint8_t*)(INITRAMFS_BASE + (uint32_t)loc)));
    }
    else if (type == 1) {
        *((uint8_t*)(INITRAMFS_BASE + (uint32_t)loc)) = payload;
        return SUCCESS;
    }
}

void init_initramfs(void) {

    kputs("\nInitialising initramfs driver...");

    kernel_add_device("initramfs", 0, INITRAMFS_SIZE, &initramfs_io_wrapper);

    kputs("\nInitialised initramfs.");

}
