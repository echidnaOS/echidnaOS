#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include <paging.h>
#include <system.h>
#include <task.h>
#include <klib.h>

#define BITMAP_MAX (memory_size / PAGE_SIZE)
#define BITMAP_FULL ((0x100000000 / PAGE_SIZE) / 32)
#define BITMAP_BASE (MEMORY_BASE / PAGE_SIZE)

static uint32_t mem_bitmap[BITMAP_FULL] = {0};

void init_paging(void) {
    full_identity_map();

    for (size_t i = 0; i < BITMAP_FULL; i++)
        mem_bitmap[i] = 0;

    return;
}

static int rd_bitmap(size_t i) {
    size_t entry = i / 32;
    size_t offset = i % 32;

    return (int)((mem_bitmap[entry] >> offset) & 1);
}

static void wr_bitmap(size_t i, int val) {
    size_t entry = i / 32;
    size_t offset = i % 32;

    val &= 1;

    if (val)
        mem_bitmap[entry] |= ((uint32_t)val << offset);
    else
        mem_bitmap[entry] &= ~((uint32_t)val << offset);

    return;
}

void *kmalloc(size_t pages) {
    /* allocate memory pages using a bitmap to track free and used pages */

    /* find contiguous free pages */
    size_t pg_counter = 0;
    size_t i;
    size_t strt_page;
    for (i = BITMAP_BASE; i < BITMAP_MAX; i++) {
        if (!rd_bitmap(i))
            pg_counter++;
        else
            pg_counter = 0;
        if (pg_counter == pages)
            goto found;
    }

    return (void *)0;

found:
    strt_page = i - (pages - 1);

    for (i = strt_page; i < (strt_page + pages); i++)
        wr_bitmap(i, 1);

    return (void *)(strt_page * PAGE_SIZE);

}

void kmfree(void *ptr, size_t pages) {

    size_t strt_page = (size_t)ptr / PAGE_SIZE;

    for (size_t i = strt_page; i < (strt_page + pages); i++)
        wr_bitmap(i, 0);

    return;

}

typedef struct {
    size_t pml4_entry;
    size_t pdpt_entry;
    size_t pd_entry;
    size_t pt_entry;
} pt_translation_t;

#define PML4_ADDR_SHIFT 39
#define PDPT_ADDR_SHIFT 30
#define PD_ADDR_SHIFT 21
#define PT_ADDR_SHIFT 12
#define PML4_ADDR_MASK ((size_t)0x1ff << PML4_ADDR_SHIFT)
#define PDPT_ADDR_MASK ((size_t)0x1ff << PDPT_ADDR_SHIFT)
#define PD_ADDR_MASK ((size_t)0x1ff << PD_ADDR_SHIFT)
#define PT_ADDR_MASK ((size_t)0x1ff << PT_ADDR_SHIFT)

static pt_translation_t translate_virt_addr(size_t virt_addr) {
    pt_translation_t pt_translation = {0};

    pt_translation.pml4_entry = (virt_addr & PML4_ADDR_MASK) >> PML4_ADDR_SHIFT;
    pt_translation.pdpt_entry = (virt_addr & PDPT_ADDR_MASK) >> PDPT_ADDR_SHIFT;
    pt_translation.pd_entry = (virt_addr & PD_ADDR_MASK) >> PD_ADDR_SHIFT;
    pt_translation.pt_entry = (virt_addr & PT_ADDR_MASK) >> PT_ADDR_SHIFT;

    return pt_translation;
}

int map_page(pt_entry_t *pml4, size_t virt_addr, size_t phys_addr, size_t flags) {
    pt_entry_t *pdpt;
    pt_entry_t *pd;
    pt_entry_t *pt;

    pt_translation_t pt_t = translate_virt_addr(virt_addr);

    /* check if the pml4 entry is present */
    if (pml4[pt_t.pml4_entry] & 1) {
        pdpt = (pt_entry_t *)(pml4[pt_t.pml4_entry] & 0xfffffffffffff000);
    } else {
        pdpt = kmalloc(1);    /* allocate one page */
        /* zero out the page */
        for (size_t i = 0; i < PAGE_SIZE; i++)
            ((char *)pdpt)[i] = 0;
        pml4[pt_t.pml4_entry] = (pt_entry_t)pdpt;
        pml4[pt_t.pml4_entry] |= (pt_entry_t)0x07;
    }

    /* check if the pdpt entry is present */
    if (pdpt[pt_t.pdpt_entry] & 1) {
        pd = (pt_entry_t *)(pdpt[pt_t.pdpt_entry] & 0xfffffffffffff000);
    } else {
        pd = kmalloc(1);    /* allocate one page */
        /* zero out the page */
        for (size_t i = 0; i < PAGE_SIZE; i++)
            ((char *)pd)[i] = 0;
        pdpt[pt_t.pdpt_entry] = (pt_entry_t)pd;
        pdpt[pt_t.pdpt_entry] |= (pt_entry_t)0x07;
    }

    /* check if the page table entry is present */
    if (pd[pt_t.pd_entry] & 1) {
        pt = (pt_entry_t *)(pd[pt_t.pd_entry] & 0xfffffffffffff000);
    } else {
        pt = kmalloc(1);    /* allocate one page */
        /* zero out the page */
        for (size_t i = 0; i < PAGE_SIZE; i++)
            ((char *)pt)[i] = 0;
        pd[pt_t.pd_entry] = (pt_entry_t)pt;
        pd[pt_t.pd_entry] |= (pt_entry_t)0x07;
    }

    /* do the mapping */
    pt[pt_t.pt_entry] = (pt_entry_t)(phys_addr | flags);

    return 0;
}

int unmap_page(pt_entry_t *pml4, size_t virt_addr) {
    pt_entry_t *pdpt;
    pt_entry_t *pd;
    pt_entry_t *pt;

    pt_translation_t pt_t = translate_virt_addr(virt_addr);

    /* check if the pml4 entry is present */
    if (pml4[pt_t.pml4_entry] & 1) {
        pdpt = (pt_entry_t *)(pml4[pt_t.pml4_entry] & 0xfffffffffffff000);
    } else {
        return -1;
    }

    /* check if the pdpt entry is present */
    if (pdpt[pt_t.pdpt_entry] & 1) {
        pd = (pt_entry_t *)(pdpt[pt_t.pdpt_entry] & 0xfffffffffffff000);
    } else {
        return -1;
    }

    /* check if the pd entry is present */
    if (pd[pt_t.pd_entry] & 1) {
        pt = (pt_entry_t *)(pd[pt_t.pd_entry] & 0xfffffffffffff000);
    } else {
        return -1;
    }

    /* unmap */
    pt[pt_t.pt_entry] = (pt_entry_t)0;

    return 0;
}

size_t get_phys_addr(pt_entry_t *pml4, size_t virt_addr) {
    pt_entry_t *pdpt;
    pt_entry_t *pd;
    pt_entry_t *pt;

    pt_translation_t pt_t = translate_virt_addr(virt_addr);

    pdpt = (pt_entry_t *)(pml4[pt_t.pml4_entry] & 0xfffffffffffff000);
    pd = (pt_entry_t *)(pdpt[pt_t.pdpt_entry] & 0xfffffffffffff000);
    pt = (pt_entry_t *)(pd[pt_t.pd_entry] & 0xfffffffffffff000);
    size_t phys_addr = (size_t)(pt[pt_t.pt_entry] & 0xfffffffffffff000);
    phys_addr += (virt_addr & 0xfff);

    return phys_addr;
}

int is_mapped(pt_entry_t *pml4, size_t virt_addr) {
    pt_entry_t *pdpt;
    pt_entry_t *pd;
    pt_entry_t *pt;

    pt_translation_t pt_t = translate_virt_addr(virt_addr);

    if (!(pml4[pt_t.pml4_entry] & 1))
        return 0;

    pdpt = (pt_entry_t *)(pml4[pt_t.pml4_entry] & 0xfffffffffffff000);

    if (!(pdpt[pt_t.pdpt_entry] & 1))
        return 0;

    pd = (pt_entry_t *)(pdpt[pt_t.pdpt_entry] & 0xfffffffffffff000);

    if (!(pd[pt_t.pd_entry] & 1))
        return 0;

    pt = (pt_entry_t *)(pd[pt_t.pd_entry] & 0xfffffffffffff000);

    if (!(pt[pt_t.pt_entry] & 1))
        return 0;

    return 1;
}

pt_entry_t *new_userspace(void) {
    /* allocate the new page directory */

    pt_entry_t *new_pd = kmalloc(1);    /* allocate one page */
    /* zero out the page */
    for (size_t i = 0; i < PAGE_SIZE; i++)
        ((char *)new_pd)[i] = 0;

    /* identity map the kernel (1 MiB - 16 MiB) */
    for (size_t i = KERNEL_BASE; i < KERNEL_TOP; i += PAGE_SIZE)
        map_page(new_pd, i, i, 0b11);

    return new_pd;
}

int destroy_userspace(pt_entry_t *pml4) {
    /* this function destroys a page table and frees all used pages */
    /* including the task's text and data pages */

    pt_entry_t *pdpt;
    pt_entry_t *pd;

    for (size_t i = TASK_BASE; is_mapped(pml4, i); i += PAGE_SIZE)
        kmfree((char *)get_phys_addr(pml4, i), 1);

    /* free the page tables */

    for (size_t i = 0; i < PAGE_ENTRIES; i++) {
        if (pml4[i] & 1) {
            pdpt = (pt_entry_t *)(pml4[i] & 0xfffffffffffff000);
            for (size_t i = 0; i < PAGE_ENTRIES; i++) {
                if (pdpt[i] & 1) {
                    pd = (pt_entry_t *)(pdpt[i] & 0xfffffffffffff000);
                    for (size_t i = 0; i < PAGE_ENTRIES; i++) {
                        if (pd[i] & 1) {
                            kmfree((pt_entry_t *)(pd[i] & 0xfffffffffffff000), 1);
                        }
                    }
                    kmfree(pd, 1);
                }
            }
            kmfree(pdpt, 1);
        }
    }
    kmfree(pml4, 1);

    return 0;
}

pt_entry_t *fork_userspace(pt_entry_t *pd) {
    /* allocate the new page directory */

    pt_entry_t *new_pd = kmalloc(1);    /* allocate one page */
    /* zero out the page */
    for (size_t i = 0; i < PAGE_SIZE; i++)
        ((char *)new_pd)[i] = 0;

    /* identity map the kernel (1 MiB - 16 MiB) */
    for (size_t i = KERNEL_BASE; i < KERNEL_TOP; i += PAGE_SIZE)
        map_page(new_pd, i, i, 0b11);

    for (size_t i = TASK_BASE; is_mapped(pd, i); i += PAGE_SIZE) {
        size_t phys = get_phys_addr(pd, i);
        size_t new_page = (size_t)kmalloc(1);
        kmemcpy((char *)new_page, (char *)phys, PAGE_SIZE);
        map_page(new_pd, i, new_page, 0x07);
    }

    return new_pd;
}
