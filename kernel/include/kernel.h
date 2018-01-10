// general kernel header

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdint.h>

// kernel tunables

#define _BIG_FONTS_
//#define _SERIAL_KERNEL_OUTPUT_

#ifdef _BIG_FONTS_
  #define VD_ROWS 25
#else
  #define VD_ROWS 50
#endif
#define VD_COLS 160

#define KB_L1_SIZE 256
#define KB_L2_SIZE 2048

#define KRNL_PIT_FREQ 4000
#define KRNL_TTY_COUNT 7

#define TTY_DEF_CUR_PAL 0x70
#define TTY_DEF_TXT_PAL 0x07

#define KRNL_MEMORY_BASE 0x1000000
#define KRNL_MAX_TASKS 65536

#define DEFAULT_STACK 0x10000

#define O_RDONLY        0b0001
#define O_WRONLY        0b0010
#define O_RDWR          0b0100

#define O_APPEND        0b001000
#define O_CREAT         0b010000
#define O_TRUNC         0b100000

#define SEEK_SET 0
#define SEEK_END 1
#define SEEK_CUR 2

// memory statuses

#define KRN_STAT_ACTIVE_TASK    1
#define KRN_STAT_RES_TASK       2
#define KRN_STAT_IOWAIT_TASK    3
#define KRN_STAT_IPCWAIT_TASK   4
#define KRN_STAT_PROCWAIT_TASK  5
#define KRN_STAT_VDEVWAIT_TASK  6

#define EMPTY_PID               (task_t*)0xffffffff
#define TASK_RESERVED_SPACE     0x10000
#define PAGE_SIZE               4096

#define FILE_TYPE               0
#define DIRECTORY_TYPE          1
#define DEVICE_TYPE             2

#define IO_NOT_READY -5

// signals

#define SIGABRT                 0
#define SIGFPE                  1
#define SIGILL                  2
#define SIGINT                  3
#define SIGSEGV                 4
#define SIGTERM                 5

#define SIG_ERR                 0xffffffff

// macros

#define DISABLE_INTERRUPTS      asm volatile ("cli")
#define ENABLE_INTERRUPTS       asm volatile ("sti")
#define ENTER_IDLE              \
    asm volatile (              \
                    "sti;"      \
                    "1:"        \
                    "mov esp, 0xeffff0;"    \
                    "hlt;"      \
                    "jmp 1b;"   \
                 )

// driver inits

void init_bios_harddisks(void);
void init_ata(void);
void init_pcspk(void);
void init_tty_drv(void);
void init_streams(void);
void init_com(void);
void init_stty(void);

// end driver inits
// fs inits

void install_devfs(void);
void install_echfs(void);

// end fs inits

void kernel_add_device(char* name, uint32_t gp_value, uint64_t size,
                       int (*io_wrapper)(uint32_t, uint64_t, int, uint8_t));

// prototypes

void kputs(const char* string);
void tty_kputs(const char* string, uint8_t which_tty);
void knputs(const char* string, uint32_t count);
void tty_knputs(const char* string, uint32_t count, uint8_t which_tty);
void kuitoa(uint64_t x);
void tty_kuitoa(uint64_t x, uint8_t which_tty);
void kxtoa(uint64_t x);
void tty_kxtoa(uint64_t x, uint8_t which_tty);

#ifdef _SERIAL_KERNEL_OUTPUT_
  void debug_kernel_console_init(void);
  int com_io_wrapper(uint32_t dev, uint64_t loc, int type, uint8_t payload);
#endif

int kstrcmp(char* dest, char* source);
int kstrncmp(char* dest, char* source, uint32_t len);
void kmemcpy(char* dest, char* source, uint32_t count);
void kstrcpy(char* dest, char* source);
uint32_t kstrlen(char* str);

uint64_t power(uint64_t x, uint64_t y);

void switch_tty(uint8_t which_tty);
void init_tty(void);

typedef struct {
    uint32_t sender;
    uint32_t length;
    char* payload;
} ipc_packet_t;

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

    int status;
    int parent;
    
    uint32_t base;
    uint32_t pages;
    
    uint32_t eax_p;
    uint32_t ebx_p;
    uint32_t ecx_p;
    uint32_t edx_p;
    uint32_t esi_p;
    uint32_t edi_p;
    uint32_t ebp_p;
    uint32_t esp_p;
    uint32_t eip_p;
    uint32_t cs_p;
    uint32_t ds_p;
    uint32_t es_p;
    uint32_t fs_p;
    uint32_t gs_p;
    uint32_t ss_p;
    uint32_t eflags_p;
    
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
    
    ipc_packet_t* ipc_queue;
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
    
    file_handle_t* file_handles;
    int file_handles_ptr;

    file_handle_v2_t* file_handles_v2;
    int file_handles_v2_ptr;

} task_t;

typedef struct {
    char* path;
    char* stdin;
    char* stdout;
    char* stderr;
    char* pwd;
    char* name;
    char* server_name;
    int argc;
    char** argv;
} task_info_t;

typedef struct {
    char filename[2048];
    int filetype;
    uint64_t size;
} vfs_metadata_t;

typedef struct {
    char name[128];
    int (*read)(char* path, uint64_t loc, char* dev);
    int (*write)(char* path, uint8_t val, uint64_t loc, char* dev);
    int (*remove)(char* path, char* dev);
    int (*mkdir)(char* path, uint16_t perms, char* dev);
    int (*create)(char* path, uint16_t perms, char* dev);
    int (*get_metadata)(char* path, vfs_metadata_t* metadata, int type, char* dev);
    int (*list)(char* path, vfs_metadata_t* metadata, uint32_t entry, char* dev);
    int (*mount)(char* device);
    int (*open)(char* path, int flags, int mode, char* dev);
    int (*close)(int handle);
    int (*fork)(int handle);
    int (*uread)(int handle, char* ptr, int len);
    int (*uwrite)(int handle, char* ptr, int len);
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
int read(int handle, char* ptr, int len);
int write(int handle, char* ptr, int len);

int vfs_list(char* path, vfs_metadata_t* metadata, uint32_t entry);
int vfs_get_metadata(char* path, vfs_metadata_t* metadata, int type);
int vfs_kget_metadata(char* path, vfs_metadata_t* metadata, int type);
int vfs_read(char* path, uint64_t loc);
int vfs_kread(char* path, uint64_t loc);
int vfs_write(char* path, uint64_t loc, uint8_t val);
int vfs_kwrite(char* path, uint64_t loc, uint8_t val);
int vfs_remove(char* path);
int vfs_kremove(char* path);
int vfs_mkdir(char* path, uint16_t perms);
int vfs_kmkdir(char* path, uint16_t perms);
int vfs_create(char* path, uint16_t perms);
int vfs_kcreate(char* path, uint16_t perms);
int vfs_cd(char* path);

int vfs_open(char* path, int flags, int mode);
int vfs_kopen(char* path, int flags, int mode);
int vfs_close(int handle);
int vfs_kclose(int handle);

int vfs_kfork(int handle);
int vfs_kseek(int handle, int offset, int type);

int vfs_uread(int handle, char* ptr, int len);
int vfs_kuread(int handle, char* ptr, int len);
int vfs_uwrite(int handle, char* ptr, int len);
int vfs_kuwrite(int handle, char* ptr, int len);

int vfs_mount(char* mountpoint, char* device, char* filesystem);
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
                    int (*seek)(int handle, int offset, int type) );

int task_create(task_t new_task);
void task_fork(uint32_t eax_r, uint32_t ebx_r, uint32_t ecx_r, uint32_t edx_r, uint32_t esi_r, uint32_t edi_r, uint32_t ebp_r, uint32_t ds_r, uint32_t es_r, uint32_t fs_r, uint32_t gs_r, uint32_t eip_r, uint32_t cs_r, uint32_t eflags_r, uint32_t esp_r, uint32_t ss_r);
//uint32_t task_start(task_info_t* task_info);
void task_scheduler(void);
void task_terminate(int pid);
void task_switch(uint32_t eax_r, uint32_t ebx_r, uint32_t ecx_r, uint32_t edx_r, uint32_t esi_r, uint32_t edi_r, uint32_t ebp_r, uint32_t ds_r, uint32_t es_r, uint32_t fs_r, uint32_t gs_r, uint32_t eip_r, uint32_t cs_r, uint32_t eflags_r, uint32_t esp_r, uint32_t ss_r);
int general_execute(task_info_t* task_info);

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
    int* esc_value;
    int esc_value0;
    int esc_value1;
    int* esc_default;
    int esc_default0;
    int esc_default1;
    int raw;
    int noblock;
    int noscroll;
} tty_t;

extern uint32_t memory_size;
extern int current_task;
extern task_t** task_table;
extern uint8_t current_tty;

extern tty_t tty[KRNL_TTY_COUNT];

extern device_t* device_list;
extern uint32_t device_ptr;

void panic(const char *msg);

void task_init(void);

void init_kalloc(void);
void* kalloc(uint32_t size);
void* krealloc(void* addr, uint32_t new_size);
void kfree(void* addr);

void ipc_send_packet(uint32_t pid, char* payload, uint32_t len);
uint32_t ipc_read_packet(char* payload);
uint32_t ipc_resolve_name(char* server_name);
uint32_t ipc_payload_length(void);
uint32_t ipc_payload_sender(void);

void vga_disable_cursor(void);
void vga_80_x_50(void);
uint32_t detect_mem(void);

void* alloc(uint32_t size);

void tty_refresh(uint8_t which_tty);

void text_putchar(char c, uint8_t which_tty);
uint32_t text_get_cursor_pos_x(uint8_t which_tty);
uint32_t text_get_cursor_pos_y(uint8_t which_tty);
void text_set_cursor_pos(uint32_t x, uint32_t y, uint8_t which_tty);
void text_set_cursor_palette(uint8_t c, uint8_t which_tty);
uint8_t text_get_cursor_palette(uint8_t which_tty);
void text_set_text_palette(uint8_t c, uint8_t which_tty);
uint8_t text_get_text_palette(uint8_t which_tty);
void text_clear(uint8_t which_tty);
void text_disable_cursor(uint8_t which_tty);
void text_enable_cursor(uint8_t which_tty);

void map_PIC(uint8_t PIC0Offset, uint8_t PIC1Offset);
void set_PIC0_mask(uint8_t mask);
void set_PIC1_mask(uint8_t mask);
uint8_t get_PIC0_mask(void);
uint8_t get_PIC1_mask(void);

void set_pit_freq(uint32_t frequency);

void load_GDT(void);
void load_TSS(void);
void set_segment(uint16_t entry, uint32_t base, uint32_t page_count);

void load_IDT(void);

void keyboard_init(void);
void keyboard_handler(uint8_t input_byte);
int keyboard_fetch_char(uint8_t which_tty);

#endif
