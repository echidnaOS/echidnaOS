#include <stdint.h>
#include <kernel.h>

#define FAILURE -2
#define SUCCESS 0

char* device;

uint8_t rd_byte(uint64_t loc) {
    return (uint8_t)vfs_kread(device, loc);
}

uint16_t rd_word(uint64_t loc) {
    uint16_t x = 0;
    for (uint64_t i = 0; i < 2; i++)
        x += (uint16_t)vfs_kread(device, loc++) * (uint16_t)(power(0x100, i));
    return x;
}

uint32_t rd_dword(uint64_t loc) {
    uint32_t x = 0;
    for (uint64_t i = 0; i < 4; i++)
        x += (uint32_t)vfs_kread(device, loc++) * (uint32_t)(power(0x100, i));
    return x;
}

uint64_t rd_qword(uint64_t loc) {
    uint64_t x = 0;
    for (uint64_t i = 0; i < 8; i++)
        x += (uint64_t)vfs_kread(device, loc++) * (uint64_t)(power(0x100, i));
    return x;
}

int echfs_list(char* path, vfs_metadata_t* metadata, uint32_t entry, char* dev) {
    device = dev;
    uint64_t total_blocks = rd_qword(12);
    kxtoa(total_blocks);
    return FAILURE;
}

int echfs_write(char* path, uint8_t val, uint64_t loc, char* dev) { return 0; }
int echfs_read(char* path, uint64_t loc, char* dev) { return 0; }
int echfs_get_metadata(char* path, vfs_metadata_t* metadata, char* dev) { return 0; }

void install_echfs(void) {
    vfs_install_fs("echfs", &echfs_read, &echfs_write, &echfs_get_metadata, &echfs_list);
}
