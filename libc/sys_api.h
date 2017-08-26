/*
 *  echidnaOS kernel API
 */

#ifndef __SYS_API_H__
#define __SYS_API_H__

#include <stdint.h>

typedef struct {
    char filename[2048];
    int filetype;
} vfs_metadata_t;

typedef struct {
    char* path;
    char* stdin;
    char* stdout;
    char* stderr;
    char* pwd;
    char* name;
    char* server_name;
} task_info_t;

#define OS_alloc(value) ({              \
    void* addr;                         \
    asm volatile (  "mov eax, 0x10;"    \
                    "int 0x80;"         \
                     : "=a" (addr)      \
                     : "c" (value)      \
                     : "edx" );         \
    addr;                               \
})

#define OS_free(value) ({               \
    asm volatile (  "mov eax, 0x11;"    \
                    "int 0x80;"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_putc(value) ({               \
    asm volatile (  "mov eax, 0x20;"    \
                    "int 0x80;"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_getc() ({                    \
    int c;                              \
    asm volatile (  "mov eax, 0x21;"    \
                    "int 0x80;"         \
                     : "=a" (c)         \
                     :                  \
                     : "edx" );         \
    c;                                  \
})

#define OS_pwd(value) ({                \
    asm volatile (  "mov eax, 0x1a;"    \
                    "int 0x80;"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_what_stdin(value) ({                \
    asm volatile (  "mov eax, 0x1b;"    \
                    "int 0x80;"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_what_stdout(value) ({                \
    asm volatile (  "mov eax, 0x1c;"    \
                    "int 0x80;"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_what_stderr(value) ({                \
    asm volatile (  "mov eax, 0x1d;"    \
                    "int 0x80;"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_ipc_send_packet(pid, payload, len) ({ \
    asm volatile (  "mov eax, 0x08;"    \
                    "int 0x80;"         \
                     :                  \
                     : "c" (pid),        \
                       "d" (payload),    \
                       "D" (len)      \
                     : "eax" );         \
})

#define OS_ipc_read_packet(payload) ({  \
    uint32_t pid;                            \
    asm volatile (  "mov eax, 0x09;"    \
                    "int 0x80;"         \
                     : "=a" (pid)         \
                     : "c" (payload)    \
                     : "edx" );         \
    pid;                                \
})

#define OS_ipc_resolve_name(server_name) ({  \
    uint32_t pid;                            \
    asm volatile (  "mov eax, 0x0a;"    \
                    "int 0x80;"         \
                     : "=a" (pid)         \
                     : "c" (server_name)  \
                     : "edx" );         \
    pid;                                \
})

#define OS_ipc_payload_sender() ({      \
    uint32_t pid;                              \
    asm volatile (  "mov eax, 0x0b;"    \
                    "int 0x80;"         \
                     : "=a" (pid)         \
                     :                  \
                     : "edx" );         \
    pid;                                  \
})

#define OS_ipc_payload_length() ({      \
    uint32_t pid;                              \
    asm volatile (  "mov eax, 0x0c;"    \
                    "int 0x80;"         \
                     : "=a" (pid)         \
                     :                  \
                     : "edx" );         \
    pid;                                  \
})

#define OS_ipc_await() ({                \
    asm volatile (  "mov eax, 0x0d;"    \
                    "int 0x80;"         \
                     :                  \
                     :                    \
                     :  );  \
})

#define OS_vfs_list(path, metadata, entry) ({  \
    int return_val;                            \
    asm volatile (  "mov eax, 0x32;"    \
                    "int 0x80;"         \
                     : "=a" (return_val)         \
                     : "c" (path),  \
                       "d" (metadata), \
                       "D" (entry) \
                     :  );         \
    return_val;                                \
})

#define OS_vfs_read(path, loc) ({  \
    int return_val;                            \
    uint32_t loc_low = loc & 0x00000000ffffffff; \
    uint32_t loc_high = loc / 0x100000000; \
    asm volatile (  "mov eax, 0x30;"    \
                    "int 0x80;"         \
                     : "=a" (return_val)         \
                     : "c" (path),  \
                       "d" (loc_low), \
                       "D" (loc_high) \
                     :  );         \
    return_val;                                \
})

#define OS_vfs_write(path, loc, val) ({  \
    int return_val;                            \
    uint32_t loc_low = loc & 0x00000000ffffffff; \
    uint32_t loc_high = loc / 0x100000000; \
    asm volatile (  "mov eax, 0x31;"    \
                    "int 0x80;"         \
                     : "=a" (return_val)         \
                     : "c" (path),  \
                       "d" (loc_low), \
                       "D" (loc_high), \
                       "S" (val) \
                     :  );         \
    return_val;                                \
})

#define OS_vfs_cd(path) ({  \
    int return_val;                            \
    asm volatile (  "mov eax, 0x2f;"    \
                    "int 0x80;"         \
                     : "=a" (return_val)      \
                     : "c" (path)  \
                     : "edx" );         \
    return_val;                                \
})

#define OS_general_execute(value) ({               \
    int ret; \
    asm volatile (  "mov eax, 0x01;"    \
                    "int 0x80;"         \
                     : "=a" (ret) \
                     : "c" (value)      \
                     : "edx" );  \
    ret; \
})

#define OS_general_execute_block(value) ({               \
    uint32_t ret_low; \
    uint32_t ret_hi; \
    uint64_t ret; \
    asm volatile (  "mov eax, 0x02;"    \
                    "int 0x80;"         \
                     : "=a" (ret_low),                 \
                       "=d" (ret_hi) \
                     : "c" (value)      \
                     :  );  \
    ret = ((uint64_t)(ret_hi) << 32) + (uint64_t)ret_low; \
    ret; \
})

#endif
