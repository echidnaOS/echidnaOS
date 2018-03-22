#ifndef __KERNEL_H__
#define __KERNEL_H__


// misc tunables

#define KRNL_PIT_FREQ 4000
//#define _SERIAL_KERNEL_OUTPUT_


// task tunables

#define KRNL_MAX_TASKS 65536
#define DEFAULT_STACK 0x10000
#define DEF_STACK_BASE 0x80000000


#define MAX_MADT 128


#define MAX_CPUS 32


// tty tunables

#define KB_L1_SIZE 256
#define KB_L2_SIZE 2048

#define KRNL_TTY_COUNT 7
#define TTY_DEF_CUR_BG_COL 0x00ffffff
#define TTY_DEF_CUR_FG_COL 0x00000000
#define TTY_DEF_TXT_BG_COL 0x00000000
#define TTY_DEF_TXT_FG_COL 0x00808080

#define GUI_REDRAW_LIMIT 200

#define MOUSE_POLL_LIMIT 10


// vfs tunables

#define MAX_SIMULTANOUS_VFS_ACCESS  (64*1024)


#endif
