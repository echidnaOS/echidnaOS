#ifndef __PAGING_H__
#define __PAGING_H__

#include <stdint.h>
#include <stddef.h>

#include <initramfs.h>

/* location of the kernel's page directory */
#define KERNEL_PAGES            0x800000
#define KERNEL_PAGE             (KERNEL_PAGES + 0x400000)
#define KERNEL_BASE             0x100000
#define KERNEL_TOP              0x1000000

/* arch specific values */
#define PAGE_SIZE               4096
#define PAGE_ENTRIES            1024

#define MEMORY_BASE             (INITRAMFS_BASE + INITRAMFS_SIZE)


typedef uint32_t pt_entry_t;


void full_identity_map(pt_entry_t *);
pt_entry_t *new_userspace(void);
void *kmalloc(size_t);
void kmfree(void *, size_t);
int map_page(pt_entry_t *, size_t, size_t, size_t);
int unmap_page(pt_entry_t *, size_t);
size_t get_phys_addr(pt_entry_t *, size_t);
int is_mapped(pt_entry_t *, size_t);
pt_entry_t *fork_userspace(pt_entry_t *);
int destroy_userspace(pt_entry_t *);




#endif
