#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__

#define text_putchar(value) ({				\
	asm volatile (	"push eax;"			\
			"mov eax, 0x20;"		\
			"int 0x80;"			\
			"pop eax;"			\
			:				\
			: "a" (value)			\
			: );				\
})

#define text_putstring(value) ({			\
	asm volatile (	"push eax;"			\
			"mov eax, 0x21;"		\
			"int 0x80;"			\
			"pop eax;"			\
			:				\
			: "a" (value)			\
			: );				\
})

#endif
