#include <stdint.h>
#include <kernel.h>
#include <paging.h>
#include <klib.h>
#include <task.h>
#include <vfs.h>

#define FAILURE -2
#define SUCCESS 0
#define EOF -1

filesystem_t* filesystems;
static mountpoint_t* mountpoints;
static int mountpoints_ptr = 0;
static int filesystems_ptr = 0;

int create_file_handle_v2(int pid, file_handle_v2_t handle) {
    int handle_n;

    // check for a free handle first
    for (int i = 0; i < task_table[pid]->file_handles_v2_ptr; i++) {
        if (task_table[pid]->file_handles_v2[i].free) {
            handle_n = i;
            goto load_handle;
        }
    }

    task_table[pid]->file_handles_v2 = krealloc(task_table[pid]->file_handles_v2, (task_table[pid]->file_handles_v2_ptr + 1) * sizeof(file_handle_v2_t));
    handle_n = task_table[pid]->file_handles_v2_ptr++;
    
load_handle:
    task_table[pid]->file_handles_v2[handle_n] = handle;
    
    return handle_n;

}

int vfs_translate_mnt(char* path, char** local_path) {
    int guess = FAILURE;
    int guess_size = 0;
    for (int i = 0; i < mountpoints_ptr; i++) {
        uint32_t mountpoint_len = kstrlen(mountpoints[i].mountpoint);
        if (!kstrncmp(path, mountpoints[i].mountpoint, mountpoint_len))
            if ( (((path[mountpoint_len] == '/') || (path[mountpoint_len] == '\0'))
                 || (!kstrcmp(mountpoints[i].mountpoint, "/")))
               && (mountpoint_len > guess_size)) {
                guess = i;
                guess_size = mountpoint_len;
            }
    }
    *local_path = path + guess_size;
    if (**local_path == '/') *local_path++;
    return guess;
}

int vfs_translate_fs(int mountpoint) {
    for (int i = 0; i < filesystems_ptr; i++)
        if (!kstrcmp(filesystems[i].name, mountpoints[mountpoint].filesystem))
            return i;
    return FAILURE;
}

void vfs_get_absolute_path(char* path_ptr, char* path) {
    // converts a relative path into an absolute one
    char* orig_ptr = path_ptr;
    
    if (!*path) {
        kstrcpy(path_ptr, task_table[current_task]->pwd);
        return;
    }
    
    if (*path != '/') {
        kstrcpy(path_ptr, task_table[current_task]->pwd);
        path_ptr += kstrlen(path_ptr);
    } else {
        *path_ptr = '/';
        path_ptr++;
        path++;
    }
    
    goto first_run;
    
    for (;;) {
        switch (*path) {
            case '/':
                path++;
first_run:
                if (*path == '/') continue;
                if ((!kstrncmp(path, ".\0", 2))
                ||  (!kstrncmp(path, "./\0", 3))) {
                    goto term;
                }
                if ((!kstrncmp(path, "..\0", 3))
                ||  (!kstrncmp(path, "../\0", 4))) {
                    while (*path_ptr != '/') path_ptr--;
                    if (path_ptr == orig_ptr) path_ptr++;
                    goto term;
                }
                if (!kstrncmp(path, "../", 3)) {
                    while (*path_ptr != '/') path_ptr--;
                    if (path_ptr == orig_ptr) path_ptr++;
                    path += 2;
                    *path_ptr = 0;
                    continue;
                }
                if (!kstrncmp(path, "./", 2)) {
                    path += 1;
                    continue;
                }
                if (((path_ptr-1) != orig_ptr) && (*(path_ptr-1) != '/')) {
                    *path_ptr = '/';
                    path_ptr++;
                }
                continue;
            case '\0':
term:
                if ((*(path_ptr-1) == '/') && ((path_ptr-1) != orig_ptr)) path_ptr--;
                *path_ptr = 0;
                return;
            default:
                *path_ptr = *path;
                path++;
                path_ptr++;
                continue;
        }
    }
}

int vfs_cd(char* path) {
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    char absolute_path[2048];
    
    vfs_metadata_t metadata;
    
    vfs_get_absolute_path(absolute_path, path);

    if (vfs_kget_metadata(absolute_path, &metadata, DIRECTORY_TYPE) == FAILURE)
        return FAILURE;

    kstrcpy(task_table[current_task]->pwd, absolute_path);

    return SUCCESS;
}

int vfs_read(char* path, uint64_t loc) {
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    return vfs_kread(path, loc);
}

int vfs_kread(char* path, uint64_t loc) {
    char* local_path;
    char absolute_path[2048];
    
    if (!kstrncmp(path, ":://", 4)) {
    // read from dev directly
        path += 4;
        for (int i = 0; i < device_ptr; i++) {
            if (!kstrcmp(path, device_list[i].name))
                return (int)(*device_list[i].io_wrapper)(device_list[i].gp_value, loc, 0, 0);
        }
        return FAILURE;
    }
    
    vfs_get_absolute_path(absolute_path, path);

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].read)(local_path, loc, mountpoints[mountpoint].device);
}

int vfs_remove(char* path) {
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    return vfs_kremove(path);
}

int vfs_kremove(char* path) {
    char* local_path;
    char absolute_path[2048];


    vfs_get_absolute_path(absolute_path, path);

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].remove)(local_path, mountpoints[mountpoint].device);
}

int vfs_mkdir(char* path, uint16_t perms) {
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    return vfs_kmkdir(path, perms);
}

int vfs_kmkdir(char* path, uint16_t perms) {
    char* local_path;
    char absolute_path[2048];

    vfs_metadata_t metadata;

    if (vfs_kget_metadata(path, &metadata, DIRECTORY_TYPE) == SUCCESS)
        return FAILURE;

    vfs_get_absolute_path(absolute_path, path);

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].mkdir)(local_path, perms, mountpoints[mountpoint].device);
}

int vfs_create(char* path, uint16_t perms) {
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    return vfs_kcreate(path, perms);
}

int vfs_kcreate(char* path, uint16_t perms) {
    char* local_path;
    char absolute_path[2048];

    vfs_metadata_t metadata;

    if (vfs_kget_metadata(path, &metadata, FILE_TYPE) == SUCCESS)
        return FAILURE;

    vfs_get_absolute_path(absolute_path, path);

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].create)(local_path, perms, mountpoints[mountpoint].device);
}

int vfs_write(char* path, uint64_t loc, uint8_t val) {
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    return vfs_kwrite(path, loc, val);
}

int vfs_kwrite(char* path, uint64_t loc, uint8_t val) {
    char* local_path;
    char absolute_path[2048];
    
    if (!kstrncmp(path, ":://", 4)) {
    // write to dev directly
        path += 4;
        for (int i = 0; i < device_ptr; i++) {
            if (!kstrcmp(path, device_list[i].name))
                return (int)(*device_list[i].io_wrapper)(device_list[i].gp_value, loc, 1, val);
        }
        return FAILURE;
    }
    
    vfs_get_absolute_path(absolute_path, path);

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].write)(local_path, val, loc, mountpoints[mountpoint].device);
}

int vfs_open(char* path, int flags, int mode) {
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);

    char* local_path;
    char absolute_path[2048];

    file_handle_v2_t handle = {0};

    // read from /dev hack FIXME
    if (!kstrncmp(path, ":://", 4)) {
        local_path = path + 4;
        kstrcpy(absolute_path, "/dev/");
        kstrcpy(absolute_path + 5, local_path);
    } else {
        vfs_get_absolute_path(absolute_path, path);
    }

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return -1;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return -1;

    int internal_handle = (*filesystems[filesystem].open)(local_path, flags, mode, mountpoints[mountpoint].device);
    if (internal_handle == -1) return -1;

    handle.free = 0;
    handle.mountpoint = mountpoint;
    handle.internal_handle = internal_handle;

    return create_file_handle_v2(current_task, handle);
}

int vfs_kopen(char* path, int flags, int mode) {
    char* local_path;
    char absolute_path[2048];

    file_handle_v2_t handle = {0};

    // read from /dev hack FIXME
    if (!kstrncmp(path, ":://", 4)) {
        local_path = path + 4;
        kstrcpy(absolute_path, "/dev/");
        kstrcpy(absolute_path + 5, local_path);
    } else {
        vfs_get_absolute_path(absolute_path, path);
    }

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return -1;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return -1;

    int internal_handle = (*filesystems[filesystem].open)(local_path, flags, mode, mountpoints[mountpoint].device);
    if (internal_handle == -1) return -1;

    handle.free = 0;
    handle.mountpoint = mountpoint;
    handle.internal_handle = internal_handle;

    return create_file_handle_v2(0, handle);
}

extern int read_stat;

int vfs_uread(int handle, char* ptr, int len) {
    ptr = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)ptr);
    int filesystem = vfs_translate_fs(task_table[current_task]->file_handles_v2[handle].mountpoint);
    if (len <= MAX_SIMULTANOUS_VFS_ACCESS)
        return (*filesystems[filesystem].uread)(task_table[current_task]->file_handles_v2[handle].internal_handle, ptr, len);
    else {
        int ret = (*filesystems[filesystem].uread)(task_table[current_task]->file_handles_v2[handle].internal_handle, ptr, MAX_SIMULTANOUS_VFS_ACCESS);
        read_stat = 1;
        return ret;
    }
}

int vfs_kuread(int handle, char* ptr, int len) {
    int filesystem = vfs_translate_fs(task_table[0]->file_handles_v2[handle].mountpoint);
    return (*filesystems[filesystem].uread)(task_table[0]->file_handles_v2[handle].internal_handle, ptr, len);
}

extern int write_stat;

int vfs_uwrite(int handle, char* ptr, int len) {
    ptr = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)ptr);
    int filesystem = vfs_translate_fs(task_table[current_task]->file_handles_v2[handle].mountpoint);
    if (len <= MAX_SIMULTANOUS_VFS_ACCESS)
        return (*filesystems[filesystem].uwrite)(task_table[current_task]->file_handles_v2[handle].internal_handle, ptr, len);
    else {
        int ret = (*filesystems[filesystem].uwrite)(task_table[current_task]->file_handles_v2[handle].internal_handle, ptr, MAX_SIMULTANOUS_VFS_ACCESS);
        write_stat = 1;
        return ret;
    }
}

int vfs_kuwrite(int handle, char* ptr, int len) {
    int filesystem = vfs_translate_fs(task_table[0]->file_handles_v2[handle].mountpoint);
    return (*filesystems[filesystem].uwrite)(task_table[0]->file_handles_v2[handle].internal_handle, ptr, len);
}

int vfs_seek(int handle, int offset, int type) {
    int filesystem = vfs_translate_fs(task_table[current_task]->file_handles_v2[handle].mountpoint);
    return (*filesystems[filesystem].seek)(task_table[current_task]->file_handles_v2[handle].internal_handle, offset, type);
}

int vfs_kseek(int handle, int offset, int type) {
    int filesystem = vfs_translate_fs(task_table[0]->file_handles_v2[handle].mountpoint);
    return (*filesystems[filesystem].seek)(task_table[0]->file_handles_v2[handle].internal_handle, offset, type);
}

int vfs_kfork(int handle) {

    int mountpoint = task_table[0]->file_handles_v2[handle].mountpoint;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    int internal_handle = (*filesystems[filesystem].fork)(task_table[0]->file_handles_v2[handle].internal_handle);
    if (internal_handle == FAILURE) return FAILURE;

    file_handle_v2_t new_handle = {0};

    new_handle.free = 0;
    new_handle.mountpoint = mountpoint;
    new_handle.internal_handle = internal_handle;

    return create_file_handle_v2(0, new_handle);

}

int vfs_list(char* path, vfs_metadata_t* metadata, uint32_t entry) {
    char* local_path;
    char absolute_path[2048];
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    
    metadata = (vfs_metadata_t*)get_phys_addr(task_table[current_task]->page_directory, (size_t)metadata);
    
    vfs_get_absolute_path(absolute_path, path);

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].list)(local_path, metadata, entry, mountpoints[mountpoint].device);
}

int vfs_get_metadata(char* path, vfs_metadata_t* metadata, int type) {
    path = (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    metadata = (vfs_metadata_t*)get_phys_addr(task_table[current_task]->page_directory, (size_t)metadata);
    return vfs_kget_metadata(path, metadata, type);
}

int vfs_kget_metadata(char* path, vfs_metadata_t* metadata, int type) {
    char* local_path;
    char absolute_path[2048];
    
    vfs_get_absolute_path(absolute_path, path);

    int mountpoint = vfs_translate_mnt(absolute_path, &local_path);
    if (mountpoint == FAILURE) return FAILURE;

    int filesystem = vfs_translate_fs(mountpoint);
    if (filesystem == FAILURE) return FAILURE;

    return (*filesystems[filesystem].get_metadata)(local_path, metadata, type, mountpoints[mountpoint].device);
}

int vfs_mount(char* mountpoint, char* device, char* filesystem) {
    int i;
    for (i = 0; i < filesystems_ptr; i++)
        if (!kstrcmp(filesystems[i].name, filesystem)) break;
    
    if (((*filesystems[i].mount)(device)) == FAILURE) return FAILURE;
    
    mountpoints = krealloc(mountpoints, sizeof(mountpoint_t) * (mountpoints_ptr+1));
    
    kstrcpy(mountpoints[mountpoints_ptr].mountpoint, (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)mountpoint));
    kstrcpy(mountpoints[mountpoints_ptr].device, (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)device));
    kstrcpy(mountpoints[mountpoints_ptr].filesystem, (char*)get_phys_addr(task_table[current_task]->page_directory, (size_t)filesystem));
    
    kputs("\nMounted `"); kputs(mountpoints[mountpoints_ptr].device);
    kputs("' on `"); kputs(mountpoints[mountpoints_ptr].mountpoint);
    kputs("' using filesystem: "); kputs(mountpoints[mountpoints_ptr].filesystem);
    mountpoints_ptr++;
    return SUCCESS;
}

int vfs_close(int handle) {
    int filesystem = vfs_translate_fs(task_table[current_task]->file_handles_v2[handle].mountpoint);
    int ret = (*filesystems[filesystem].close)(task_table[current_task]->file_handles_v2[handle].internal_handle);
    if (ret == -1)
        return -1;

    if (handle < 0)
        return -1;
        
    if (handle >= task_table[current_task]->file_handles_v2_ptr)
        return -1;
    
    if (task_table[current_task]->file_handles_v2[handle].free)
        return -1;
    
    task_table[current_task]->file_handles_v2[handle].free = 1;
    
    return 0;
}

int vfs_kclose(int handle) {
    int filesystem = vfs_translate_fs(task_table[0]->file_handles_v2[handle].mountpoint);
    int ret = (*filesystems[filesystem].close)(task_table[0]->file_handles_v2[handle].internal_handle);
    if (ret == -1)
        return -1;

    if (handle < 0)
        return -1;
        
    if (handle >= task_table[0]->file_handles_v2_ptr)
        return -1;
    
    if (task_table[0]->file_handles_v2[handle].free)
        return -1;
    
    task_table[0]->file_handles_v2[handle].free = 1;
    
    return 0;
}

void vfs_install_fs(char* name,
                    int (*read)(char* path, uint64_t loc, char* dev),
                    int (*write)(char* path, uint8_t val, uint64_t loc, char* dev),
                    int (*remove)(char* path, char* dev),
                    int (*mkdir)(char* path, uint16_t perms, char* dev),
                    int (*create)(char* path, uint16_t perms, char* dev),
                    int (*get_metadata)(char* path, vfs_metadata_t* metadata, int type, char* dev),
                    int (*list)(char* path, vfs_metadata_t* metadata, uint32_t entry, char* dev),
                    int (*mount)(char* device),
                    int (*open)(char* path, int flags, int mode, char* dev),
                    int (*close)(int handle),
                    int (*fork)(int handle),
                    int (*uread)(int handle, char* ptr, int len),
                    int (*uwrite)(int handle, char* ptr, int len),
                    int (*seek)(int handle, int offset, int type) ) {
    
    filesystems = krealloc(filesystems, sizeof(filesystem_t) * (filesystems_ptr+1));
    
    kstrcpy(filesystems[filesystems_ptr].name, name);
    filesystems[filesystems_ptr].read = read;
    filesystems[filesystems_ptr].write = write;
    filesystems[filesystems_ptr].remove = remove;
    filesystems[filesystems_ptr].mkdir = mkdir;
    filesystems[filesystems_ptr].create = create;
    filesystems[filesystems_ptr].get_metadata = get_metadata;
    filesystems[filesystems_ptr].list = list;
    filesystems[filesystems_ptr].mount = mount;
    filesystems[filesystems_ptr].open = open;
    filesystems[filesystems_ptr].close = close;
    filesystems[filesystems_ptr].fork = fork;
    filesystems[filesystems_ptr].uread = uread;
    filesystems[filesystems_ptr].uwrite = uwrite;
    filesystems[filesystems_ptr].seek = seek;
    
    filesystems_ptr++;
    return;
}
