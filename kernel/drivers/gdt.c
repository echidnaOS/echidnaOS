#include <kernel.h>

#define GDT_ENTRIES 0x05
#define GDT_LENGTH (GDT_ENTRIES*8)

char GDT_pointer[6];
char GDT[GDT_LENGTH];

void build_gdt_pointer(void);
void build_gdt_entry(uint8_t entry, uint8_t access);

void build_gdt_pointer(void) {
	mem_store_w(GDT_pointer, GDT_LENGTH-1);	// gdt length
	mem_store_d(GDT_pointer+2, GDT);		// gdt start
	load_gdt(GDT_pointer);					// load the GDT register
}

void build_gdt_entry(uint8_t entry, uint8_t access) {
	if (access==0) {
		mem_store_d((entry*8)+GDT, 0x00000000);			// null descriptor
		mem_store_d((entry*8)+GDT+4, 0x00000000);		// null descriptor
	} else {
		mem_store_w((entry*8)+GDT, 0xFFFF);				// limit
		mem_store_w((entry*8)+GDT+2, 0x0000);			// base low-16
		mem_store_b((entry*8)+GDT+4, 0x00);				// base mid-8
		mem_store_b((entry*8)+GDT+5, access);			// access
		mem_store_b((entry*8)+GDT+6, 0b11001111);		// granularity
		mem_store_b((entry*8)+GDT+7, 0x00);				// base high-8
	}
}

void create_GDT(void) {

	build_gdt_pointer();		// load the GDTR

	// define table entries

	build_gdt_entry(0x00, 0x00);		// null
	build_gdt_entry(0x01, 0b10011010);	// kernel code
	build_gdt_entry(0x02, 0b10010010);	// kernel data
	build_gdt_entry(0x03, 0b11111010);	// user code
	build_gdt_entry(0x04, 0b11110010);	// user data

}
