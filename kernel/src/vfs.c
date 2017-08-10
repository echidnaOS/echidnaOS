#include <stdint.h>
#include <kernel.h>

#define FAILURE -2
#define SUCCESS 0
#define EOF -1

mountpoint_t* mountpoints;
filesystem_t* filesystems;
int mountpoints_ptr = 0;
int filesystems_ptr = 0;

int vfs_translate_mnt(char* path, char** local_path) {
    int guess = FAILURE;
    int guess_size = 0;
    for (int i = 0; i < mountpoints_ptr; i++)
        if (!kstrncmp(path, mountpoints[i].mountpoint, kstrlen(mountpoints[i].mountpoint)))
            if (kstrlen(mountpoints[i].mountpoint) > guess_size) {
                guess = i;
                guess_size = kstrlen(mountpoints[i].mountpoint);
            }
    *local_path = path + guess_size;
    return guess;
}

int vfs_translate_fs(int mountpoint) {
    for (int i = 0; i < filesystems_ptr; i++)
        if (!kstrcmp(filesystems[i].name, mountpoints[mountpoint].filesystem))
            return i;
    return FAILURE;
}

/*
int vfs_read(char* path, uint64_t loc) {
    char* local_path;
    path += task_table[current_task]->base;

    int mountpoint = vfs_translate_mnt(path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].read)(local_path, loc, dev);
}
*/

int vfs_list(char* path, vfs_metadata_t* metadata, uint32_t entry) {
    char* local_path;
    path += task_table[current_task]->base;

    int mountpoint = vfs_translate_mnt(path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].list)(local_path, metadata, entry, mountpoints[mountpoint].device);
}

int vfs_mount(char* mountpoint, char* device, char* filesystem) {
    mountpoints = krealloc(mountpoints, sizeof(mountpoint_t) * (mountpoints_ptr+1));
    
    kstrcpy(mountpoints[mountpoints_ptr].mountpoint, mountpoint + task_table[current_task]->base);
    kstrcpy(mountpoints[mountpoints_ptr].device, device + task_table[current_task]->base);
    kstrcpy(mountpoints[mountpoints_ptr].filesystem, filesystem + task_table[current_task]->base);
    
    kputs("\nMounted `"); kputs(mountpoints[mountpoints_ptr].device);
    kputs("' on `"); kputs(mountpoints[mountpoints_ptr].mountpoint);
    kputs("' using filesystem: "); kputs(mountpoints[mountpoints_ptr].filesystem);
    mountpoints_ptr++;
    return SUCCESS;
}

void vfs_install_fs(char* name,
                    int (*read)(char* path, uint64_t loc, char* dev),
                    int (*write)(char* path, uint8_t val, uint64_t loc, char* dev),
                    int (*get_metadata)(char* path, vfs_metadata_t* metadata, char* dev),
                    int (*list)(char* path, vfs_metadata_t* metadata, uint32_t entry, char* dev) ) {
    
    filesystems = krealloc(filesystems, sizeof(filesystem_t) * (filesystems_ptr+1));
    
    kstrcpy(filesystems[filesystems_ptr].name, name);
    filesystems[filesystems_ptr].read = read;
    filesystems[filesystems_ptr].write = write;
    filesystems[filesystems_ptr].get_metadata = get_metadata;
    filesystems[filesystems_ptr].list = list;
    
    filesystems_ptr++;
    return;
}
