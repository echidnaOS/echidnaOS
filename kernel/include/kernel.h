#ifndef __KERNEL_H__
#define __KERNEL_H__


// misc tunables

#define KRNL_PIT_FREQ 4000
//#define _SERIAL_KERNEL_OUTPUT_


// task tunables

#define KRNL_MAX_TASKS 65536
#define DEFAULT_STACK 0x10000


// tty tunables

//<<<<<<< Updated upstream
#define _BIG_FONTS_
/*=======
typedef struct {
    int free;
    char path[1024];
    int flags;
    int mode;
    long ptr;
    long begin;
    long end;
    int isblock;
} file_handle_t;

typedef struct {
    int free;
    int mountpoint;
    int internal_handle;
} file_handle_v2_t;

typedef struct {

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t cs;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t ss;
    uint32_t eflags;

} cpu_t;

typedef struct {

    int status;
    int parent;
    
    uint32_t base;
    uint32_t pages;

    cpu_t cpu;
    
    char pwd[2048];
    char name[128];
    char server_name[128];
    
    char stdin[2048];
    char stdout[2048];
    char stderr[2048];
    
    char iowait_dev[2048];
    uint64_t iowait_loc;
    int iowait_type;
    uint8_t iowait_payload;
    int iowait_handle;
    uint32_t iowait_ptr;
    int iowait_len;
    int iowait_done;
    
    ipc_packet_t *ipc_queue;
    uint32_t ipc_queue_ptr;
    
    uint32_t heap_base;
    uint32_t heap_size;
    
    // signals
    uint32_t sigabrt;
    uint32_t sigfpe;
    uint32_t sigill;
    uint32_t sigint;
    uint32_t sigsegv;
    uint32_t sigterm;
    
    file_handle_t *file_handles;
    int file_handles_ptr;

    file_handle_v2_t *file_handles_v2;
    int file_handles_v2_ptr;

} task_t;

typedef struct {
    char *path;
    char *stdin;
    char *stdout;
    char *stderr;
    char *pwd;
    char *name;
    char *server_name;
    int argc;
    char **argv;
} task_info_t;

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

typedef struct {
    char name[32];
    uint32_t gp_value;
    uint64_t size;
    int (*io_wrapper)(uint32_t, uint64_t, int, uint8_t);
} device_t;

int create_file_handle(int pid, file_handle_t handle);
int create_file_handle_v2(int pid, file_handle_v2_t handle);
int read(int handle, char *ptr, int len);
int write(int handle, char *ptr, int len);

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

int task_create(task_t new_task);
void task_scheduler(void);
void task_quit(int pid, int64_t return_value);
int general_execute(task_info_t *task_info);

typedef struct {
    int free;
    uint32_t size;
    uint32_t prev_chunk;
} heap_chunk_t;

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) GDT_entry_t;

typedef struct {
    uint32_t cursor_offset;
    int cursor_status;
    uint8_t cursor_palette;
    uint8_t text_palette;
    char field[VD_ROWS * VD_COLS];
    char kb_l1_buffer[KB_L1_SIZE];
    char kb_l2_buffer[KB_L2_SIZE];
    uint16_t kb_l1_buffer_index;
    uint16_t kb_l2_buffer_index;
    int escape;
    int *esc_value;
    int esc_value0;
    int esc_value1;
    int *esc_default;
    int esc_default0;
    int esc_default1;
    int raw;
    int noblock;
    int noscroll;
} tty_t;

extern uint32_t memory_size;
extern int current_task;
extern task_t **task_table;
extern uint8_t current_tty;

extern tty_t tty[KRNL_TTY_COUNT];

extern device_t *device_list;
extern uint32_t device_ptr;

void panic(const char *msg);

void task_init(void);

void init_kalloc(void);
void *kalloc(uint32_t size);
void *krealloc(void *addr, uint32_t new_size);
void kfree(void *addr);

void ipc_send_packet(uint32_t pid, char *payload, uint32_t len);
uint32_t ipc_read_packet(char *payload);
uint32_t ipc_resolve_name(char *server_name);
uint32_t ipc_payload_length(void);
uint32_t ipc_payload_sender(void);

void vga_disable_cursor(void);
void vga_80_x_50(void);
uint32_t detect_mem(void);

void *alloc(uint32_t size);

void tty_refresh(uint8_t which_tty);
>>>>>>> Stashed changes*/

#define KB_L1_SIZE 256
#define KB_L2_SIZE 2048

#define KRNL_TTY_COUNT 7
#define TTY_DEF_CUR_PAL 0x70
#define TTY_DEF_TXT_PAL 0x07


// vfs tunables

#define MAX_SIMULTANOUS_VFS_ACCESS  4096


#endif
