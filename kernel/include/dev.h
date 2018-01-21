#ifndef __DEV_H__
#define __DEV_H__

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char name[32];
    uint32_t gp_value;
    uint64_t size;
    int (*io_wrapper)(uint32_t, uint64_t, int, uint8_t);
} device_t;

extern device_t *device_list;
extern size_t device_ptr;


void kernel_add_device(char *name, uint32_t gp_value, uint64_t size,
                       int (*io_wrapper)(uint32_t, uint64_t, int, uint8_t));




#endif
