#include <kernel.h>
#include <stdint.h>

#define KERNEL_SELECTOR 0x08
#define IDT_ENTRIES 0x81
#define IDT_LENGTH (IDT_ENTRIES*8)

char IDT_pointer[6];
char IDT[IDT_LENGTH];

// ISRs
void handler_simple(void);
void handler_code(void);
void handler_irq_pic0(void);
void handler_irq_pic1(void);
void keyboard_isr(void);
void syscall(void);

// internal functions

void build_pointer(void);
void build_idt_entry(uint8_t vector, uint16_t selector, uint32_t offset, uint8_t type);

void build_pointer(void) {
	mem_store_w(IDT_pointer, IDT_LENGTH-1);	// idt length
	mem_store_d(IDT_pointer+2, IDT);		// idt start
	load_idt(IDT_pointer);					// load the IDT register
}

void build_idt_entry(uint8_t vector, uint16_t selector, uint32_t offset, uint8_t type) {
	mem_store_w((vector*8)+IDT, offset%0x10000);
	mem_store_w((vector*8)+IDT+2, selector);
	mem_store_b((vector*8)+IDT+5, type);
	mem_store_w((vector*8)+IDT+6, offset/0x10000);
}

void create_IDT(void) {

	// define handlers' pointers
	void (*handler_simple_ptr)(void) = handler_simple;
	void (*handler_code_ptr)(void) = handler_code;
	void (*handler_irq_pic0_ptr)(void) = handler_irq_pic0;
	void (*handler_irq_pic1_ptr)(void) = handler_irq_pic1;
	void (*keyboard_isr_ptr)(void) = keyboard_isr;
	void (*syscall_ptr)(void) = syscall;

	// pointers to uint32_t
	uint32_t handler_simple_ptr_int = (uint32_t)handler_simple_ptr;
	uint32_t handler_code_ptr_int = (uint32_t)handler_code_ptr;
	uint32_t handler_irq_pic0_ptr_int = (uint32_t)handler_irq_pic0_ptr;
	uint32_t handler_irq_pic1_ptr_int = (uint32_t)handler_irq_pic1_ptr;
	uint32_t keyboard_isr_ptr_int = (uint32_t)keyboard_isr_ptr;
	uint32_t syscall_ptr_int = (uint32_t)syscall_ptr;

	build_pointer();		// load the IDTR

	// define interrupt handlers

	build_idt_entry(0x00, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x00, divide by 0
	build_idt_entry(0x01, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x01, debug
	build_idt_entry(0x02, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x02, NMI
	build_idt_entry(0x03, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x03, breakpoint
	build_idt_entry(0x04, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x04, overflow
	build_idt_entry(0x05, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x05, bound range exceeded
	build_idt_entry(0x06, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x06, invalid opcode
	build_idt_entry(0x07, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x07, device not available
	build_idt_entry(0x08, KERNEL_SELECTOR, handler_code_ptr_int, 0b10001110);			// int 0x08, double fault
	build_idt_entry(0x09, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x09, coprocessor segment overrun
	build_idt_entry(0x0A, KERNEL_SELECTOR, handler_code_ptr_int, 0b10001110);			// int 0x0A, invalid TSS
	build_idt_entry(0x0B, KERNEL_SELECTOR, handler_code_ptr_int, 0b10001110);			// int 0x0B, segment not present
	build_idt_entry(0x0C, KERNEL_SELECTOR, handler_code_ptr_int, 0b10001110);			// int 0x0C, stack-segment fault
	build_idt_entry(0x0D, KERNEL_SELECTOR, handler_code_ptr_int, 0b10001110);			// int 0x0D, general protection fault
	build_idt_entry(0x0E, KERNEL_SELECTOR, handler_code_ptr_int, 0b10001110);			// int 0x0E, page fault
	build_idt_entry(0x10, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x10, x87 floating point exception
	build_idt_entry(0x11, KERNEL_SELECTOR, handler_code_ptr_int, 0b10001110);			// int 0x11, alignement check
	build_idt_entry(0x12, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x12, machine check
	build_idt_entry(0x13, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x13, SIMD floating point exception
	build_idt_entry(0x14, KERNEL_SELECTOR, handler_simple_ptr_int, 0b10001110);		// int 0x14, virtualisation exception
	build_idt_entry(0x1E, KERNEL_SELECTOR, handler_code_ptr_int, 0b10001110);			// int 0x1E, security exception

	// define IRQ's ISRs

	build_idt_entry(0x20, KERNEL_SELECTOR, handler_irq_pic0_ptr_int, 0b10001110);		// IRQ 0, PIT
	build_idt_entry(0x21, KERNEL_SELECTOR, keyboard_isr_ptr_int, 0b10001110);			// IRQ 1, keyboard
	build_idt_entry(0x22, KERNEL_SELECTOR, handler_irq_pic0_ptr_int, 0b10001110);		// IRQ 2
	build_idt_entry(0x23, KERNEL_SELECTOR, handler_irq_pic0_ptr_int, 0b10001110);		// IRQ 3, COM2
	build_idt_entry(0x24, KERNEL_SELECTOR, handler_irq_pic0_ptr_int, 0b10001110);		// IRQ 4, COM1
	build_idt_entry(0x25, KERNEL_SELECTOR, handler_irq_pic0_ptr_int, 0b10001110);		// IRQ 5, LPT2
	build_idt_entry(0x26, KERNEL_SELECTOR, handler_irq_pic0_ptr_int, 0b10001110);		// IRQ 6, floppy
	build_idt_entry(0x27, KERNEL_SELECTOR, handler_irq_pic0_ptr_int, 0b10001110);		// IRQ 7, LPT1/spurious
	build_idt_entry(0x28, KERNEL_SELECTOR, handler_irq_pic1_ptr_int, 0b10001110);		// IRQ 8, CMOS RTC
	build_idt_entry(0x29, KERNEL_SELECTOR, handler_irq_pic1_ptr_int, 0b10001110);		// IRQ 9
	build_idt_entry(0x2A, KERNEL_SELECTOR, handler_irq_pic1_ptr_int, 0b10001110);		// IRQ 10
	build_idt_entry(0x2B, KERNEL_SELECTOR, handler_irq_pic1_ptr_int, 0b10001110);		// IRQ 11
	build_idt_entry(0x2C, KERNEL_SELECTOR, handler_irq_pic1_ptr_int, 0b10001110);		// IRQ 12, ps/2 mouse
	build_idt_entry(0x2D, KERNEL_SELECTOR, handler_irq_pic1_ptr_int, 0b10001110);		// IRQ 13, FPU
	build_idt_entry(0x2E, KERNEL_SELECTOR, handler_irq_pic1_ptr_int, 0b10001110);		// IRQ 14, primary ATA
	build_idt_entry(0x2F, KERNEL_SELECTOR, handler_irq_pic1_ptr_int, 0b10001110);		// IRQ 15, secondary ATA

	// kernel API syscall

	build_idt_entry(0x80, KERNEL_SELECTOR, syscall_ptr_int, 0b11101110);

}
