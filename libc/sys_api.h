/*
 *  echidnaOS kernel API
 */

#ifndef __SYS_API_H__
#define __SYS_API_H__

#define OS_putc(value) ({               \
    asm volatile (  "push eax;"         \
                    "mov eax, 0x20;"    \
                    "int 0x80;"         \
                    "pop eax;"          \
                     :                  \
                     : "a" (value)      \
                     : );               \
})

#define OS_getc() ({                    \
    int c;                              \
    asm volatile (  "mov eax, 0x21;"    \
                    "int 0x80;"         \
                     : "=a" (c)         \
                     :                  \
                     : );               \
    c;                                  \
})

#define OS_cls() ({                     \
    asm volatile (  "mov eax, 0x24;"    \
                    "int 0x80;"         \
                     :                  \
                     :                  \
                     : "eax" );         \
})

#endif
