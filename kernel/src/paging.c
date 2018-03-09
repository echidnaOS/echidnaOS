#include <stdint.h>
#include <stddef.h>
#include <kernel.h>
#include <paging.h>
#include <system.h>
#include <task.h>
#include <klib.h>

#define MBITMAP_FULL ((0x100000000 / PAGE_SIZE) / 32)
size_t BITMAP_FULL = MBITMAP_FULL;
#define BITMAP_BASE (MEMORY_BASE / PAGE_SIZE)

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t unused;
} __attribute__((packed)) e820_entry_t;

e820_entry_t e820_map[256];

static const char *e820_type(uint32_t type) {
    switch (type) {
        case 1:
            return "Usable RAM";
        case 2:
            return "Reserved";
        case 3:
            return "ACPI reclaimable";
        case 4:
            return "ACPI NVS";
        case 5:
            return "Bad memory";
        default:
            return "???";
    }
}

static volatile uint32_t *mem_bitmap;
static volatile uint32_t initial_bitmap[MBITMAP_FULL];
static volatile uint32_t *tmp_bitmap;

static int rd_bitmap(size_t i) {
    size_t entry = i / 32;
    size_t offset = i % 32;

    return (int)((mem_bitmap[entry] >> offset) & 1);
}

static void wr_bitmap(size_t i, int val) {
    size_t entry = i / 32;
    size_t offset = i % 32;

    if (val)
        mem_bitmap[entry] |= (1 << offset);
    else
        mem_bitmap[entry] &= ~(1 << offset);

    return;
}

void bmrealloc(void) {
    /* makes the bitmap a bit bigger */
    if ((tmp_bitmap = kalloc((BITMAP_FULL + 2048) * sizeof(uint32_t))) == 0)
        panic("", 0);

    kmemcpy(tmp_bitmap, mem_bitmap, BITMAP_FULL * sizeof(uint32_t));
    for (size_t i = BITMAP_FULL; i < BITMAP_FULL + 2048; i++)
        tmp_bitmap[i] = 0xffffffff;

    BITMAP_FULL += 2048;

    asm volatile (
        "xchg rax, rdx;"
        : "=a" (tmp_bitmap), "=d" (mem_bitmap)
        : "a" (tmp_bitmap), "d" (mem_bitmap)
    );

    kfree(tmp_bitmap);

    return;
}

void init_paging(void) {
    full_identity_map();

    for (size_t i = 0; i < BITMAP_FULL; i++)
        initial_bitmap[i] = 0;

    mem_bitmap = initial_bitmap;
    tmp_bitmap = kalloc(BITMAP_FULL * sizeof(uint32_t));
    for (size_t i = 0; i < BITMAP_FULL; i++)
        tmp_bitmap[i] = initial_bitmap[i];
    mem_bitmap = tmp_bitmap;

    for (size_t i = memory_size / PAGE_SIZE; i < BITMAP_FULL * 32; i++)
        wr_bitmap(i, 1);

    /* get e820 memory map */
    get_e820(e820_map);

    /* print out memory map */
    for (size_t i = 0; e820_map[i].type; i++) {
        kprint(KPRN_INFO, "e820: [%X -> %X] : %X  <%s>", e820_map[i].base,
                                              e820_map[i].base + e820_map[i].length,
                                              e820_map[i].length,
                                              e820_type(e820_map[i].type));

        if (e820_map[i].base >= memory_size) {
            for (size_t j = 0; (j * PAGE_SIZE) < e820_map[i].length; j++) {
                size_t addr = e820_map[i].base + j * PAGE_SIZE;
                size_t page = addr / PAGE_SIZE;
                map_page(kernel_pagemap, addr, addr, 0x03);
                while (page >= BITMAP_FULL * 32)
                    bmrealloc();
                if (e820_map[i].type == 1)
                    wr_bitmap(page, 0);
                else
                    wr_bitmap(page, 1);
            }
        }
    }

    return;
}

void *kmalloc(size_t pages) {
    /* allocate memory pages using a bitmap to track free and used pages */

    /* find contiguous free pages */
    size_t pg_counter = 0;
    size_t i;
    size_t strt_page;
    for (i = BITMAP_BASE; i < BITMAP_FULL * 32; i++) {
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

static size_t translate_phys_addr(size_t pml4_entry,
                                  size_t pdpt_entry,
                                  size_t pd_entry,
                                  size_t pt_entry) {
    size_t virt_addr = 0;

    virt_addr |= pml4_entry << PML4_ADDR_SHIFT;
    virt_addr |= pdpt_entry << PDPT_ADDR_SHIFT;
    virt_addr |= pd_entry << PD_ADDR_SHIFT;
    virt_addr |= pt_entry << PT_ADDR_SHIFT;

    return virt_addr;
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
    pt_entry_t *pt;

    /* cool and hip meme */

    for (size_t i = 0; i < PAGE_ENTRIES; i++) {
        if (pml4[i] & 1) {
            pdpt = (pt_entry_t *)(pml4[i] & 0xfffffffffffff000);
            for (size_t i = 0; i < PAGE_ENTRIES; i++) {
                if (pdpt[i] & 1) {
                    pd = (pt_entry_t *)(pdpt[i] & 0xfffffffffffff000);
                    for (size_t i = 0; i < PAGE_ENTRIES; i++) {
                        if (pd[i] & 1) {
                            pt = (pt_entry_t *)(pd[i] & 0xfffffffffffff000);
                            for (size_t i = 0; i < PAGE_ENTRIES; i++) {
                                if (pt[i] & 1)
                                    kmfree((pt_entry_t *)(pt[i] & 0xfffffffffffff000), 1);
                            }
                            kmfree(pt, 1);
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

pt_entry_t *fork_userspace(pt_entry_t *pml4) {
    /* allocate the new page directory */

    pt_entry_t *pdpt;
    pt_entry_t *pd;
    pt_entry_t *pt;

    pt_entry_t *new_pml4 = kmalloc(1);    /* allocate one page */
    /* zero out the page */
    for (size_t i = 0; i < PAGE_SIZE; i++)
        ((char *)new_pml4)[i] = 0;

    /* identity map the kernel (1 MiB - 16 MiB) */
    for (size_t i = KERNEL_BASE; i < KERNEL_TOP; i += PAGE_SIZE)
        map_page(new_pml4, i, i, 0b11);

    /* cool and hip meme */

    for (size_t i = 0; i < PAGE_ENTRIES; i++) {
        if (pml4[i] & 1) {
            pdpt = (pt_entry_t *)(pml4[i] & 0xfffffffffffff000);
            for (size_t j = 0; j < PAGE_ENTRIES; j++) {
                if (pdpt[j] & 1) {
                    pd = (pt_entry_t *)(pdpt[j] & 0xfffffffffffff000);
                    for (size_t k = 0; k < PAGE_ENTRIES; k++) {
                        if (pd[k] & 1) {
                            pt = (pt_entry_t *)(pd[k] & 0xfffffffffffff000);
                            for (size_t l = 0; l < PAGE_ENTRIES; l++) {
                                if (pt[l] & 1) {
                                    if (translate_phys_addr(i, j, k, l) < 0x1000000)
                                        goto ignore;
                                    size_t new_page = (size_t)kmalloc(1);
                                    kmemcpy((char *)new_page, (char *)(pt[l] & 0xfffffffffffff000), PAGE_SIZE);
                                    map_page(new_pml4, translate_phys_addr(i, j, k, l), new_page, 0x07);
                                }
                                ignore: ;
                            }
                        }
                    }
                }
            }
        }
    }

    return new_pml4;
}
