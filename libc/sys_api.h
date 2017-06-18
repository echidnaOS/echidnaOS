/*
 *  echidnaOS kernel API
 */

#ifndef __SYS_API_H__
#define __SYS_API_H__

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

#define OS_cls() ({                     \
    asm volatile (  "mov eax, 0x22;"    \
                    "int 0x80;"         \
                     :                  \
                     :                  \
                     : "eax", "edx" );  \
})

#endif
