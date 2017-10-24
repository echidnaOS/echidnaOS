/*
 *  echidnaOS kernel API
 */

#ifndef __SYS_API_H__
#define __SYS_API_H__

#define VFS_FILE_TYPE 0
#define VFS_DIRECTORY_TYPE 1
#define VFS_DEVICE_TYPE 2
#define VFS_SUCCESS 0
#define VFS_FAILURE -2

#define ECH_O_RDONLY        0b0001
#define ECH_O_WRONLY        0b0010
#define ECH_O_RDWR          0b0100

#define ECH_O_APPEND        0b001000
#define ECH_O_CREAT         0b010000
#define ECH_O_TRUNC         0b100000

#define ECH_SEEK_SET 0
#define ECH_SEEK_END 1
#define ECH_SEEK_CUR 2

#define OS_open(path, flags, mode) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x2a, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)         \
                     : "c" (path),  \
                       "d" (flags), \
                       "D" (mode) \
                     :  );         \
    return_val;                                \
})

#define OS_close(handle) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x2b, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)         \
                     : "c" (handle) \
                     : "edx" );         \
    return_val;                                \
})

#define OS_read(handle, buf, len) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x2c, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)         \
                     : "c" (handle),  \
                       "d" (buf), \
                       "D" (len) \
                     :  );         \
    return_val;                                \
})

#define OS_write(handle, buf, len) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x2d, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)         \
                     : "c" (handle),  \
                       "d" (buf), \
                       "D" (len) \
                     :  );         \
    return_val;                                \
})

#define OS_lseek(a, b, c) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x2e, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)         \
                     : "c" (a),  \
                       "d" (b), \
                       "D" (c) \
                     :  );         \
    return_val;                                \
})

#define OS_signal(sig, handler) ({ \
    int ret; \
    asm volatile (  "movl $0x16, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret)                 \
                     : "c" (sig),        \
                       "d" (handler)    \
                     :  );         \
    ret; \
})

#define OS_getpid() ({              \
    uint32_t val;                         \
    asm volatile (  "movl $0x15, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (val)      \
                     : \
                     : "edx" );         \
    val;                               \
})

#define OS_get_heap_base() ({              \
    uint32_t val;                         \
    asm volatile (  "movl $0x10, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (val)      \
                     : \
                     : "edx" );         \
    val;                               \
})

#define OS_get_heap_size() ({              \
    uint32_t val;                         \
    asm volatile (  "movl $0x11, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (val)      \
                     : \
                     : "edx" );         \
    val;                               \
})

#define OS_resize_heap(val) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x12, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)      \
                     : "c" (val)  \
                     : "edx" );         \
    return_val;                                \
})

#define OS_pwd(value) ({                \
    asm volatile (  "movl $0x1a, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_what_stdin(value) ({                \
    asm volatile (  "movl $0x1b, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_what_stdout(value) ({                \
    asm volatile (  "movl $0x1c, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_what_stderr(value) ({                \
    asm volatile (  "movl $0x1d, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     :                  \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_ipc_send_packet(pid, payload, len) ({ \
    asm volatile (  "movl $0x08, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     :                  \
                     : "c" (pid),        \
                       "d" (payload),    \
                       "D" (len)      \
                     : "eax" );         \
})

#define OS_ipc_read_packet(payload) ({  \
    uint32_t pid;                            \
    asm volatile (  "movl $0x09, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (pid)         \
                     : "c" (payload)    \
                     : "edx" );         \
    pid;                                \
})

#define OS_ipc_resolve_name(server_name) ({  \
    uint32_t pid;                            \
    asm volatile (  "movl $0x0a, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (pid)         \
                     : "c" (server_name)  \
                     : "edx" );         \
    pid;                                \
})

#define OS_ipc_payload_sender() ({      \
    uint32_t pid;                              \
    asm volatile (  "movl $0x0b, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (pid)         \
                     :                  \
                     : "edx" );         \
    pid;                                  \
})

#define OS_ipc_payload_length() ({      \
    uint32_t pid;                              \
    asm volatile (  "movl $0x0c, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (pid)         \
                     :                  \
                     : "edx" );         \
    pid;                                  \
})

#define OS_ipc_await() ({                \
    asm volatile (  "movl $0x0d, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     :                  \
                     :                    \
                     :  );  \
})

#define OS_vfs_list(path, metadata, entry) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x32, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)         \
                     : "c" (path),  \
                       "d" (metadata), \
                       "D" (entry) \
                     :  );         \
    return_val;                                \
})

#define OS_vfs_get_metadata(path, metadata, type) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x33, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)         \
                     : "c" (path),  \
                       "d" (metadata), \
                       "D" (type) \
                     :  );         \
    return_val;                                \
})

#define OS_vfs_read(path, loc) ({  \
    int return_val;                            \
    uint32_t loc_low = loc & 0x00000000ffffffff; \
    uint32_t loc_high = loc / 0x100000000; \
    asm volatile (  "movl $0x30, %%eax\n\t"    \
                    "int $0x80\n\t"         \
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
    asm volatile (  "movl $0x31, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)         \
                     : "c" (path),  \
                       "d" (loc_low), \
                       "D" (loc_high), \
                       "S" (val) \
                     :  );         \
    return_val;                                \
})

#define OS_vfs_mkdir(path, perms) ({ \
    int ret; \
    asm volatile (  "movl $0x35, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret)                 \
                     : "c" (path),        \
                       "d" (perms)    \
                     :  );         \
    ret; \
})

#define OS_vfs_create(path, perms) ({ \
    int ret; \
    asm volatile (  "movl $0x36, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret)                 \
                     : "c" (path),        \
                       "d" (perms)    \
                     :  );         \
    ret; \
})

#define OS_vfs_cd(path) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x2f, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)      \
                     : "c" (path)  \
                     : "edx" );         \
    return_val;                                \
})

#define OS_vfs_remove(path) ({  \
    int return_val;                            \
    asm volatile (  "movl $0x34, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (return_val)      \
                     : "c" (path)  \
                     : "edx" );         \
    return_val;                                \
})

#define OS_vdev_in_ready(value) ({               \
    int ret; \
    asm volatile (  "movl $0x21, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret) \
                     : "c" (value)      \
                     : "edx" );  \
    ret; \
})

#define OS_vdev_out_ready(value) ({               \
    int ret; \
    asm volatile (  "movl $0x22, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret) \
                     : "c" (value)      \
                     : "edx" );  \
    ret; \
})

#define OS_vdev_register(in, fin, out, fout) ({ \
    int ret; \
    asm volatile (  "movl $0x20, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret)                 \
                     : "c" (in),        \
                       "d" (fin),    \
                       "D" (out), \
                       "S" (fout) \
                     :  );         \
    ret; \
})

#define OS_vdev_await() ({                \
    int ret; \
    asm volatile (  "movl $0x23, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret)                 \
                     :                    \
                     : "edx" );  \
    ret; \
})

#define OS_general_execute(value) ({               \
    int ret; \
    asm volatile (  "movl $0x01, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret) \
                     : "c" (value)      \
                     : "edx" );  \
    ret; \
})

#define OS_general_execute_block(value) ({               \
    uint32_t ret_low; \
    uint32_t ret_hi; \
    uint64_t ret; \
    asm volatile (  "movl $0x02, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret_low),                 \
                       "=d" (ret_hi) \
                     : "c" (value)      \
                     :  );  \
    ret = ((uint64_t)(ret_hi) << 32) + (uint64_t)ret_low; \
    ret; \
})

#define OS_exit(value) ({               \
    asm volatile (  "movl $0x00, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : \
                     : "c" (value)      \
                     : "eax", "edx" );  \
})

#define OS_fork() ({               \
    int ret; \
    asm volatile (  "movl $0x05, %%eax\n\t"    \
                    "int $0x80\n\t"         \
                     : "=a" (ret) \
                     :     \
                     : "edx" );  \
    ret; \
})

#endif
