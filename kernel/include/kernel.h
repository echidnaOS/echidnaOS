#ifndef __KERNEL_H__
#define __KERNEL_H__


// misc tunables

#define KRNL_PIT_FREQ 4000
#define _SERIAL_KERNEL_OUTPUT_


// task tunables

#define KRNL_MAX_TASKS 65536
#define DEFAULT_STACK 0x10000


// tty tunables

#define _BIG_FONTS_

#define KB_L1_SIZE 256
#define KB_L2_SIZE 2048

#define KRNL_TTY_COUNT 7
#define TTY_DEF_CUR_PAL 0x70
#define TTY_DEF_TXT_PAL 0x07


// vfs tunables

#define MAX_SIMULTANOUS_VFS_ACCESS  4096


#endif
