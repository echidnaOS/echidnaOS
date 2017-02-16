#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "../libs/stdint.h"
#include "../libs/stddef.h"

#define system_halt() ({					\
	asm volatile ("cli; hlt");				\
})

#define disable_ints() ({					\
	asm volatile ("cli");					\
})

#define enable_ints() ({					\
	asm volatile ("sti");					\
})

#define load_idt(location) ({				\
	asm volatile (	"lidt [ebx]"			\
					:						\
					: "b" (location)		\
					: );					\
})

#if defined(__32BIT__)		// 32-bit memory operations

#define mem_store_b(address, value) ({				\
	asm volatile (	"mov byte ptr ds:[ebx], al"		\
					:								\
					: "a" (value), "b" (address)	\
					: );							\
})

#define mem_store_w(address, value) ({				\
	asm volatile (	"mov word ptr ds:[ebx], ax"		\
					:								\
					: "a" (value), "b" (address)	\
					: );							\
})

#define mem_store_d(address, value) ({				\
	asm volatile (	"mov dword ptr ds:[ebx], eax"	\
					:								\
					: "a" (value), "b" (address)	\
					: );							\
})

#define mem_load_b(address) ({						\
	uint8_t value;									\
	asm volatile (	"mov al, byte ptr ds:[ebx]"		\
					: "=a" (value)					\
					: "b" (address)					\
					: );							\
	value;											\
})

#define mem_load_w(address) ({						\
	uint16_t value;									\
	asm volatile (	"mov ax, word ptr ds:[ebx]"		\
					: "=a" (value)					\
					: "b" (address)					\
					: );							\
	value;											\
})

#define mem_load_d(address) ({						\
	uint32_t value;									\
	asm volatile (	"mov eax, dword ptr ds:[ebx]"	\
					: "=a" (value)					\
					: "b" (address)					\
					: );							\
	value;											\
})

#elif defined(__64BIT__)	// 64-bit memory operations

#define mem_store_b(address, value) ({				\
	asm volatile (	"mov byte ptr ds:[rbx], al"		\
					:								\
					: "a" (value), "b" (address)	\
					: );							\
})

#define mem_store_w(address, value) ({				\
	asm volatile (	"mov word ptr ds:[rbx], ax"		\
					:								\
					: "a" (value), "b" (address)	\
					: );							\
})

#define mem_store_d(address, value) ({				\
	asm volatile (	"mov dword ptr ds:[rbx], eax"	\
					:								\
					: "a" (value), "b" (address)	\
					: );							\
})

#define mem_store_q(address, value) ({				\
	asm volatile (	"mov qword ptr ds:[rbx], rax"	\
					:								\
					: "a" (value), "b" (address)	\
					: );							\
})

#define mem_load_b(address) ({						\
	uint8_t value;									\
	asm volatile (	"mov al, byte ptr ds:[rbx]"		\
					: "=a" (value)					\
					: "b" (address)					\
					: );							\
	value;											\
})

#define mem_load_w(address) ({						\
	uint16_t value;									\
	asm volatile (	"mov ax, word ptr ds:[rbx]"		\
					: "=a" (value)					\
					: "b" (address)					\
					: );							\
	value;											\
})

#define mem_load_d(address) ({						\
	uint32_t value;									\
	asm volatile (	"mov eax, dword ptr ds:[rbx]"	\
					: "=a" (value)					\
					: "b" (address)					\
					: );							\
	value;											\
})

#define mem_load_q(address) ({						\
	uint64_t value;									\
	asm volatile (	"mov rax, qword ptr ds:[rbx]"	\
					: "=a" (value)					\
					: "b" (address)					\
					: );							\
	value;											\
})

#endif

#define io_wait() ({						\
	asm volatile (	"out 0x80, al"			\
					:						\
					: "a" (0)				\
					: );					\
})

#define port_out_b(port, value) ({				\
	asm volatile (	"out dx, al"				\
					:							\
					: "a" (value), "d" (port)	\
					: );						\
})

#define port_out_w(port, value) ({				\
	asm volatile (	"out dx, ax"				\
					:							\
					: "a" (value), "d" (port)	\
					: );						\
})

#define port_out_d(port, value) ({				\
	asm volatile (	"out dx, eax"				\
					:							\
					: "a" (value), "d" (port)	\
					: );						\
})

#define port_in_b(port) ({						\
	uint8_t value;								\
	asm volatile (	"in al, dx"					\
					: "=a" (value)				\
					: "d" (port)				\
					: );						\
	value;										\
})

#define port_in_w(port) ({						\
	uint16_t value;								\
	asm volatile (	"in ax, dx"					\
					: "=a" (value)				\
					: "d" (port)				\
					: );						\
	value;										\
})

#define port_in_d(port) ({						\
	uint32_t value;								\
	asm volatile (	"in eax, dx"				\
					: "=a" (value)				\
					: "d" (port)				\
					: );						\
	value;										\
})

#endif