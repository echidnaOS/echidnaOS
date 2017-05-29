/*
This file includes the description of the actual system calls to be made to
the OS.
This defaults to the echidnaOS system calls, but can be modified to fit
any other OS.
*/

#ifndef __ARCH_H__
#define __ARCH_H__

#define char_to_stdout(value) ({        \
    asm volatile (  "push eax;"         \
                    "mov eax, 0x20;"    \
                    "int 0x80;"         \
                    "pop eax;"          \
                    :                   \
                    : "a" (value)       \
                    : );                \
})

#define char_from_stdin() ({            \
    int c;                              \
    asm volatile (  "mov eax, 0x21;"    \
                    "int 0x80;"         \
                    : "=a" (c)          \
                    :                   \
                    : );                \
    c;                                  \
})

#endif
