#include <stdint.h>
#include <stddef.h>
#include <kernel.h>

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

typedef uint32_t pt_entry_t;

pt_entry_t* create_empty_pt(void) {
    return (pt_entry_t*)kalloc(PAGE_SIZE);
}

int map_page(pt_entry_t* pd, size_t virt_addr, size_t phys_addr, size_t flags) {
    pt_entry_t* pt;

    size_t virt_page = virt_addr / PAGE_SIZE;
    size_t phys_page = phys_addr / PAGE_SIZE;

    size_t pd_entry = virt_page / PAGE_SIZE;

    /* check if the page table entry is present */

    if (pd[pd_entry] & 1)
            pt = (pt_entry_t*)(pd[pd_entry] & 0xfffff000);
    else {
            pt = create_empty_pt();
            pd[pd_entry] = (size_t)pt;
            pd[pd_entry] += flags;
    }

    /* do the mapping */

    /* TODO */
}
