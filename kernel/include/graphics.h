#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <stdint.h>

typedef struct {
    uint8_t version_min;
    uint8_t version_maj;
    char* oem;
    uint32_t capabilities;
    uint16_t* vid_modes;
    uint16_t vid_mem_blocks;
    uint16_t software_rev;
    char* vendor;
    char* prod_name;
    char* prod_rev;
} __attribute__((packed)) vbe_info_struct_t;






#endif
