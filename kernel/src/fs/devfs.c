#include <stdint.h>
#include <kernel.h>

#define FAILURE -2
#define SUCCESS 0

typedef struct {
    int free;
    int processes;
    char path[1024];
    int flags;
    int mode;
    long ptr;
    long begin;
    long end;
    int isblock;
    int device;
} devfs_handle_t;

devfs_handle_t* devfs_handles = (devfs_handle_t*)0;
int devfs_handles_ptr = 0;

int devfs_create_handle(devfs_handle_t handle) {
    int handle_n;

    // check for a free handle first
    for (int i = 0; i < devfs_handles_ptr; i++) {
        if (devfs_handles[i].free) {
            handle_n = i;
            goto load_handle;
        }
    }

    devfs_handles = krealloc(devfs_handles, (devfs_handles_ptr + 1) * sizeof(devfs_handle_t));
    handle_n = devfs_handles_ptr++;
    
load_handle:
    devfs_handles[handle_n] = handle;
    
    return handle_n;

}

int devfs_list(char* path, vfs_metadata_t* metadata, uint32_t entry, char* dev) {
    if (entry >= device_ptr) return FAILURE;
    kstrcpy(metadata->filename, device_list[entry].name);
    metadata->filetype = DEVICE_TYPE;
    metadata->size = device_list[entry].size;
    return SUCCESS;
}

int devfs_write(char* path, uint8_t val, uint64_t loc, char* dev) {
    if (*path == '/') path++;
    for (int i = 0; i < device_ptr; i++) {
        if (!kstrcmp(path, device_list[i].name))
            return (int)(*device_list[i].io_wrapper)(device_list[i].gp_value, loc, 1, val);
    }
    return FAILURE;
}

int devfs_read(char* path, uint64_t loc, char* dev) {
    if (*path == '/') path++;
    for (int i = 0; i < device_ptr; i++) {
        if (!kstrcmp(path, device_list[i].name))
            return (int)(*device_list[i].io_wrapper)(device_list[i].gp_value, loc, 0, 0);
    }
    return FAILURE;
}

int devfs_remove(char* path, char* dev) { return FAILURE; }
int devfs_mkdir(char* path, uint16_t perms, char* dev) { return FAILURE; }
int devfs_create(char* path, uint16_t perms, char* dev) { return FAILURE; }

int devfs_uread(int handle, char* ptr, int len) { return -1; }
int devfs_uwrite(int handle, char* ptr, int len) { return -1; }

int devfs_get_metadata(char* path, vfs_metadata_t* metadata, int type, char* dev) {
    if (type == DIRECTORY_TYPE) {
        if (!kstrcmp(path, "/") || !*path) {
            metadata->filetype = type;
            metadata->size = 0;
            kstrcpy(metadata->filename, "/");
            return SUCCESS;
        }
        else return FAILURE;
    }
    
    if (type == DEVICE_TYPE) {
        if (*path == '/') path++;
        for (int i = 0; i < device_ptr; i++) {
            if (!kstrcmp(path, device_list[i].name)) {
                kstrcpy(metadata->filename, device_list[i].name);
                metadata->filetype = DEVICE_TYPE;
                metadata->size = device_list[i].size;
                return SUCCESS;
            }
        }
        return FAILURE;
    }
    
    if (type == FILE_TYPE)
        return FAILURE;
}

int devfs_mount(char* device) { return 0; }

int devfs_open(char* path, int flags, int mode, char* dev) {
    vfs_metadata_t metadata;

    if (devfs_get_metadata(path, &metadata, DEVICE_TYPE, dev) != -2) {
        if (flags & O_TRUNC)
            return -1;
        if (flags & O_APPEND)
            return -1;
        if (flags & O_CREAT)
            return -1;
        devfs_handle_t new_handle = {0};
        new_handle.free = 0;
        new_handle.processes = 1;
        kstrcpy(new_handle.path, path);
        new_handle.flags = flags;
        new_handle.mode = mode;
        new_handle.end = metadata.size;
        if (!metadata.size)
            new_handle.isblock = 1;
        new_handle.ptr = 0;
        new_handle.begin = 0;
        if (*path == '/') path++;
        int device;
        for (device = 0; device < device_ptr; device++)
            if (!kstrcmp(path, device_list[device].name)) break;
        new_handle.device = device;
        return devfs_create_handle(new_handle);
    } else
        return -1;

}

int devfs_fork(int handle) {

    devfs_handles[handle].processes++;
    return SUCCESS;

}

void install_devfs(void) {
    vfs_install_fs("devfs", &devfs_read, &devfs_write, &devfs_remove, &devfs_mkdir,
                            &devfs_create, &devfs_get_metadata, &devfs_list, &devfs_mount,
                            &devfs_open, &devfs_fork, &devfs_uread, &devfs_uwrite );
}
