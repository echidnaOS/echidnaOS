#include <kernel.h>
#include <stdint.h>

void memory_map(uint32_t directory_number, uint32_t phys_address, uint32_t virt_address, uint8_t access) {
	uint32_t directory_address = 0x100000+(directory_number*1024);
	uint32_t entry_offset = virt_address/0x100000;
	if (access) {
		access = 0x87;
	} else {
		access = 0x83;
	}
	mem_store_d(directory_address+entry_offset, phys_address+access);
}
