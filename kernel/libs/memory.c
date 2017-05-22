#include <kernel.h>
#include <stdint.h>

void* kmalloc(uint32_t size) {
    memory_bottom -= size;
    return (void*) memory_bottom;
}
