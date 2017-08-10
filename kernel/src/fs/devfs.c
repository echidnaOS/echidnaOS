#include <stdint.h>
#include <kernel.h>

#define FAILURE -2
#define SUCCESS 0

int devfs_list(char* path, vfs_metadata_t* metadata, uint32_t entry, char* dev) {
    if (entry >= device_ptr) return FAILURE;
    kstrcpy(metadata->filename + task_table[current_task]->base, device_list[entry].name);
    return SUCCESS;
}

int devfs_write(char* path, uint8_t val, uint64_t loc, char* dev) { return 0; }
int devfs_read(char* path, uint64_t loc, char* dev) { return 0; }
int devfs_get_metadata(char* path, vfs_metadata_t* metadata, char* dev) { return 0; }

void install_devfs(void) {
    vfs_install_fs("devfs", &devfs_read, &devfs_write, &devfs_get_metadata, &devfs_list);
}
