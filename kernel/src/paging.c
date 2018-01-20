#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include <paging.h>
#include <system.h>

#define BYTEMAP_MAX (memory_size / PAGE_SIZE)
#define BYTEMAP_FULL (0x100000000 / PAGE_SIZE)
#define BYTEMAP_BASE (KRNL_MEMORY_BASE / PAGE_SIZE)

static uint8_t mem_bytemap[BYTEMAP_FULL] = {0};

// TODO use a bitmap instead to save space(?) and be faster(?)

void* kmalloc(size_t pages) {
    /* allocate memory pages using a bytemap to track free and used pages */

    /* find contiguous free pages */
    size_t pg_counter = 0;
    size_t i;
    size_t strt_page;
    for (i = BYTEMAP_BASE; i < BYTEMAP_MAX; i++) {
        if (!mem_bytemap[i])
            pg_counter++;
        else
            pg_counter = 0;
        if (pg_counter == pages)
            goto found;
    }

    return (void*)0;

found:
    strt_page = i - (pages - 1);

    for (i = strt_page; i < (strt_page + pages); i++)
        mem_bytemap[i] = 1;

    return (void*)(strt_page * PAGE_SIZE);

}

void kmfree(void* ptr, size_t pages) {

    size_t strt_page = (size_t)ptr / PAGE_SIZE;

    for (size_t i = strt_page; i < (strt_page + pages); i++)
        mem_bytemap[i] = 0;

    return;

}

int map_page(pt_entry_t* pd, size_t virt_addr, size_t phys_addr, size_t flags) {
    pt_entry_t* pt;

    size_t virt_page = virt_addr / PAGE_SIZE;
    size_t pd_entry = virt_page / 1024;
    size_t pt_entry = virt_page % 1024;

    /* check if the page table entry is present */
    if (pd[pd_entry] & 1) {
            pt = (pt_entry_t*)(pd[pd_entry] & 0xfffff000);
    } else {
            pt = kmalloc(1);    /* allocate one page */
            /* zero out the page */
            for (size_t i = 0; i < PAGE_SIZE; i++)
                ((char*)pt)[i] = 0;
            pd[pd_entry] = (pt_entry_t)pt;
            pd[pd_entry] |= (pt_entry_t)flags;
    }

    /* do the mapping */
    pt[pt_entry] = (pt_entry_t)(phys_addr | flags);

    return 0;
}

int unmap_page(pt_entry_t* pd, size_t virt_addr) {
    pt_entry_t* pt;

    size_t virt_page = virt_addr / PAGE_SIZE;
    size_t pd_entry = virt_page / 1024;
    size_t pt_entry = virt_page % 1024;

    /* check if the page table entry is present */
    if (pd[pd_entry] & 1) {
            pt = (pt_entry_t*)(pd[pd_entry] & 0xfffff000);
    } else {
            return -1;
    }

    /* unmap */
    pt[pt_entry] = (pt_entry_t)0;

    return 0;
}

size_t get_phys_addr(pt_entry_t* pd, size_t virt_addr) {
    pt_entry_t* pt;
    pt_entry_t* phys;

    size_t virt_page = virt_addr / PAGE_SIZE;
    size_t pd_entry = virt_page / 1024;
    size_t pt_entry = virt_page % 1024;

    pt = (pt_entry_t*)(pd[pd_entry] & 0xfffff000);
    phys = (pt_entry_t*)(pt[pt_entry] & 0xfffff000);
    size_t phys_addr = (size_t)phys;
    phys_addr += (virt_addr & 0xfff);

    return phys_addr;
}

pt_entry_t* new_userspace(void) {
    /* allocate the new page directory */

    pt_entry_t* new_pd = kmalloc(1);    /* allocate one page */
    /* zero out the page */
    for (size_t i = 0; i < PAGE_SIZE; i++)
        ((char*)new_pd)[i] = 0;

    /* identity map the kernel (1 MiB - 16 MiB) */
    for (size_t i = 0x100000; i < 0x1000000; i += 0x1000)
        map_page(new_pd, i, i, 0b11);

    return new_pd;
}
