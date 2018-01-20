#include <stdint.h>
#include <kernel.h>
#include <klib.h>

#define FAILURE -2
#define SUCCESS 0

typedef struct {
    int free;
    char path[1024];
    int flags;
    int mode;
    long ptr;
    long begin;
    long end;
    int isblock;
    int device;
} devfs_handle_t;

static devfs_handle_t* devfs_handles = (devfs_handle_t*)0;
static int devfs_handles_ptr = 0;

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

extern int write_stat;

int devfs_uwrite(int handle, char* ptr, int len) {
    write_stat = 0;

    int device = devfs_handles[handle].device;
    int gp_value = device_list[device].gp_value;
    int i_ptr = devfs_handles[handle].ptr;
    int (*io_wrapper)(uint32_t, uint64_t, int, uint8_t) = device_list[device].io_wrapper;
    int i;
    for (i = 0; i < len; i++) {
        int c = (*io_wrapper)(gp_value, i_ptr, 1, ptr[i]);
        switch (c) {
            case FAILURE:
            case -1:
                goto out;
            case IO_NOT_READY:
                write_stat = 1;
                goto out;
            default:
                break;
        }
        i_ptr++;
    }
out:
    devfs_handles[handle].ptr = i_ptr;
    return i;
}

int devfs_read(char* path, uint64_t loc, char* dev) {
    if (*path == '/') path++;
    for (int i = 0; i < device_ptr; i++) {
        if (!kstrcmp(path, device_list[i].name))
            return (int)(*device_list[i].io_wrapper)(device_list[i].gp_value, loc, 0, 0);
    }
    return FAILURE;
}

extern int read_stat;

int devfs_uread(int handle, char* ptr, int len) {
    read_stat = 0;

    int device = devfs_handles[handle].device;
    int gp_value = device_list[device].gp_value;
    int i_ptr = devfs_handles[handle].ptr;
    int (*io_wrapper)(uint32_t, uint64_t, int, uint8_t) = device_list[device].io_wrapper;
    int i;
    for (i = 0; i < len; i++) {
        int c = (*io_wrapper)(gp_value, i_ptr, 0, 0);
        switch (c) {
            case FAILURE:
            case -1:
                goto out;
            case IO_NOT_READY:
                read_stat = 1;
                goto out;
            default:
                break;
        }
        i_ptr++;
        ptr[i] = (char)c;
    }
out:
    devfs_handles[handle].ptr = i_ptr;
    return i;
}

int devfs_remove(char* path, char* dev) { return FAILURE; }
int devfs_mkdir(char* path, uint16_t perms, char* dev) { return FAILURE; }
int devfs_create(char* path, uint16_t perms, char* dev) { return FAILURE; }

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

int devfs_close(int handle) {

    if (handle < 0)
        return -1;
        
    if (handle >= devfs_handles_ptr)
        return -1;
    
    if (devfs_handles[handle].free)
        return -1;
    
    devfs_handles[handle].free = 1;
    
    return 0;
    
}

int devfs_seek(int handle, int offset, int type) {
    
    if (handle < 0)
        return -1;

    if (handle >= devfs_handles_ptr)
        return -1;
    
    if (devfs_handles[handle].free)
        return -1;
    
    if (devfs_handles[handle].isblock)
        return -1;
        
    switch (type) {
        case SEEK_SET:
            if ((devfs_handles[handle].begin + offset) > devfs_handles[handle].end ||
                (devfs_handles[handle].begin + offset) < devfs_handles[handle].begin) return -1;
            devfs_handles[handle].ptr = devfs_handles[handle].begin + offset;
            return devfs_handles[handle].ptr;
        case SEEK_END:
            if ((devfs_handles[handle].end + offset) > devfs_handles[handle].end ||
                (devfs_handles[handle].end + offset) < devfs_handles[handle].begin) return -1;
            devfs_handles[handle].ptr = devfs_handles[handle].end + offset;
            return devfs_handles[handle].ptr;
        case SEEK_CUR:
            if ((devfs_handles[handle].ptr + offset) > devfs_handles[handle].end ||
                (devfs_handles[handle].ptr + offset) < devfs_handles[handle].begin) return -1;
            devfs_handles[handle].ptr += offset;
            return devfs_handles[handle].ptr;
        default:
            return -1;
    }
}

int devfs_fork(int handle) {
    return devfs_create_handle(devfs_handles[handle]);
}

void install_devfs(void) {
    vfs_install_fs("devfs", &devfs_read, &devfs_write, &devfs_remove, &devfs_mkdir,
                            &devfs_create, &devfs_get_metadata, &devfs_list, &devfs_mount,
                            &devfs_open, &devfs_close, &devfs_fork, &devfs_uread,
                            &devfs_uwrite, &devfs_seek );
}
