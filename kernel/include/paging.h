#ifndef __PAGING_H__
#define __PAGING_H__

#include <stdint.h>
#include <stddef.h>

#define KERNEL_PAGES            0x800000
#define KERNEL_PAGE             (KERNEL_PAGES + 0x400000)
#define PAGE_SIZE               4096
#define INITRAMFS_BASE          0x1000000
#define INITRAMFS_SIZE          0x800000
#define KRNL_MEMORY_BASE        (INITRAMFS_BASE + INITRAMFS_SIZE)


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
