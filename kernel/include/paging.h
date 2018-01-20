#ifndef __PAGING_H__
#define __PAGING_H__

#include <stdint.h>
#include <stddef.h>



typedef uint32_t pt_entry_t;


void full_identity_map(pt_entry_t*);
pt_entry_t* new_userspace(void);
void* kmalloc(size_t);
void kmfree(void*, size_t);
int map_page(pt_entry_t*, size_t, size_t, size_t);
int unmap_page(pt_entry_t*, size_t);
size_t get_phys_addr(pt_entry_t*, size_t);




#endif
