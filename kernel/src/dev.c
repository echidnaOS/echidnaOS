#include <stdint.h>
#include <kernel.h>

device_t* device_list;
uint32_t device_ptr = 0;

void kernel_add_device(char* name, uint32_t gp_value, uint64_t size,
                       int (*io_wrapper)(uint32_t, uint64_t, int, uint8_t)) {
    device_list = krealloc(device_list, (device_ptr + 1) * sizeof(device_t));
    if (!device_list) panic("Unable to add device");
    kstrcpy(device_list[device_ptr].name, name);
    device_list[device_ptr].gp_value = gp_value;
    device_list[device_ptr].io_wrapper = io_wrapper;
    device_list[device_ptr].size = size;
    device_ptr++;
    return;
}
