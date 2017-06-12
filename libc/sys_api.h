/*
 *  echidnaOS kernel API
 */

#ifndef __SYS_API_H__
#define __SYS_API_H__

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
    asm volatile (  "mov eax, 0x24;"    \
                    "int 0x80;"         \
                     :                  \
                     :                  \
                     : "eax", "edx" );  \
})

#endif
