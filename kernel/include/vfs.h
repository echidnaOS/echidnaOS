#ifndef __VFS_H__
#define __VFS_H__

#include <stdint.h>
#include <kernel.h>


#define O_RDONLY        0b0001
#define O_WRONLY        0b0010
#define O_RDWR          0b0100

#define O_APPEND        0b001000
#define O_CREAT         0b010000
#define O_TRUNC         0b100000

#define SEEK_SET 0
#define SEEK_END 1
#define SEEK_CUR 2

#define FILE_TYPE               0
#define DIRECTORY_TYPE          1
#define DEVICE_TYPE             2

#define IO_NOT_READY -5




typedef struct {
    int free;
    int mountpoint;
    int internal_handle;
} file_handle_t;

typedef struct {
    char filename[2048];
    int filetype;
    uint64_t size;
} vfs_metadata_t;

typedef struct {
    char name[128];
    int (*read)(char *path, uint64_t loc, char *dev);
    int (*write)(char *path, uint8_t val, uint64_t loc, char *dev);
    int (*remove)(char *path, char *dev);
    int (*mkdir)(char *path, uint16_t perms, char *dev);
    int (*create)(char *path, uint16_t perms, char *dev);
    int (*get_metadata)(char *path, vfs_metadata_t *metadata, int type, char *dev);
    int (*list)(char *path, vfs_metadata_t *metadata, uint32_t entry, char *dev);
    int (*mount)(char *device);
    int (*open)(char *path, int flags, int mode, char *dev);
    int (*close)(int handle);
    int (*fork)(int handle);
    int (*uread)(int handle, char *ptr, int len);
    int (*uwrite)(int handle, char *ptr, int len);
    int (*seek)(int handle, int offset, int type);
} filesystem_t;

typedef struct {
    char mountpoint[2048];
    char device[2048];
    char filesystem[128];
} mountpoint_t;


extern filesystem_t *filesystems;
extern mountpoint_t *mountpoints;


int create_file_handle(int pid, file_handle_t handle);
int read(int handle, char *ptr, int len);
int write(int handle, char *ptr, int len);

int vfs_translate_fs(int mountpoint);

int vfs_list(char *path, vfs_metadata_t *metadata, uint32_t entry);
int vfs_get_metadata(char *path, vfs_metadata_t *metadata, int type);
int vfs_kget_metadata(char *path, vfs_metadata_t *metadata, int type);
int vfs_read(char *path, uint64_t loc);
int vfs_kread(char *path, uint64_t loc);
int vfs_write(char *path, uint64_t loc, uint8_t val);
int vfs_kwrite(char *path, uint64_t loc, uint8_t val);
int vfs_remove(char *path);
int vfs_kremove(char *path);
int vfs_mkdir(char *path, uint16_t perms);
int vfs_kmkdir(char *path, uint16_t perms);
int vfs_create(char *path, uint16_t perms);
int vfs_kcreate(char *path, uint16_t perms);
int vfs_cd(char *path);

int vfs_open(char *path, int flags, int mode);
int vfs_kopen(char *path, int flags, int mode);
int vfs_close(int handle);
int vfs_kclose(int handle);

int vfs_kfork(int handle);
int vfs_seek(int handle, int offset, int type);
int vfs_kseek(int handle, int offset, int type);

int vfs_uread(int handle, char *ptr, int len);
int vfs_kuread(int handle, char *ptr, int len);
int vfs_uwrite(int handle, char *ptr, int len);
int vfs_kuwrite(int handle, char *ptr, int len);

int vfs_mount(char *mountpoint, char *device, char *filesystem);
void vfs_install_fs(char* name,
                    int (*read)(char *path, uint64_t loc, char *dev),
                    int (*write)(char *path, uint8_t val, uint64_t loc, char *dev),
                    int (*remove)(char *path, char *dev),
                    int (*mkdir)(char *path, uint16_t perms, char *dev),
                    int (*create)(char *path, uint16_t perms, char *dev),
                    int (*get_metadata)(char *path, vfs_metadata_t *metadata, int type, char *dev),
                    int (*list)(char *path, vfs_metadata_t *metadata, uint32_t entry, char *dev),
                    int (*mount)(char *device),
                    int (*open)(char *path, int flags, int mode, char *dev),
                    int (*close)(int handle),
                    int (*fork)(int handle),
                    int (*uread)(int handle, char *ptr, int len),
                    int (*uwrite)(int handle, char *ptr, int len),
                    int (*seek)(int handle, int offset, int type) );






#endif
